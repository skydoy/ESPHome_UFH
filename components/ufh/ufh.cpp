#include "ufh.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cinttypes>

#include "esphome/core/application.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "esphome/components/json/json_util.h"
#include "esphome/components/network/util.h"
#ifdef USE_MQTT
#include "esphome/components/mqtt/mqtt_client.h"
#endif

namespace esphome {
namespace ufh {

static const char *const TAG = "ufh";
UFHComponent *global_ufh = nullptr;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static constexpr uint32_t PERSIST_MAGIC = 0x55464850;  // "UFHP"
static constexpr uint32_t HOUR_MS = 3600000UL;

static inline float x10(int16_t v) { return v / 10.0f; }
static inline int16_t to_x10(float v) { return (int16_t) lroundf(v * 10.0f); }

// ---------------------------------------------------------------------------------------------
// defaults
// ---------------------------------------------------------------------------------------------
void default_settings(Settings &s, uint8_t relay_count, bool active_low) {
  memset(&s, 0, sizeof(s));
  s.magic = SETTINGS_MAGIC;
  s.version = SETTINGS_VERSION;
  s.configured = 0;
  s.zone_count = relay_count > 2 ? relay_count - 2 : 1;
  s.relay_active_low = active_low ? 1 : 0;
  if (s.zone_count < relay_count)
    s.relay_role[s.zone_count] = ROLE_PUMP;
  if (s.zone_count + 1 < relay_count)
    s.relay_role[s.zone_count + 1] = ROLE_DEMAND;
  s.house_mode = HOUSE_HOME;
  s.pump_mode = PUMP_AUTO;
  s.pump_start_s = 120;
  s.pump_stop_s = 60;
  s.stale_min = 360;
  s.min_cycle_min = 10;
  s.hyst_on_x10 = 3;
  s.hyst_off_x10 = 1;
  s.holiday_x10 = 70;
  s.pv_delta_x10 = 10;
  s.pv_threshold_w = 500;
  s.tariff_s1 = 0;   // 00:00-08:00 cheap (PT bi-horário, roughly)
  s.tariff_e1 = 48;
  s.tariff_s2 = 132;  // 22:00-24:00
  s.tariff_e2 = 144;
  s.tariff_boost_x10 = 5;
  s.tariff_cut_x10 = 5;
  s.mqtt_port = 1883;
  strncpy(s.altherma_flow, "/sensor/leaving_water_temp__after_buh__r2t_", sizeof(s.altherma_flow) - 1);
  strncpy(s.altherma_return, "/sensor/inlet_water_temp__r4t_", sizeof(s.altherma_return) - 1);
  for (uint8_t i = 0; i < MAX_ZONES; i++) {
    ZoneCfg &z = s.zones[i];
    snprintf(z.name, sizeof(z.name), "Zone %u", i + 1);
    z.leader = 0;
    z.mode = ZONE_AUTO;
    z.comfort_x10 = 210;
    z.eco_x10 = 180;
    z.manual_x10 = 210;
    z.floor_max_x10 = 0;
    for (uint8_t d = 0; d < 7; d++) {
      if (d < 5) {
        z.sched[d] = {39, 51, 102, 135};  // 06:30-08:30, 17:00-22:30
      } else {
        z.sched[d] = {48, 138, 0, 0};  // 08:00-23:00
      }
    }
  }
}

// ---------------------------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------------------------
uint64_t parse_mac(const char *s) {
  uint64_t v = 0;
  int n = 0;
  for (const char *p = s; *p && n < 12; p++) {
    char c = *p;
    int d;
    if (c >= '0' && c <= '9')
      d = c - '0';
    else if (c >= 'a' && c <= 'f')
      d = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      d = c - 'A' + 10;
    else
      continue;
    v = (v << 4) | d;
    n++;
  }
  return n == 12 ? v : 0;
}

uint64_t parse_hex64(const char *s) {
  uint64_t v = 0;
  int n = 0;
  for (const char *p = s; *p && n < 16; p++) {
    char c = *p;
    int d;
    if (c >= '0' && c <= '9')
      d = c - '0';
    else if (c >= 'a' && c <= 'f')
      d = c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
      d = c - 'A' + 10;
    else
      continue;
    v = (v << 4) | d;
    n++;
  }
  return n == 16 ? v : 0;
}

std::string mac_to_str(uint64_t mac) {
  char b[18];
  snprintf(b, sizeof(b), "%02X:%02X:%02X:%02X:%02X:%02X", (unsigned) ((mac >> 40) & 0xFF),
           (unsigned) ((mac >> 32) & 0xFF), (unsigned) ((mac >> 24) & 0xFF), (unsigned) ((mac >> 16) & 0xFF),
           (unsigned) ((mac >> 8) & 0xFF), (unsigned) (mac & 0xFF));
  return b;
}

std::string hex64_to_str(uint64_t v) {
  char b[17];
  snprintf(b, sizeof(b), "%08" PRIx32 "%08" PRIx32, (uint32_t) (v >> 32), (uint32_t) v);
  return b;
}

static uint16_t slot_of_week(const ESPTime &t) {
  uint8_t dow0 = (t.day_of_week + 5) % 7;  // ESPTime: 1 = Sunday -> Monday = 0
  return dow0 * 144 + t.hour * 6 + t.minute / 10;
}

// ---------------------------------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------------------------------
void UFHComponent::setup() {
  global_ufh = this;
  // All relays off first (before we know the polarity, assume the default).
  bool al = default_active_low_;
  for (auto *r : relays_)
    r->set_state(al);

  this->load_settings_();
  for (uint8_t i = 0; i < relays_.size(); i++)
    this->write_relay_(i, false);

  this->pref_persist_ = global_preferences->make_preference<Persist>(fnv1_hash("ufh_persist_v1"));
  if (!this->pref_persist_.load(&this->p_) || this->p_.magic != PERSIST_MAGIC) {
    memset(&this->p_, 0, sizeof(this->p_));
    this->p_.magic = PERSIST_MAGIC;
    for (auto &r : this->p_.rate_c_per_h)
      r = 1.0f;
  }

  for (uint8_t i = 0; i < MAX_ZONES; i++) {
    ZoneCfg &z = s_.zones[i];
    ble_mac_[i][0] = z.room.type == SRC_BTHOME ? parse_mac(z.room.param) : 0;
    ble_mac_[i][1] = z.floor.type == SRC_BTHOME ? parse_mac(z.floor.param) : 0;
#ifdef USE_UFH_ONE_WIRE
    ow_addr_[i][0] = z.room.type == SRC_DS18B20 ? parse_hex64(z.room.param) : 0;
    ow_addr_[i][1] = z.floor.type == SRC_DS18B20 ? parse_hex64(z.floor.param) : 0;
#endif
  }

  this->apply_entities_();
  this->start_sources_();
  this->last_persist_ms_ = millis();
}

void UFHComponent::load_settings_() {
  this->pref_settings_ = global_preferences->make_preference<Settings>(fnv1_hash("ufh_settings_v1"));
  bool ok = this->pref_settings_.load(&this->s_);
  uint8_t n = relays_.size();
  if (!ok || s_.magic != SETTINGS_MAGIC || s_.version != SETTINGS_VERSION) {
    ESP_LOGW(TAG, "No settings stored: using defaults (open the device web page to run the wizard)");
    default_settings(s_, n, default_active_low_);
  }
  if (s_.zone_count < 1)
    s_.zone_count = 1;
  if (s_.zone_count > n)
    s_.zone_count = n;
  // make strings safe
  for (auto &z : s_.zones) {
    z.name[sizeof(z.name) - 1] = 0;
    z.room.param[sizeof(z.room.param) - 1] = 0;
    z.room.key[sizeof(z.room.key) - 1] = 0;
    z.floor.param[sizeof(z.floor.param) - 1] = 0;
    z.floor.key[sizeof(z.floor.key) - 1] = 0;
    if (z.leader > MAX_ZONES)
      z.leader = 0;
  }
  s_.mqtt_host[sizeof(s_.mqtt_host) - 1] = 0;
  s_.mqtt_user[sizeof(s_.mqtt_user) - 1] = 0;
  s_.mqtt_pass[sizeof(s_.mqtt_pass) - 1] = 0;
  s_.pv_topic[sizeof(s_.pv_topic) - 1] = 0;
  s_.pv_key[sizeof(s_.pv_key) - 1] = 0;
  s_.altherma_host[sizeof(s_.altherma_host) - 1] = 0;
  s_.altherma_flow[sizeof(s_.altherma_flow) - 1] = 0;
  s_.altherma_return[sizeof(s_.altherma_return) - 1] = 0;
}

void UFHComponent::save_settings_() {
  this->pref_settings_.save(&this->s_);
  global_preferences->sync();
}

void UFHComponent::save_persist_() { this->pref_persist_.save(&this->p_); }

uint8_t UFHComponent::effective_zone_count_() const {
  uint8_t n = s_.zone_count;
  if (n > relays_.size())
    n = relays_.size();
  return n;
}

void UFHComponent::apply_entities_() {
  uint8_t zc = this->effective_zone_count_();
  for (uint8_t i = 0; i < MAX_ZONES; i++) {
    ZoneRt &r = zr_[i];
    const char *n = s_.zones[i].name[0] ? s_.zones[i].name : "Zone";
    r.name_climate = n;
    r.name_heating = std::string(n) + " Heating";
    r.name_today = std::string(n) + " Runtime Today";
    r.name_total = std::string(n) + " Runtime Total";
    r.name_duty = std::string(n) + " Duty 24h";
    r.name_temp = std::string(n) + " Temperature";
  }
  for (auto *c : climates_) {
    uint8_t i = c->zone() - 1;
    if (i >= MAX_ZONES)
      continue;
    bool hide = (i >= zc) || (s_.zones[i].leader != 0);
    if (!hide)
      c->rename(zr_[i].name_climate.c_str());
    c->set_internal(hide);
  }
  for (auto *b : zone_bins_) {
    uint8_t i = b->zone() - 1;
    if (i >= MAX_ZONES)
      continue;
    bool hide = i >= zc;
    if (!hide)
      b->rename(zr_[i].name_heating.c_str());
    b->set_internal(hide);
  }
  for (auto *s : zone_sensors_) {
    uint8_t i = s->zone() - 1;
    if (i >= MAX_ZONES)
      continue;
    bool hide = i >= zc;
    if (s->kind() == ZS_TEMPERATURE && s_.zones[i].leader != 0)
      hide = true;
    if (!hide) {
      switch (s->kind()) {
        case ZS_RUNTIME_TODAY:
          s->rename(zr_[i].name_today.c_str());
          break;
        case ZS_RUNTIME_TOTAL:
          s->rename(zr_[i].name_total.c_str());
          break;
        case ZS_DUTY_24H:
          s->rename(zr_[i].name_duty.c_str());
          break;
        default:
          s->rename(zr_[i].name_temp.c_str());
          break;
      }
    }
    s->set_internal(hide);
  }
}

void UFHComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "UFH controller (%s):", board_name_);
  ESP_LOGCONFIG(TAG, "  Relays: %u, zones: %u, configured: %s, relays active %s", (unsigned) relays_.size(),
                (unsigned) s_.zone_count, YESNO(s_.configured), s_.relay_active_low ? "LOW" : "HIGH");
  for (uint8_t i = s_.zone_count; i < relays_.size(); i++) {
    static const char *const ROLES[] = {"-", "pump", "heat demand", "pump + heat demand"};
    ESP_LOGCONFIG(TAG, "  Relay %u: %s", i + 1, ROLES[s_.relay_role[i] & 3]);
  }
}

void UFHComponent::on_shutdown() {
  for (uint8_t i = 0; i < relays_.size(); i++)
    this->write_relay_(i, false);
  this->save_persist_();
}

void UFHComponent::write_relay_(uint8_t idx, bool on) {
  if (idx >= relays_.size())
    return;
  relay_state_[idx] = on;
  relays_[idx]->set_state(s_.relay_active_low ? !on : on);
}

// ---------------------------------------------------------------------------------------------
// readings
// ---------------------------------------------------------------------------------------------
void UFHComponent::web_reading(const PendingReading &r) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (inbox_.size() < 64)
    inbox_.push_back(r);
}

void UFHComponent::accept_reading_(const PendingReading &r, uint32_t now_ms) {
  if (r.zone < 1 || r.zone > MAX_ZONES || std::isnan(r.temp) || r.temp < -40 || r.temp > 90)
    return;
  uint8_t i = r.zone - 1;
  ZoneRt &z = zr_[i];
  if (r.floor) {
    z.floor_temp = r.temp;
    z.floor_ms = now_ms;
    ESP_LOGD(TAG, "Zone %u floor: %.1f °C", r.zone, r.temp);
    return;
  }
  // window detection: a fast drop (>= 1 °C within 10 min) closes the zone for 30 min
  if (s_.opt_window) {
    if (std::isnan(z.ref_temp) || now_ms - z.ref_ms > 600000UL || r.temp > z.ref_temp) {
      z.ref_temp = r.temp;
      z.ref_ms = now_ms;
    } else if (z.ref_temp - r.temp >= 1.0f) {
      z.window_until_ms = now_ms + 1800000UL;
      z.ref_temp = NAN;
      ESP_LOGI(TAG, "Zone %u: window open? temperature fell fast, closing for 30 min", r.zone);
    }
  }
  z.temp = r.temp;
  z.has_temp = true;
  z.temp_ms = now_ms;
  if (time_ != nullptr) {
    ESPTime t = time_->now();
    if (t.is_valid()) {
      p_.last_temp[i] = r.temp;
      p_.last_epoch[i] = t.timestamp;
    }
  }
  if (!std::isnan(r.hum))
    z.hum = r.hum;
  if (r.batt >= 0)
    z.batt = r.batt;
  ESP_LOGD(TAG, "Zone %u: %.1f °C", r.zone, r.temp);
}

void UFHComponent::ha_set_zone_temperature(int zone, float t) {
  PendingReading r{(uint8_t) zone, false, t, NAN, -1};
  this->web_reading(r);
}
void UFHComponent::ha_set_floor_temperature(int zone, float t) {
  PendingReading r{(uint8_t) zone, true, t, NAN, -1};
  this->web_reading(r);
}
void UFHComponent::ha_set_pv_surplus(bool on) {
  pv_surplus_ = on;
  pv_ms_ = millis();
}

// ---------------------------------------------------------------------------------------------
// modes / control
// ---------------------------------------------------------------------------------------------
void UFHComponent::set_house_mode(uint8_t m) {
  if (m > HOUSE_SUMMER)
    return;
  s_.house_mode = m;
  this->pref_settings_.save(&s_);
  ESP_LOGI(TAG, "House mode: %u", m);
}

void UFHComponent::set_pump_mode(uint8_t m) {
  if (m > PUMP_OFF)
    return;
  s_.pump_mode = m;
  this->pref_settings_.save(&s_);
}

void UFHComponent::all_zones_off() {
  for (uint8_t i = 0; i < MAX_ZONES; i++)
    s_.zones[i].mode = ZONE_OFF;
  this->pref_settings_.save(&s_);
  ESP_LOGI(TAG, "All zones off");
}

void UFHComponent::ha_set_zone_mode(int zone, int mode) {
  if (zone < 1 || zone > MAX_ZONES || mode < 0 || mode > ZONE_HEAT)
    return;
  s_.zones[zone - 1].mode = mode;
  zr_[zone - 1].override_active = false;
  this->pref_settings_.save(&s_);
}

void UFHComponent::ha_set_zone_target(int zone, float t) {
  if (zone < 1 || zone > MAX_ZONES || std::isnan(t))
    return;
  uint8_t i = zone - 1;
  ZoneCfg &z = s_.zones[i];
  t = std::clamp(t, 5.0f, 30.0f);
  if (z.mode == ZONE_AUTO) {
    ESPTime now = time_ != nullptr ? time_->now() : ESPTime{};
    zr_[i].override_active = true;
    zr_[i].override_x10 = to_x10(t);
    zr_[i].override_until_slot = now.is_valid() ? next_switch_slot_(z, slot_of_week(now)) : 0xFFFF;
    ESP_LOGI(TAG, "Zone %d: %.1f °C until the next schedule change", zone, t);
  } else {
    z.manual_x10 = to_x10(t);
    if (z.mode == ZONE_OFF)
      z.mode = ZONE_HEAT;
    this->pref_settings_.save(&s_);
  }
}

void UFHComponent::climate_control(uint8_t zone, const climate::ClimateCall &call) {
  if (zone < 1 || zone > MAX_ZONES)
    return;
  if (call.get_mode().has_value()) {
    climate::ClimateMode m = *call.get_mode();
    int zm = m == climate::CLIMATE_MODE_OFF ? ZONE_OFF : (m == climate::CLIMATE_MODE_AUTO ? ZONE_AUTO : ZONE_HEAT);
    this->ha_set_zone_mode(zone, zm);
  }
  if (call.get_target_temperature().has_value())
    this->ha_set_zone_target(zone, *call.get_target_temperature());
}

climate::ClimateTraits UFHZoneClimate::traits() {
  climate::ClimateTraits t;
  t.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE | climate::CLIMATE_SUPPORTS_ACTION);
  t.add_supported_mode(climate::CLIMATE_MODE_OFF);
  t.add_supported_mode(climate::CLIMATE_MODE_HEAT);
  t.add_supported_mode(climate::CLIMATE_MODE_AUTO);
  t.set_visual_min_temperature(5);
  t.set_visual_max_temperature(30);
  t.set_visual_target_temperature_step(0.5f);
  t.set_visual_current_temperature_step(0.1f);
  return t;
}

void UFHZoneClimate::control(const climate::ClimateCall &call) {
  if (parent_ != nullptr)
    parent_->climate_control(zone_, call);
}

// ---------------------------------------------------------------------------------------------
// schedule
// ---------------------------------------------------------------------------------------------
bool UFHComponent::in_comfort_(const ZoneCfg &z, uint16_t sow) const {
  uint8_t d = sow / 144, s = sow % 144;
  const DaySchedule &ds = z.sched[d];
  if (ds.s1 != ds.e1 && s >= ds.s1 && s < ds.e1)
    return true;
  if (ds.s2 != ds.e2 && s >= ds.s2 && s < ds.e2)
    return true;
  return false;
}

int UFHComponent::minutes_to_next_comfort_(const ZoneCfg &z, uint16_t sow) const {
  for (int k = 1; k <= 48; k++) {  // look 8 h ahead
    uint16_t s = (sow + k) % (7 * 144);
    if (in_comfort_(z, s))
      return k * 10;
  }
  return -1;
}

uint16_t UFHComponent::next_switch_slot_(const ZoneCfg &z, uint16_t sow) const {
  bool c = in_comfort_(z, sow);
  for (int k = 1; k <= 7 * 144; k++) {
    uint16_t s = (sow + k) % (7 * 144);
    if (in_comfort_(z, s) != c)
      return s;
  }
  return 0xFFFF;
}

float UFHComponent::tariff_offset_(const ESPTime &now) const {
  if (!s_.opt_tariff || !now.is_valid())
    return 0;
  uint8_t s = now.hour * 6 + now.minute / 10;
  bool cheap = (s_.tariff_s1 != s_.tariff_e1 && s >= s_.tariff_s1 && s < s_.tariff_e1) ||
               (s_.tariff_s2 != s_.tariff_e2 && s >= s_.tariff_s2 && s < s_.tariff_e2);
  return cheap ? x10(s_.tariff_boost_x10) : -x10(s_.tariff_cut_x10);
}

float UFHComponent::target_for_(uint8_t i, const ESPTime &now) {
  const ZoneCfg &z = s_.zones[i];
  ZoneRt &r = zr_[i];
  r.comfort_now = false;
  r.early_start = false;
  if (s_.house_mode == HOUSE_SUMMER || z.mode == ZONE_OFF)
    return NAN;
  if (s_.house_mode == HOUSE_HOLIDAY)
    return x10(s_.holiday_x10);
  float t;
  if (z.mode == ZONE_HEAT) {
    t = x10(z.manual_x10);
  } else {
    bool comfort = true;  // no clock yet: stay on the comfortable side
    if (now.is_valid()) {
      uint16_t sow = slot_of_week(now);
      comfort = in_comfort_(z, sow);
      if (r.override_active && sow == r.override_until_slot)
        r.override_active = false;
      if (!comfort && s_.opt_optimum_start && r.has_temp) {
        int mins = minutes_to_next_comfort_(z, sow);
        float need_c = x10(z.comfort_x10) - r.temp;
        if (mins > 0 && need_c > 0) {
          float rate = std::clamp(p_.rate_c_per_h[i], 0.1f, 5.0f);
          float need_min = need_c / rate * 60.0f;
          if (need_min > 240)
            need_min = 240;
          if (mins <= need_min) {
            comfort = true;
            r.early_start = true;
          }
        }
      }
    }
    r.comfort_now = comfort;
    t = x10(comfort ? z.comfort_x10 : z.eco_x10);
    if (r.override_active)
      t = x10(r.override_x10);
  }
  if (s_.house_mode == HOUSE_AWAY)
    t = std::min(t, x10(z.eco_x10));
  if (s_.house_mode == HOUSE_HOME) {
    t += this->tariff_offset_(now);
    if (s_.opt_pv && z.pv_boost && pv_surplus_)
      t += x10(s_.pv_delta_x10);
  }
  return t;
}

// ---------------------------------------------------------------------------------------------
// main loop
// ---------------------------------------------------------------------------------------------
void UFHComponent::loop() {
  uint32_t now_ms = millis();

  if (!relays_rewritten_) {
    // outputs that were set up after us may have reset their pins: write every relay again
    relays_rewritten_ = true;
    for (uint8_t i = 0; i < relays_.size(); i++)
      this->write_relay_(i, relay_state_[i]);
  }
  if (!web_registered_ && web_base_ != nullptr && network::is_connected()) {
    web_registered_ = true;
    register_web_handler(this, web_base_);
  }

  // apply wizard config (from the HTTP task)
  std::string cfg;
  {
    std::lock_guard<std::mutex> lock(mtx_);
    while (!inbox_.empty()) {
      this->accept_reading_(inbox_.front(), now_ms);
      inbox_.pop_front();
    }
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
      this->save_persist_();
      global_preferences->sync();
      App.safe_reboot();
    }
  }

#ifdef USE_UFH_ONE_WIRE
  this->ow_tick_(now_ms);
#endif

  if (now_ms - last_tick_ms_ < 1000)
    return;
  last_tick_ms_ = now_ms;
  std::lock_guard<std::mutex> lock(mtx_);
  this->tick_();
}

void UFHComponent::tick_() {
  uint32_t now_ms = millis();
  ESPTime now = time_ != nullptr ? time_->now() : ESPTime{};
  uint8_t zc = this->effective_zone_count_();

#ifdef USE_MQTT
  // MQTT starts once the network is up (the client is set up after this component)
  if (!mqtt_started_ && network::is_connected() && now_ms > 15000) {
    mqtt_started_ = true;
    this->mqtt_subscribe_();
  }
#endif

  // after a restart: reuse the last readings while they are not older than the stale limit
  if (!restored_ && now.is_valid()) {
    restored_ = true;
    for (uint8_t i = 0; i < MAX_ZONES; i++) {
      if (zr_[i].has_temp || p_.last_epoch[i] == 0 || now.timestamp < (time_t) p_.last_epoch[i])
        continue;
      uint32_t age_s = now.timestamp - p_.last_epoch[i];
      if (age_s >= (uint32_t) s_.stale_min * 60UL)
        continue;
      zr_[i].temp = p_.last_temp[i];
      zr_[i].has_temp = true;
      zr_[i].temp_ms = now_ms - std::min<uint32_t>(age_s * 1000UL, now_ms);
    }
  }

  for (uint8_t i = 0; i < zc; i++)
    if (s_.zones[i].leader == 0)
      this->update_zone_(i, now_ms, now);
  this->update_outputs_(now_ms, now);
  this->per_second_(now_ms, now);
  this->publish_entities_();

  if (now_ms - last_persist_ms_ > HOUR_MS) {
    last_persist_ms_ = now_ms;
    this->save_persist_();
  }
}

void UFHComponent::update_zone_(uint8_t i, uint32_t now_ms, const ESPTime &now) {
  ZoneCfg &z = s_.zones[i];
  ZoneRt &r = zr_[i];
  r.warn = 0;
  float target = this->target_for_(i, now);
  r.target = target;

  bool want = false;
  bool forced = false;  // switch off immediately (ignores the minimum cycle)
  if (!s_.configured || std::isnan(target)) {
    forced = true;
  } else if (z.room.type == SRC_NONE) {
    // no sensor: Heat = always on; Auto = on in Comfort periods
    want = z.mode == ZONE_HEAT || (z.mode == ZONE_AUTO && r.comfort_now && s_.house_mode == HOUSE_HOME);
    r.warn |= WARN_NO_SENSOR;
  } else {
    bool fresh = r.has_temp && (now_ms - r.temp_ms) < (uint32_t) s_.stale_min * 60000UL;
    if (!fresh) {
      forced = true;
      r.warn |= WARN_STALE;
    } else if (r.window_until_ms != 0 && (int32_t) (r.window_until_ms - now_ms) > 0) {
      forced = true;
      r.warn |= WARN_WINDOW;
    } else {
      r.window_until_ms = 0;
      float on_below = target - x10(s_.hyst_on_x10);
      float off_above = target + x10(s_.hyst_off_x10);
      want = r.demand ? (r.temp < off_above) : (r.temp <= on_below);
    }
  }

  // floor limit
  if (z.floor_max_x10 > 0 && z.floor.type != SRC_NONE) {
    bool ffresh = !std::isnan(r.floor_temp) && (now_ms - r.floor_ms) < (uint32_t) s_.stale_min * 60000UL;
    if (ffresh) {
      if (r.floor_temp >= x10(z.floor_max_x10))
        r.floor_block = true;
      else if (r.floor_temp <= x10(z.floor_max_x10) - 1.0f)
        r.floor_block = false;
    } else {
      r.floor_block = false;
    }
    if (r.floor_block) {
      want = false;
      forced = true;
      r.warn |= WARN_FLOOR;
    }
  }
  if (r.batt >= 0 && r.batt < 20)
    r.warn |= WARN_BATTERY;

  r.demand = want;
  if (want != r.relay) {
    bool cycle_ok = (now_ms - r.relay_changed_ms) >= (uint32_t) s_.min_cycle_min * 60000UL || r.relay_changed_ms == 0;
    if ((!want && forced) || cycle_ok) {
      // learning: heating rate when the target was reached
      if (!want && r.relay && !forced && r.has_temp && !std::isnan(r.heat_start_temp)) {
        float hours = (now_ms - r.heat_start_ms) / 3600000.0f;
        float rise = r.temp - r.heat_start_temp;
        if (hours > 0.5f && rise >= 0.3f) {
          float rate = rise / hours;
          p_.rate_c_per_h[i] = std::clamp(0.8f * p_.rate_c_per_h[i] + 0.2f * rate, 0.1f, 5.0f);
          ESP_LOGI(TAG, "Zone %u: learned heating rate %.2f °C/h", i + 1, p_.rate_c_per_h[i]);
        }
      }
      r.relay = want;
      r.relay_changed_ms = now_ms;
      if (want) {
        r.heat_start_ms = now_ms;
        r.heat_start_temp = r.has_temp ? r.temp : NAN;
      }
      ESP_LOGI(TAG, "Zone %u (%s): %s", i + 1, z.name, want ? "heating" : "closed");
    }
  }
  // not heating up: relay on for 4 h and less than 0.2 °C gained
  if (r.relay && r.has_temp && !std::isnan(r.heat_start_temp) && now_ms - r.heat_start_ms > 4 * HOUR_MS &&
      r.temp - r.heat_start_temp < 0.2f)
    r.warn |= WARN_NOT_HEATING;
}

void UFHComponent::update_outputs_(uint32_t now_ms, const ESPTime &now) {
  uint8_t zc = this->effective_zone_count_();
  // followers copy their leader
  for (uint8_t i = 0; i < zc; i++) {
    uint8_t L = s_.zones[i].leader;
    if (L != 0 && L <= zc && L - 1 != i && s_.zones[L - 1].leader == 0) {
      zr_[i].relay = zr_[L - 1].relay;
      zr_[i].demand = zr_[L - 1].demand;
      zr_[i].target = zr_[L - 1].target;
      zr_[i].warn = 0;
    } else if (L != 0) {
      zr_[i].relay = false;  // invalid leader
    }
  }

  this->exercise_tick_(now_ms, now);

  bool any = false;
  for (uint8_t i = 0; i < zc; i++) {
    bool on = zr_[i].relay;
    if (ex_state_ != 0) {
      bool open_all = s_.opt_valve_exercise;
      on = open_all ? true : (i == 0);
    }
    if (relay_state_[i] != on)
      this->write_relay_(i, on);
    if (zr_[i].relay)
      any = true;
  }

  if (any && !any_relay_)
    any_since_ms_ = now_ms;
  if (!any && any_relay_)
    none_since_ms_ = now_ms;
  any_relay_ = any;
  if (any && now.is_valid())
    p_.last_demand_epoch = now.timestamp;

  bool pump = false;
  if (ex_state_ == 2) {
    pump = true;
  } else if (ex_state_ != 0) {
    pump = false;
  } else if (s_.house_mode == HOUSE_SUMMER) {
    pump = false;
  } else if (s_.pump_mode == PUMP_ON) {
    pump = true;
  } else if (s_.pump_mode == PUMP_OFF) {
    pump = false;
  } else if (any) {
    pump = pump_out_ || (now_ms - any_since_ms_) >= (uint32_t) s_.pump_start_s * 1000UL;
  } else {
    pump = pump_out_ && (now_ms - none_since_ms_) < (uint32_t) s_.pump_stop_s * 1000UL;
  }
  bool demand = any && s_.house_mode != HOUSE_SUMMER && ex_state_ == 0 &&
                (demand_out_ || (now_ms - any_since_ms_) >= (uint32_t) s_.pump_start_s * 1000UL);

  bool has_pump = false, has_demand = false;
  for (uint8_t k = zc; k < relays_.size(); k++) {
    bool on = false;
    switch (s_.relay_role[k]) {
      case ROLE_PUMP:
        on = pump;
        has_pump = true;
        break;
      case ROLE_DEMAND:
        on = demand;
        has_demand = true;
        break;
      case ROLE_PUMP_DEMAND:
        on = pump || demand;
        has_pump = has_demand = true;
        break;
      default:
        on = false;
    }
    if (relay_state_[k] != on)
      this->write_relay_(k, on);
  }
  pump_out_ = has_pump && pump;
  demand_out_ = has_demand ? demand : (any && s_.house_mode != HOUSE_SUMMER);

  // text: active zones
  std::string t;
  for (uint8_t i = 0; i < zc; i++) {
    if (!zr_[i].relay)
      continue;
    if (!t.empty())
      t += ", ";
    t += s_.zones[i].name;
  }
  active_text_ = t.empty() ? std::string("-") : t;
}

void UFHComponent::exercise_tick_(uint32_t now_ms, const ESPTime &now) {
  if (!s_.opt_pump_exercise && !s_.opt_valve_exercise) {
    ex_state_ = 0;
    return;
  }
  uint32_t open_s = s_.pump_start_s < 60 ? 60 : s_.pump_start_s;
  switch (ex_state_) {
    case 0: {
      if (!now.is_valid() || any_relay_)
        return;
      uint8_t dow0 = (now.day_of_week + 5) % 7;
      if (dow0 != 0 || now.hour != 10 || now.minute > 5)
        return;
      if (now.timestamp - p_.last_exercise_epoch < 6 * 86400UL)
        return;
      if (p_.last_demand_epoch != 0 && now.timestamp - p_.last_demand_epoch < 86400UL)
        return;
      ex_state_ = 1;
      ex_ms_ = now_ms;
      p_.last_exercise_epoch = now.timestamp;
      ESP_LOGI(TAG, "Weekly anti-seize: opening %s", s_.opt_valve_exercise ? "all zones" : "zone 1");
      break;
    }
    case 1:  // valves opening
      if (now_ms - ex_ms_ >= open_s * 1000UL) {
        ex_state_ = s_.opt_pump_exercise ? 2 : 3;
        ex_ms_ = now_ms;
      }
      break;
    case 2:  // pump 60 s
      if (now_ms - ex_ms_ >= 60000UL) {
        ex_state_ = 3;
        ex_ms_ = now_ms;
      }
      break;
    case 3:  // keep the valves open a little longer, then close
      if (now_ms - ex_ms_ >= (s_.opt_valve_exercise ? 120000UL : 0UL)) {
        ex_state_ = 0;
        ESP_LOGI(TAG, "Weekly anti-seize: done");
      }
      break;
  }
}

void UFHComponent::per_second_(uint32_t now_ms, const ESPTime &now) {
  if (now_ms - last_sec_ms_ < 1000)
    return;
  uint32_t secs = last_sec_ms_ == 0 ? 1 : (now_ms - last_sec_ms_) / 1000;
  last_sec_ms_ += secs * 1000;
  if (secs > 5)
    secs = 1;
  int hour = now.is_valid() ? now.hour : (int) ((now_ms / HOUR_MS) % 24);
  int day = now.is_valid() ? now.day_of_year : (int) (now_ms / (24 * HOUR_MS));
  if (hour != last_hour_) {
    last_hour_ = hour;
    for (auto &z : zr_)
      z.hour_s[hour] = 0;
  }
  if (day != last_day_) {
    if (last_day_ != -1) {
      for (auto &z : zr_)
        z.today_s = 0;
      pump_today_s_ = 0;
    }
    last_day_ = day;
  }
  uint8_t zc = this->effective_zone_count_();
  for (uint8_t i = 0; i < zc; i++) {
    if (!relay_state_[i])
      continue;
    zr_[i].today_s += secs;
    p_.zone_total_s[i] += secs;
    zr_[i].hour_s[hour] += secs;
  }
  if (pump_out_) {
    pump_today_s_ += secs;
    p_.pump_total_s += secs;
  }

  // warnings text
  {
    std::string w;
    if (!s_.configured)
      w = "setup";
    for (uint8_t i = 0; i < zc; i++) {
      uint32_t bits = zr_[i].warn;
      if (bits & WARN_STALE)
        w += (w.empty() ? "" : "; ") + std::string(s_.zones[i].name) + ": sensor";
      if (bits & WARN_BATTERY)
        w += (w.empty() ? "" : "; ") + std::string(s_.zones[i].name) + ": battery";
      if (bits & WARN_NOT_HEATING)
        w += (w.empty() ? "" : "; ") + std::string(s_.zones[i].name) + ": not heating";
      if (bits & WARN_FLOOR)
        w += (w.empty() ? "" : "; ") + std::string(s_.zones[i].name) + ": floor limit";
    }
    if (pump_out_ && !any_relay_ && ex_state_ == 0 && s_.pump_mode == PUMP_ON)
      w += (w.empty() ? "" : "; ") + std::string("pump on, no zone open");
    warnings_text_ = w;
  }
}

void UFHComponent::publish_entities_() {
  static uint32_t last_slow = 0;
  uint32_t now_ms = millis();
  bool slow = now_ms - last_slow > 30000UL || last_slow == 0;
  if (slow)
    last_slow = now_ms;
  uint8_t zc = this->effective_zone_count_();

  for (auto *c : climates_) {
    uint8_t i = c->zone() - 1;
    if (i >= zc || c->is_internal())
      continue;
    const ZoneCfg &z = s_.zones[i];
    ZoneRt &r = zr_[i];
    climate::ClimateMode m = z.mode == ZONE_OFF   ? climate::CLIMATE_MODE_OFF
                             : z.mode == ZONE_AUTO ? climate::CLIMATE_MODE_AUTO
                                                   : climate::CLIMATE_MODE_HEAT;
    float tgt = std::isnan(r.target) ? x10(z.mode == ZONE_HEAT ? z.manual_x10 : z.eco_x10) : r.target;
    float cur = r.has_temp ? r.temp : NAN;
    climate::ClimateAction a = z.mode == ZONE_OFF ? climate::CLIMATE_ACTION_OFF
                               : r.relay          ? climate::CLIMATE_ACTION_HEATING
                                                  : climate::CLIMATE_ACTION_IDLE;
    bool changed = c->mode != m || c->action != a || std::fabs(c->target_temperature - tgt) > 0.01f ||
                   (std::isnan(c->current_temperature) != std::isnan(cur)) ||
                   (!std::isnan(cur) && std::fabs(c->current_temperature - cur) > 0.01f);
    if (changed) {
      c->mode = m;
      c->action = a;
      c->target_temperature = tgt;
      c->current_temperature = cur;
      c->publish_state();
    }
  }
  for (auto *b : zone_bins_) {
    uint8_t i = b->zone() - 1;
    if (i >= zc || b->is_internal())
      continue;
    b->publish_state(relay_state_[i]);
  }
  if (!slow)
    return;
  for (auto *s : zone_sensors_) {
    uint8_t i = s->zone() - 1;
    if (i >= zc || s->is_internal())
      continue;
    ZoneRt &r = zr_[i];
    switch (s->kind()) {
      case ZS_RUNTIME_TODAY:
        s->publish_state(r.today_s / 3600.0f);
        break;
      case ZS_RUNTIME_TOTAL:
        s->publish_state(p_.zone_total_s[i] / 3600.0f);
        break;
      case ZS_DUTY_24H: {
        uint32_t sum = 0;
        for (auto h : r.hour_s)
          sum += h;
        s->publish_state(sum / 864.0f);
        break;
      }
      default:
        if (r.has_temp)
          s->publish_state(r.temp);
        break;
    }
  }
}

// ---------------------------------------------------------------------------------------------
// sources
// ---------------------------------------------------------------------------------------------
void UFHComponent::start_sources_() {
  if (s_.opt_altherma && s_.altherma_host[0]) {
    snprintf(alt_url_flow_, sizeof(alt_url_flow_), "http://%s%s", s_.altherma_host, s_.altherma_flow);
    snprintf(alt_url_return_, sizeof(alt_url_return_), "http://%s%s", s_.altherma_host, s_.altherma_return);
    this->altherma_task_start_();
  }
}

#ifdef USE_MQTT
static bool json_path_number(JsonVariantConst v, const std::string &path, float &out) {
  size_t start = 0;
  while (start <= path.size()) {
    size_t dot = path.find('.', start);
    std::string k = path.substr(start, dot == std::string::npos ? std::string::npos : dot - start);
    if (!v.is<JsonObjectConst>())
      return false;
    v = v[k.c_str()];
    if (dot == std::string::npos)
      break;
    start = dot + 1;
  }
  if (v.is<float>() || v.is<int>()) {
    out = v.as<float>();
    return true;
  }
  if (v.is<bool>()) {
    out = v.as<bool>() ? 1 : 0;
    return true;
  }
  if (v.is<const char *>()) {
    const char *s = v.as<const char *>();
    if (!strcasecmp(s, "on") || !strcasecmp(s, "true")) {
      out = 1;
      return true;
    }
    if (!strcasecmp(s, "off") || !strcasecmp(s, "false")) {
      out = 0;
      return true;
    }
    char *end;
    out = strtof(s, &end);
    return end != s;
  }
  return false;
}

static bool payload_number(const std::string &key, const std::string &payload, float &out) {
  if (key.empty()) {
    if (!strcasecmp(payload.c_str(), "on") || !strcasecmp(payload.c_str(), "true")) {
      out = 1;
      return true;
    }
    if (!strcasecmp(payload.c_str(), "off") || !strcasecmp(payload.c_str(), "false")) {
      out = 0;
      return true;
    }
    char *end;
    out = strtof(payload.c_str(), &end);
    return end != payload.c_str();
  }
  JsonDocument doc;
  if (deserializeJson(doc, payload))
    return false;
  return json_path_number(doc.as<JsonVariantConst>(), key, out);
}

void UFHComponent::mqtt_payload_(int zone, bool floor, const std::string &key, const std::string &payload) {
  float v;
  if (!payload_number(key, payload, v))
    return;
  if (zone == 0) {  // solar surplus: on/off/true/false/1/0, or watts above the threshold
    pv_surplus_ = v == 1.0f || v > s_.pv_threshold_w;
    pv_ms_ = millis();
    return;
  }
  PendingReading r{(uint8_t) zone, floor, v, NAN, -1};
  this->web_reading(r);
}

void UFHComponent::mqtt_subscribe_() {
  auto *m = mqtt::global_mqtt_client;
  if (m == nullptr || s_.mqtt_host[0] == 0)
    return;
  bool needed = s_.opt_pv && s_.pv_topic[0];
  for (uint8_t i = 0; i < s_.zone_count; i++)
    if (s_.zones[i].room.type == SRC_MQTT || s_.zones[i].floor.type == SRC_MQTT)
      needed = true;
  if (!needed)
    return;
  m->set_broker_address(s_.mqtt_host);
  m->set_broker_port(s_.mqtt_port ? s_.mqtt_port : 1883);
  m->set_username(s_.mqtt_user);
  m->set_password(s_.mqtt_pass);
  for (uint8_t i = 0; i < s_.zone_count; i++) {
    for (int f = 0; f < 2; f++) {
      const SourceCfg &src = f ? s_.zones[i].floor : s_.zones[i].room;
      if (src.type != SRC_MQTT || !src.param[0])
        continue;
      int zone = i + 1;
      std::string key = src.key;
      bool floor = f;
      m->subscribe(src.param, [this, zone, floor, key](const std::string &topic, const std::string &payload) {
        this->mqtt_payload_(zone, floor, key, payload);
      });
    }
  }
  if (s_.opt_pv && s_.pv_topic[0]) {
    std::string key = s_.pv_key;
    m->subscribe(s_.pv_topic, [this, key](const std::string &topic, const std::string &payload) {
      this->mqtt_payload_(0, false, key, payload);
    });
  }
  m->enable();
  ESP_LOGI(TAG, "MQTT: broker %s", s_.mqtt_host);
}
#endif

#ifdef USE_UFH_BLE
bool UFHComponent::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  static const auto BTHOME = esp32_ble_tracker::ESPBTUUID::from_uint16(0xFCD2);
  for (const auto &sd : device.get_service_datas()) {
    if (!(sd.uuid == BTHOME))
      continue;
    float t = NAN, h = NAN;
    int8_t b = -1;
    if (!bthome_parse(sd.data.data(), sd.data.size(), t, h, b))
      return false;
    const uint8_t *a = device.address();
    uint64_t mac = 0;
    for (int k = 0; k < 6; k++)
      mac = (mac << 8) | a[k];
    uint32_t now_ms = millis();
    // remember recently seen devices (for the wizard)
    int slot = -1, oldest = 0;
    for (int k = 0; k < 12; k++) {
      if (ble_seen_[k].mac == mac) {
        slot = k;
        break;
      }
      if (ble_seen_[k].ms < ble_seen_[oldest].ms)
        oldest = k;
    }
    if (slot < 0)
      slot = oldest;
    ble_seen_[slot] = BleSeen{mac, t, h, b, (int8_t) device.get_rssi(), now_ms};
    if (std::isnan(t))
      return true;
    for (uint8_t i = 0; i < MAX_ZONES; i++) {
      for (int f = 0; f < 2; f++) {
        if (ble_mac_[i][f] == mac) {
          PendingReading r{(uint8_t) (i + 1), f == 1, t, f ? NAN : h, f ? (int8_t) -1 : b};
          this->web_reading(r);
        }
      }
    }
    return true;
  }
  return false;
}
#endif

#ifdef USE_UFH_ONE_WIRE
static uint8_t crc8_dallas(const uint8_t *d, uint8_t len) {
  uint8_t crc = 0;
  while (len--) {
    uint8_t in = *d++;
    for (uint8_t i = 8; i; i--) {
      uint8_t mix = (crc ^ in) & 0x01;
      crc >>= 1;
      if (mix)
        crc ^= 0x8C;
      in >>= 1;
    }
  }
  return crc;
}

void UFHComponent::ow_tick_(uint32_t now_ms) {
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
    ow_bus_->write8(0x44);  // convert T (all)
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
      continue;  // power-on value
    ow_found_temp_[k] = t;
    for (uint8_t i = 0; i < MAX_ZONES; i++)
      for (int f = 0; f < 2; f++)
        if (ow_addr_[i][f] == addr) {
          PendingReading r{(uint8_t) (i + 1), f == 1, t, NAN, -1};
          this->web_reading(r);
        }
  }
}
#endif

// ---------------------------------------------------------------------------------------------
// BTHome v2 (unencrypted) parser: temperature, humidity, battery
// ---------------------------------------------------------------------------------------------
static int bthome_len(uint8_t id) {
  switch (id) {
    case 0x00: case 0x01: case 0x09: case 0x0F: case 0x10: case 0x11: case 0x2E: case 0x2F: case 0x3A:
    case 0x46: case 0x57: case 0x58: case 0x59: case 0x60:
      return 1;
    case 0x02: case 0x03: case 0x06: case 0x07: case 0x08: case 0x0C: case 0x0D: case 0x0E: case 0x12:
    case 0x13: case 0x14: case 0x3C: case 0x3D: case 0x3F: case 0x40: case 0x41: case 0x43: case 0x44:
    case 0x45: case 0x47: case 0x48: case 0x49: case 0x4A: case 0x51: case 0x52: case 0x5A: case 0x5D:
    case 0x5E: case 0x5F: case 0x61: case 0xF0:
      return 2;
    case 0x04: case 0x05: case 0x0A: case 0x0B: case 0x42: case 0x4B: case 0xF2:
      return 3;
    case 0x3E: case 0x4C: case 0x4D: case 0x4E: case 0x4F: case 0x50: case 0x5B: case 0x5C: case 0xF1:
      return 4;
    default:
      if (id >= 0x15 && id <= 0x2D)
        return 1;  // binary sensors
      return -1;
  }
}

bool bthome_parse(const uint8_t *d, size_t len, float &temp, float &hum, int8_t &batt) {
  if (len < 2)
    return false;
  uint8_t info = d[0];
  if (info & 0x01)
    return false;  // encrypted: not supported
  if (((info >> 5) & 0x07) != 2)
    return false;  // BTHome v2 only
  size_t i = 1;
  bool any = false;
  while (i < len) {
    uint8_t id = d[i++];
    if (id == 0x53 || id == 0x54) {  // text / raw: length-prefixed
      if (i >= len)
        break;
      i += 1 + d[i];
      continue;
    }
    int n = bthome_len(id);
    if (n < 0 || i + n > len)
      break;
    const uint8_t *v = d + i;
    switch (id) {
      case 0x01:
        batt = (int8_t) v[0];
        break;
      case 0x02:
        temp = (int16_t) (v[0] | (v[1] << 8)) * 0.01f;
        any = true;
        break;
      case 0x45:
        temp = (int16_t) (v[0] | (v[1] << 8)) * 0.1f;
        any = true;
        break;
      case 0x57:
        temp = (int8_t) v[0];
        any = true;
        break;
      case 0x03:
        hum = (uint16_t) (v[0] | (v[1] << 8)) * 0.01f;
        break;
      case 0x2E:
        hum = v[0];
        break;
      default:
        break;
    }
    i += n;
  }
  return any || batt >= 0;
}

}  // namespace ufh
}  // namespace esphome
