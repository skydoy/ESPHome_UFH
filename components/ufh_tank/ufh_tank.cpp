#include "ufh_tank.h"
#include "packet_rx.h"
#include "packet_tx.h"

#include "esphome/core/application.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/components/network/util.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <cerrno>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <unistd.h>

namespace esphome {
namespace ufh_tank {

static const char *const TAG = "ufh_tank";
static constexpr uint32_t kUdpStaleMs = 60000UL;
static constexpr uint16_t kUdpPort = 18511;

TankComponent *global_tank = nullptr;

static int16_t to_x10(float v) { return (int16_t) lroundf(v * 10.0f); }
static float x10(int16_t v) { return v / 10.0f; }
static bool udp_fresh(uint32_t ms) { return ms != 0 && millis() - ms < kUdpStaleMs; }

uint64_t parse_hex64(const char *s) {
  if (s == nullptr || !s[0])
    return 0;
  uint64_t v = 0;
  for (const char *p = s; *p; p++) {
    char c = *p;
    if (c == ':' || c == '-' || c == ' ')
      continue;
    uint8_t n;
    if (c >= '0' && c <= '9')
      n = c - '0';
    else if (c >= 'a' && c <= 'f')
      n = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      n = c - 'A' + 10;
    else
      return 0;
    v = (v << 4) | n;
  }
  return v;
}

std::string hex64_to_str(uint64_t v) {
  char b[17];
  snprintf(b, sizeof(b), "%016llx", (unsigned long long) v);
  return std::string(b);
}

static uint8_t crc8_dallas(const uint8_t *d, size_t n) {
  uint8_t crc = 0;
  for (size_t i = 0; i < n; i++) {
    uint8_t in = d[i];
    for (int k = 0; k < 8; k++) {
      uint8_t mix = (crc ^ in) & 1;
      crc >>= 1;
      if (mix)
        crc ^= 0x8C;
      in >>= 1;
    }
  }
  return crc;
}

void TankComponent::setup() {
  global_tank = this;
  bool al = default_active_low_;
  if (pump_out_)
    pump_out_->set_state(al);
  if (heater_out_)
    heater_out_->set_state(al);
  if (pwm_out_) {
    pwm_out_->set_level(0);
    pwm_out_->turn_off();
  }
  this->load_settings_();
  if (cfg_.relay_active_low)
    default_active_low_ = true;
#ifdef USE_UFH_ONE_WIRE
  ow_tank_addr_ = cfg_.tank_probe[0] ? parse_hex64(cfg_.tank_probe) : 0;
#endif
  this->write_pump_(false);
  this->write_heater_(false);
  this->write_pwm_(0);
  ESP_LOGI(TAG, "UFH tanque ready (PWM pin GPIO%d, profile %s)", pwm_pin_,
           cfg_.pwm_invert ? "A inverted" : "direct");
}

void TankComponent::load_settings_() {
  this->pref_ = global_preferences->make_preference<TankCfg>(fnv1_hash("ufh_tank_v2"));
  if (!this->pref_.load(&this->cfg_) || this->cfg_.magic != TANK_MAGIC) {
    default_tank(this->cfg_);
    ESP_LOGW(TAG, "No tank settings stored: open the device web page");
  }
  cfg_.ufh_provider[sizeof(cfg_.ufh_provider) - 1] = 0;
  cfg_.altherma_provider[sizeof(cfg_.altherma_provider) - 1] = 0;
  cfg_.tank_probe[sizeof(cfg_.tank_probe) - 1] = 0;
  if (cfg_.tank_target_x10 == 0 || cfg_.tank_cut_x10 == 0) {
    cfg_.tank_target_x10 = 400;
    cfg_.tank_cut_x10 = 450;
    if (cfg_.tank_hyst_x10 == 0)
      cfg_.tank_hyst_x10 = 30;
    if (cfg_.comp_zero_min == 0)
      cfg_.comp_zero_min = 15;
  }
  if (cfg_.pwm_max_speed == 0)
    cfg_.pwm_max_speed = 100;
  if (cfg_.pwm_zones_max == 0)
    cfg_.pwm_zones_max = 8;
}

void TankComponent::save_settings_() { this->pref_.save(&this->cfg_); }

void TankComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "UFH tanque");
  ESP_LOGCONFIG(TAG, "  Board: %s", board_name_.c_str());
  ESP_LOGCONFIG(TAG, "  UFH provider: %s", cfg_.ufh_provider[0] ? cfg_.ufh_provider : "(none)");
  ESP_LOGCONFIG(TAG, "  Altherma provider: %s", cfg_.altherma_provider[0] ? cfg_.altherma_provider : "(none)");
}

void TankComponent::on_shutdown() {
  this->write_pump_(false);
  this->write_heater_(false);
  this->write_pwm_(0);
}

void TankComponent::write_pump_(bool on) {
  pump_on_ = on;
  if (pump_out_ == nullptr)
    return;
  bool level = default_active_low_ ? !on : on;
  pump_out_->set_state(level);
}

void TankComponent::write_heater_(bool on) {
  heater_on_ = on;
  if (heater_out_ == nullptr)
    return;
  bool level = default_active_low_ ? !on : on;
  heater_out_->set_state(level);
}

void TankComponent::write_pwm_(float duty_pct) {
  if (duty_pct < 0)
    duty_pct = 0;
  if (duty_pct > 100)
    duty_pct = 100;
  pwm_duty_ = duty_pct;
  if (pwm_out_ == nullptr)
    return;
  if (duty_pct <= 0) {
    pwm_out_->set_level(0);
    pwm_out_->turn_off();
  } else {
    pwm_out_->turn_on();
    pwm_out_->set_level(duty_pct / 100.0f);
  }
}

float TankComponent::tank_temp() const {
  return tank_temp_ms_ != 0 && millis() - tank_temp_ms_ < 300000UL ? (float) tank_temp_ : NAN;
}

bool TankComponent::ufh_fresh() const { return udp_fresh(ufh_ms_); }
bool TankComponent::altherma_fresh() const { return udp_fresh(alt_ms_); }

int TankComponent::ufh_age_s() const {
  return ufh_fresh() ? (int) ((millis() - ufh_ms_) / 1000) : -1;
}
int TankComponent::altherma_age_s() const {
  return altherma_fresh() ? (int) ((millis() - alt_ms_) / 1000) : -1;
}

float TankComponent::compressor_freq() const {
  return this->altherma_fresh() ? (float) alt_freq_ : NAN;
}
float TankComponent::outdoor_temp() const {
  return this->altherma_fresh() ? (float) alt_outdoor_ : NAN;
}
int TankComponent::heat_pump_fault_state() const {
  if (!this->altherma_fresh())
    return -1;
  return alt_fault_;
}

void TankComponent::altherma_error_text(bool outdoor, char *buf, size_t len) const {
  if (buf == nullptr || len < 3)
    return;
  buf[0] = 0;
  if (!this->altherma_fresh())
    return;
  float v = outdoor ? (float) alt_out_err_ : (float) alt_in_err_;
  if (std::isnan(v))
    return;
  uint8_t b = (uint8_t) lroundf(v);
  if (b == 0)
    return;
  char tmp[4]{};
  esphome::ufh::packet_rx::decode_error_code(b, tmp);
  std::strncpy(buf, tmp, len - 1);
  buf[len - 1] = 0;
}
bool TankComponent::heat_pump_fault() const { return this->heat_pump_fault_state() == 1; }

void TankComponent::set_ufh_provider(const char *name) {
  char prev[sizeof(cfg_.ufh_provider)];
  memcpy(prev, cfg_.ufh_provider, sizeof(prev));
  memset(cfg_.ufh_provider, 0, sizeof(cfg_.ufh_provider));
  if (name != nullptr)
    strncpy(cfg_.ufh_provider, name, sizeof(cfg_.ufh_provider) - 1);
  if (memcmp(prev, cfg_.ufh_provider, sizeof(prev)) != 0) {
    ufh_ms_ = 0;
    ufh_demand_ = -1;
    ufh_zones_ = NAN;
    this->save_settings_();
  }
}

void TankComponent::set_altherma_provider(const char *name) {
  char prev[sizeof(cfg_.altherma_provider)];
  memcpy(prev, cfg_.altherma_provider, sizeof(prev));
  memset(cfg_.altherma_provider, 0, sizeof(cfg_.altherma_provider));
  if (name != nullptr)
    strncpy(cfg_.altherma_provider, name, sizeof(cfg_.altherma_provider) - 1);
  if (memcmp(prev, cfg_.altherma_provider, sizeof(prev)) != 0) {
    alt_ms_ = 0;
    alt_fault_ = -1;
    alt_freq_ = alt_outdoor_ = alt_out_err_ = alt_in_err_ = NAN;
    this->save_settings_();
  }
}

void TankComponent::set_target(float v) {
  v = std::clamp(v, 30.0f, 44.0f);
  int16_t x = to_x10(v);
  if (cfg_.tank_target_x10 == x)
    return;
  cfg_.tank_target_x10 = x;
  if (cfg_.tank_cut_x10 <= cfg_.tank_target_x10)
    cfg_.tank_cut_x10 = std::min<int16_t>(450, (int16_t) (cfg_.tank_target_x10 + 10));
  this->save_settings_();
}

void TankComponent::set_cut(float v) {
  v = std::clamp(v, 35.0f, 45.0f);
  int16_t x = to_x10(v);
  if (x < cfg_.tank_target_x10 + 5)
    x = (int16_t) (cfg_.tank_target_x10 + 5);
  if (x > 450)
    x = 450;
  if (cfg_.tank_cut_x10 == x)
    return;
  cfg_.tank_cut_x10 = x;
  this->save_settings_();
}

void TankComponent::set_comp_zero_min(uint16_t v) {
  if (v < 5)
    v = 5;
  if (v > 120)
    v = 120;
  if (cfg_.comp_zero_min == v)
    return;
  cfg_.comp_zero_min = v;
  this->save_settings_();
}

void TankComponent::set_pwm_speeds(uint8_t min_s, uint8_t max_s, uint8_t zones_max, bool invert) {
  if (min_s > 100)
    min_s = 100;
  if (max_s > 100)
    max_s = 100;
  if (max_s < min_s)
    max_s = min_s;
  if (zones_max < 1)
    zones_max = 1;
  if (zones_max > 16)
    zones_max = 16;
  cfg_.pwm_min_speed = min_s;
  cfg_.pwm_max_speed = max_s;
  cfg_.pwm_zones_max = zones_max;
  cfg_.pwm_invert = invert ? 1 : 0;
  this->save_settings_();
}

void TankComponent::set_heater_manual(bool on) {
  if (on) {
    heater_manual_on_ = true;
    heater_auto_blocked_ = false;
  } else {
    heater_manual_on_ = false;
    if (this->heat_pump_fault())
      heater_auto_blocked_ = true;
  }
}

void TankComponent::loop() {
  uint32_t now_ms = millis();
  net_up_ = network::is_connected();
  if (!web_registered_ && web_base_ != nullptr && net_up_) {
    web_registered_ = true;
    register_web_handler(this, web_base_);
  }
  if (!udp_task_started_ && net_up_)
    this->udp_task_start_();

  std::string cfg;
  {
    std::lock_guard<std::mutex> lock(mtx_);
    if (cfg_pending_) {
      cfg.swap(pending_cfg_);
      cfg_pending_ = false;
    }
  }
  if (!cfg.empty()) {
    std::string err;
    std::lock_guard<std::mutex> lock(mtx_);
    this->web_apply_config(cfg, err);
  }
  if (restart_requested_) {
    if (restart_at_ms_ == 0)
      restart_at_ms_ = now_ms + 1500;
    else if ((int32_t) (now_ms - restart_at_ms_) > 0) {
      global_preferences->sync();
      App.safe_reboot();
    }
  }
#ifdef USE_UFH_ONE_WIRE
  this->ow_tick_(now_ms);
#endif
  if (now_ms - last_tick_ms_ < 500)
    return;
  last_tick_ms_ = now_ms;
  std::lock_guard<std::mutex> lock(mtx_);
  this->tick_(now_ms);
}

void TankComponent::tick_(uint32_t now_ms) {
  this->update_outputs_(now_ms);

  std::string w;
  if (!cfg_.configured)
    w = "setup";
  if (!cfg_.ufh_provider[0])
    w += (w.empty() ? "" : "; ") + std::string("UFH: pick a board");
  else if (!this->ufh_fresh() && cfg_.configured)
    w += (w.empty() ? "" : "; ") + std::string("UFH link: no response");
  if (!cfg_.altherma_provider[0])
    w += (w.empty() ? "" : "; ") + std::string("Altherma: pick a board");
  else if (!this->altherma_fresh() && cfg_.configured)
    w += (w.empty() ? "" : "; ") + std::string("Altherma link: no response");
  if (this->heat_pump_fault())
    w += (w.empty() ? "" : "; ") + std::string("heat pump fault");
  if (comp_zero_warn_)
    w += (w.empty() ? "" : "; ") + std::string("compressor 0 with heat demand: turn the heater on by hand");
  if (tank_cut_warn_ms_ != 0 && now_ms - tank_cut_warn_ms_ < 600000UL)
    w += (w.empty() ? "" : "; ") + std::string("tank over 45 C: heater cut off");
  if (cfg_.tank_probe[0] == 0 || std::isnan(this->tank_temp()))
    w += (w.empty() ? "" : "; ") + std::string("tank probe");
  warnings_text_ = w;
}

void TankComponent::update_outputs_(uint32_t now_ms) {
  bool ufh_ok = this->ufh_fresh();
  bool demand = ufh_ok && ufh_demand_ == 1;
  int zones = 0;
  if (ufh_ok && !std::isnan(ufh_zones_))
    zones = std::max(0, (int) lroundf(ufh_zones_));

  // Stale UFH: pump off, heater off.
  bool want_pump = demand;  // no zones / no demand => relay off
  if (!ufh_ok)
    want_pump = false;

  // PWM by number of open zones. Relay off when no demand.
  float speed = 0, duty = 0;
  if (want_pump) {
    uint8_t nmax = cfg_.pwm_zones_max ? cfg_.pwm_zones_max : 8;
    uint8_t mn = cfg_.pwm_min_speed, mx = cfg_.pwm_max_speed ? cfg_.pwm_max_speed : 100;
    if (mx < mn)
      mx = mn;
    int n = std::max(1, std::min((int) nmax, zones > 0 ? zones : 1));
    if (nmax <= 1)
      speed = mx;
    else
      speed = mn + (float) (mx - mn) * (float) (n - 1) / (float) (nmax - 1);
    duty = esphome::ufh::packet_rx::speed_to_duty(speed, cfg_.pwm_invert != 0);
  }
  pwm_speed_ = speed;
  if (want_pump != pump_on_)
    this->write_pump_(want_pump);
  this->write_pwm_(want_pump ? duty : 0);

  // compressor-zero warning (never auto-starts the heater)
  float freq = this->compressor_freq();
  if (demand && !std::isnan(freq) && freq <= 0.0f && this->heat_pump_fault_state() == 0) {
    if (comp_zero_since_ms_ == 0)
      comp_zero_since_ms_ = now_ms ? now_ms : 1;
    uint32_t need = (uint32_t) cfg_.comp_zero_min * 60000UL;
    if (need < 60000UL)
      need = 60000UL;
    comp_zero_warn_ = (now_ms - comp_zero_since_ms_) >= need;
  } else {
    comp_zero_since_ms_ = 0;
    comp_zero_warn_ = false;
  }

  float t = this->tank_temp();
  bool probe_ok = !std::isnan(t);
  const float cut = x10(cfg_.tank_cut_x10);
  const float target = x10(cfg_.tank_target_x10);
  const float hyst = x10(cfg_.tank_hyst_x10 > 0 ? cfg_.tank_hyst_x10 : 30);
  bool over = probe_ok && t >= cut;
  if (over)
    tank_cut_warn_ms_ = now_ms ? now_ms : 1;

  bool fault = this->heat_pump_fault();
  if (!fault)
    heater_auto_blocked_ = false;

  bool want = false;
  // Auto only on fresh altherma_fault. Stale UFH or failed probe: heater off.
  if (ufh_ok && demand && probe_ok && !over) {
    if (fault && !heater_auto_blocked_) {
      if (!heater_on_)
        want = t <= target - hyst;
      else
        want = t < target;
    }
    if (heater_manual_on_)
      want = true;
  }
  if (over || !probe_ok || !ufh_ok)
    want = false;

  const uint32_t min_on = 120000UL, min_off = 300000UL;
  if (want != heater_on_ && heater_since_ms_ != 0) {
    uint32_t held = now_ms - heater_since_ms_;
    if (heater_on_ && held < min_on && !over && probe_ok && ufh_ok)
      want = true;
    else if (!heater_on_ && held < min_off)
      want = false;
  }
  if (want != heater_on_) {
    this->write_heater_(want);
    heater_since_ms_ = now_ms ? now_ms : 1;
    tx_dirty_ = true;
  }
  if (want_pump != pump_on_)
    tx_dirty_ = true;
}

void TankComponent::udp_task_start_() {
  if (udp_task_started_)
    return;
  udp_task_started_ = true;
  xTaskCreate(&TankComponent::udp_task_, "tank_udp", 6144, this, 1, nullptr);
  ESP_LOGI(TAG, "UDP packet_transport on port 18511");
}

void TankComponent::udp_on_packet_(const uint8_t *data, size_t len) {
  using namespace esphome::ufh::packet_rx;
  Packet pkt{};
  if (!parse(data, len, pkt) || pkt.is_ping || !pkt.hostname[0])
    return;
  std::string me = App.get_name().str();
  if (me == pkt.hostname)
    return;
  bool is_ufh = looks_like_ufh(pkt);
  bool is_alt = looks_like_altherma(pkt);
  if (!is_ufh && !is_alt)
    return;
  uint32_t now = millis();
  if (now == 0)
    now = 1;
  {
    std::lock_guard<std::mutex> lock(udp_mtx_);
    int slot = -1;
    for (uint8_t i = 0; i < seen_n_; i++) {
      if (strcmp(seen_[i].name, pkt.hostname) == 0) {
        slot = i;
        break;
      }
    }
    if (slot < 0) {
      if (seen_n_ < MAX_SEEN) {
        slot = seen_n_++;
        seen_[slot] = Seen{};
      } else {
        slot = 0;
        for (uint8_t i = 1; i < seen_n_; i++)
          if (seen_[i].ms < seen_[slot].ms)
            slot = i;
        seen_[slot] = Seen{};
      }
    }
    strncpy(seen_[slot].name, pkt.hostname, sizeof(seen_[slot].name) - 1);
    seen_[slot].name[sizeof(seen_[slot].name) - 1] = 0;
    seen_[slot].ms = now;
    seen_[slot].is_ufh = seen_[slot].is_ufh || is_ufh;
    seen_[slot].is_alt = seen_[slot].is_alt || is_alt;
  }
  if (is_ufh && cfg_.ufh_provider[0] && strcmp(cfg_.ufh_provider, pkt.hostname) == 0) {
    if (const auto *b = find_binary(pkt, "ufh_heat_demand"))
      ufh_demand_ = b->value ? 1 : 0;
    if (const auto *s = find_sensor(pkt, "ufh_open_zones"))
      ufh_zones_ = s->value;
    ufh_ms_ = now;
  }
  if (is_alt && cfg_.altherma_provider[0] && strcmp(cfg_.altherma_provider, pkt.hostname) == 0) {
    if (const auto *s = find_sensor(pkt, "altherma_outdoor_temp"))
      alt_outdoor_ = s->value;
    if (const auto *s = find_sensor(pkt, "altherma_comp_freq"))
      alt_freq_ = s->value;
    if (const auto *b = find_binary(pkt, "altherma_fault"))
      alt_fault_ = b->value ? 1 : 0;
    if (const auto *s = find_sensor(pkt, "altherma_outdoor_error"))
      alt_out_err_ = s->value;
    if (const auto *s = find_sensor(pkt, "altherma_indoor_error"))
      alt_in_err_ = s->value;
    alt_ms_ = now;
  }
}

void TankComponent::udp_send_(int sock) {
  using namespace esphome::ufh::packet_rx;
  std::string name = App.get_name().str();
  if (name.empty())
    return;
  TxSensor sensors[2] = {{"tank_temp", this->tank_temp()}, {"tank_pwm", pwm_duty_}};
  TxBinary binary[4] = {{"tank_pump", pump_on_ ? (uint8_t) 1 : (uint8_t) 0},
                        {"tank_heater", heater_on_ ? (uint8_t) 1 : (uint8_t) 0},
                        {"tank_heater_manual", heater_manual_on_ ? (uint8_t) 1 : (uint8_t) 0},
                        {"tank_comp_zero", comp_zero_warn_ ? (uint8_t) 1 : (uint8_t) 0}};
  // NAN temp: still send the field (receiver treats stale by age)
  uint8_t buf[256];
  size_t n = 0;
  if (!build(buf, sizeof(buf), n, name.c_str(), sensors, 2, binary, 4))
    return;
  sockaddr_in dst{};
  dst.sin_family = AF_INET;
  dst.sin_port = htons(kUdpPort);
  dst.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  ::sendto(sock, buf, n, 0, reinterpret_cast<sockaddr *>(&dst), sizeof(dst));
  last_tx_ms_ = millis();
  tx_dirty_ = false;
}

void TankComponent::udp_task_(void *arg) {
  auto *self = static_cast<TankComponent *>(arg);
  for (;;) {
    while (!self->net_up_)
      vTaskDelay(pdMS_TO_TICKS(1000));
    int sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
      ESP_LOGW(TAG, "UDP socket failed: %d", errno);
      vTaskDelay(pdMS_TO_TICKS(2000));
      continue;
    }
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#ifdef SO_REUSEPORT
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));
#endif
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(kUdpPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0) {
      ESP_LOGW(TAG, "UDP bind 18511 failed: %d", errno);
      close(sock);
      vTaskDelay(pdMS_TO_TICKS(2000));
      continue;
    }
    timeval tv{};
    tv.tv_sec = 1;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ESP_LOGI(TAG, "Listening for UFH/Altherma packet_transport on UDP 18511");
    uint8_t buf[512];
    while (self->net_up_) {
      int n = ::recvfrom(sock, buf, sizeof(buf), 0, nullptr, nullptr);
      if (n > 0)
        self->udp_on_packet_(buf, (size_t) n);
      uint32_t now = millis();
      if (self->tx_dirty_ || self->last_tx_ms_ == 0 || now - self->last_tx_ms_ >= 15000UL)
        self->udp_send_(sock);
    }
    close(sock);
  }
}

#ifdef USE_UFH_ONE_WIRE
void TankComponent::ow_tick_(uint32_t now_ms) {
  if (ow_bus_ == nullptr)
    return;
  if (ow_scan_requested_) {
    ow_scan_requested_ = false;
    ow_bus_->search();
    ow_found_ = ow_bus_->get_devices();
    ow_found_temp_.assign(ow_found_.size(), NAN);
    ESP_LOGI(TAG, "1-Wire: %u device(s)", (unsigned) ow_found_.size());
  }
  if (ow_found_.empty())
    return;
  if (!ow_converting_) {
    if (now_ms - ow_last_ms_ < 30000UL)
      return;
    if (!ow_bus_->skip())
      return;
    ow_bus_->write8(0x44);
    ow_converting_ = true;
    ow_last_ms_ = now_ms;
    return;
  }
  if (now_ms - ow_last_ms_ < 800)
    return;
  ow_converting_ = false;
  for (size_t k = 0; k < ow_found_.size(); k++) {
    uint64_t addr = ow_found_[k];
    if ((addr & 0xFF) != 0x28 && (addr & 0xFF) != 0x22 && (addr & 0xFF) != 0x10)
      continue;
    if (!ow_bus_->select(addr))
      continue;
    ow_bus_->write8(0xBE);
    uint8_t b[9];
    for (auto &x : b)
      x = ow_bus_->read8();
    if (crc8_dallas(b, 8) != b[8])
      continue;
    int16_t raw = (int16_t) ((b[1] << 8) | b[0]);
    float t = (addr & 0xFF) == 0x10 ? raw / 2.0f : raw / 16.0f;
    if (t == 85.0f)
      continue;
    ow_found_temp_[k] = t;
    if (ow_tank_addr_ != 0 && ow_tank_addr_ == addr) {
      tank_temp_ = t;
      tank_temp_ms_ = now_ms ? now_ms : 1;
      tx_dirty_ = true;
    }
  }
}
#endif

}  // namespace ufh_tank
}  // namespace esphome
