// HTTP endpoints of the UFH controller (served by the device web server, port 80):
//   GET  /ufh/temp?zone=N&t=21.5[&h=55][&b=80][&floor=1]  temperature from Shelly H&T actions etc.
//   GET  /ufh/state                                        live state (web page mockup)
//   GET  /ufh/config                                       settings (wizard)
//   POST /ufh/config   (application/json)                  save settings
//   POST /ufh/restart                                      restart the device
//   GET  /ufh/log                                           event log (last 30, newest first)
//   GET  /ufh/scan                                          search the 1-Wire bus again
// Runs in the HTTP server task: everything shared with loop() is guarded by mtx_.
#include "ufh.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <strings.h>

#include "esphome/core/application.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/json/json_util.h"
#include "esphome/components/network/util.h"

#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace esphome {
namespace ufh {

static const char *const TAG = "ufh.web";

static inline float x10(int16_t v) { return v / 10.0f; }
static inline int16_t to_x10(float v) { return (int16_t) lroundf(v * 10.0f); }

static void put_num(JsonObject o, const char *k, float v) {
  if (std::isnan(v))
    o[k] = nullptr;
  else
    o[k] = roundf(v * 10.0f) / 10.0f;
}

static void copy_str(char *dst, size_t n, JsonVariantConst v) {
  if (!v.is<const char *>())
    return;
  strncpy(dst, v.as<const char *>(), n - 1);
  dst[n - 1] = 0;
}

// ---------------------------------------------------------------------------------------------
class UFHWebHandler : public AsyncWebHandler {
 public:
  explicit UFHWebHandler(UFHComponent *p) : p_(p) {}

  bool canHandle(AsyncWebServerRequest *request) const override {
    char buf[AsyncWebServerRequest::URL_BUF_SIZE];
    StringRef url = request->url_to(buf);
    return url.size() >= 5 && strncmp(url.c_str(), "/ufh/", 5) == 0;
  }

  void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) override {
    if (index == 0) {
      body_.clear();
      if (total > 12000)
        return;
      body_.reserve(total);
    }
    if (body_.size() + len <= 12000)
      body_.append((const char *) data, len);
  }

  void handleRequest(AsyncWebServerRequest *request) override {
    char buf[AsyncWebServerRequest::URL_BUF_SIZE];
    StringRef url = request->url_to(buf);
    std::string u(url.c_str(), url.size());
    bool post = request->method() == HTTP_POST;

    if (u == "/ufh/temp") {
      if (!request->hasParam("zone") || !request->hasParam("t")) {
        request->send(400, "text/plain", "zone and t required");
        return;
      }
      PendingReading r{};
      r.zone = (uint8_t) atoi(request->arg("zone").c_str());
      r.temp = strtof(request->arg("t").c_str(), nullptr);
      r.hum = request->hasParam("h") ? strtof(request->arg("h").c_str(), nullptr) : NAN;
      r.batt = request->hasParam("b") ? (int8_t) atoi(request->arg("b").c_str()) : -1;
      r.floor = request->hasParam("floor") && request->arg("floor") == "1";
      if (r.zone < 1 || r.zone > MAX_ZONES || std::isnan(r.temp)) {
        request->send(400, "text/plain", "bad zone or t");
        return;
      }
      p_->web_reading(r);
      request->send(200, "text/plain", "OK");
      return;
    }
    if (u == "/ufh/state") {
      std::string out = p_->web_state_json();
      request->send(200, "application/json", out.c_str());
      return;
    }
    if (u == "/ufh/log") {
      std::string out = p_->web_log_json();
      request->send(200, "application/json", out.c_str());
      return;
    }
    if (u == "/ufh/config" && !post) {
      std::string out = p_->web_config_json();
      request->send(200, "application/json", out.c_str());
      return;
    }
    if (u == "/ufh/config" && post) {
      JsonDocument doc;
      if (body_.empty() || deserializeJson(doc, body_)) {
        body_.clear();
        request->send(400, "application/json", "{\"ok\":false,\"error\":\"invalid JSON\"}");
        return;
      }
      {
        std::lock_guard<std::mutex> lock(p_->mtx_);
        p_->pending_cfg_.swap(body_);
        p_->cfg_pending_ = true;
      }
      body_.clear();
      request->send(200, "application/json", "{\"ok\":true}");
      return;
    }
    if (u == "/ufh/restart" && post) {
      p_->web_request_restart();
      request->send(200, "application/json", "{\"ok\":true}");
      return;
    }
    if (u == "/ufh/scan") {
      p_->web_ds18b20_scan();
      request->send(200, "application/json", "{\"ok\":true}");
      return;
    }
    request->send(404);
  }

  bool isRequestHandlerTrivial() const override { return false; }

 protected:
  UFHComponent *p_;
  std::string body_;
};

void register_web_handler(UFHComponent *p, web_server_base::WebServerBase *base) {
  base->init();
  base->add_handler(new UFHWebHandler(p));  // NOLINT(cppcoreguidelines-owning-memory)
}

// ---------------------------------------------------------------------------------------------
// JSON
// ---------------------------------------------------------------------------------------------
std::string UFHComponent::web_state_json() {
  std::lock_guard<std::mutex> lock(mtx_);
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  uint32_t now_ms = millis();
  root["configured"] = (bool) s_.configured;
  root["board"] = board_name_;
  root["relays"] = (int) relays_.size();
  root["zone_count"] = s_.zone_count;
  root["ext_count"] = this->effective_zone_count_() - this->local_zone_count_();
  root["house"] = s_.house_mode;
  root["pump_mode"] = s_.pump_mode;
  root["pump"] = pump_out_;
  root["demand"] = demand_out_;
  root["any"] = any_relay_;
  root["pv"] = pv_surplus_;
  root["exercise"] = ex_state_;
  root["warnings"] = warnings_text_;
  root["active_low"] = (bool) s_.relay_active_low;
  root["ps"] = s_.pump_start_s;
  root["pe"] = s_.pump_stop_s;
  if (any_relay_ && !pump_out_ && s_.pump_mode == PUMP_AUTO) {
    int32_t left = (int32_t) s_.pump_start_s - (int32_t) ((now_ms - any_since_ms_) / 1000);
    root["pump_in"] = left > 0 ? left : 0;
  }
  ESPTime now = time_ != nullptr ? time_->now() : ESPTime{};
  if (now.is_valid()) {
    char b[24];
    snprintf(b, sizeof(b), "%02d:%02d", now.hour, now.minute);
    root["time"] = b;
    root["dow"] = (now.day_of_week + 5) % 7;
  }
  JsonArray roles = root["roles"].to<JsonArray>();
  JsonArray rs = root["relay_on"].to<JsonArray>();
  for (uint8_t k = 0; k < relays_.size(); k++) {
    roles.add(k < s_.zone_count ? -1 : s_.relay_role[k]);
    rs.add(relay_state_[k]);
  }
  JsonArray zs = root["zones"].to<JsonArray>();
  uint8_t lc = this->local_zone_count_();
  for (uint8_t i = 0; i < this->effective_zone_count_(); i++) {
    const ZoneCfg &z = s_.zones[i];
    const ZoneRt &r = zr_[i];
    JsonObject o = zs.add<JsonObject>();
    o["n"] = z.name;
    o["m"] = z.mode;
    o["lead"] = z.leader;
    o["fm"] = (bool) (z.follow_main && z.leader == 0);
    o["src"] = z.room.type;
    put_num(o, "t", r.has_temp ? r.temp : NAN);
    put_num(o, "h", r.hum);
    put_num(o, "f", r.floor_temp);
    put_num(o, "tg", r.target);
    o["age"] = r.has_temp ? (int) ((now_ms - r.temp_ms) / 60000) : -1;
    o["b"] = r.batt;
    o["r"] = zone_out_[i];
    if (i >= lc) {
      o["ext"] = true;
      o["eok"] = (bool) ext_ok_[i - lc];
    }
    o["w"] = r.warn;
    o["early"] = r.early_start;
    o["estop"] = r.early_stop;
    o["ovr"] = r.override_active;
    o["today"] = r.today_s / 60;
    uint32_t sum = 0;
    for (auto h : r.hour_s)
      sum += h;
    o["duty"] = (int) (sum / 864);
    o["rate"] = roundf(p_.rate_c_per_h[i] * 100) / 100;
    o["cool"] = roundf(l_.cool_c_per_h[i] * 100) / 100;
    o["bst"] = r.boost_until_ms ? (int) ((r.boost_until_ms - now_ms) / 60000 + 1) : 0;
    JsonArray wk = o["wk"].to<JsonArray>();  // minutes heating, 6 days ago ... yesterday, today
    for (uint8_t k = 1; k <= 6; k++)
      wk.add(wk_.min[i][(wk_.idx + k) % 7]);
    wk.add(r.today_s / 60);
  }
  {
    ESPTime tnow = time_ != nullptr ? time_->now() : ESPTime{};
    float mt = this->main_target_(tnow);
    JsonObject mo = root["main"].to<JsonObject>();
    mo["m"] = m_.mode;
    put_num(mo, "tg", mt);
    mo["c"] = main_comfort_now_;
    mo["ovr"] = main_override_;
    mo["comfort"] = x10(m_.comfort_x10);
    mo["eco"] = x10(m_.eco_x10);
  }
  put_num(root, "out", this->outdoor_temp());
  put_num(root, "dt", this->flow_delta());
  JsonObject pmp = root["pump_h"].to<JsonObject>();
  pmp["today"] = pump_today_s_ / 60;
  pmp["total"] = p_.pump_total_s / 3600;
  if (s_.opt_altherma) {
    JsonObject a = root["altherma"].to<JsonObject>();
    put_num(a, "flow", this->flow_temp());
    put_num(a, "ret", this->return_temp());
    put_num(a, "freq", this->compressor_freq());
    a["src"] = this->altherma_source();
    a["age"] = this->altherma_age_s();
    a["fault"] = this->heat_pump_fault_state();
    char eo[8]{}, ei[8]{};
    this->altherma_error_text(true, eo, sizeof(eo));
    this->altherma_error_text(false, ei, sizeof(ei));
    a["err_out"] = eo;
    a["err_in"] = ei;
    a["provider"] = x_.altherma_provider;
    a["comp_zero"] = (bool) comp_zero_warn_;
    JsonArray seen = a["seen"].to<JsonArray>();
    {
      std::lock_guard<std::mutex> lock(alt_mtx_);
      for (uint8_t i = 0; i < alt_seen_n_; i++) {
        if (alt_seen_[i].name[0] == 0)
          continue;
        if (now_ms - alt_seen_[i].ms > 600000UL)
          continue;
        JsonObject o = seen.add<JsonObject>();
        o["name"] = alt_seen_[i].name;
        o["age"] = (int) ((now_ms - alt_seen_[i].ms) / 1000);
      }
    }
  }
  if (x_.opt_tank) {
    JsonObject tnk = root["tank"].to<JsonObject>();
    put_num(tnk, "t", this->tank_temp());
    tnk["pump"] = tank_pump_out_;
    tnk["heater"] = tank_heater_out_;
    tnk["manual"] = heater_manual_on_;
    tnk["provider"] = tank_link_.tank_provider;
    tnk["fresh"] = this->tank_udp_fresh();
    tnk["age"] = this->tank_age_s();
    put_num(tnk, "pwm", (float) tank_pwm_);
    tnk["remote"] = true;
    JsonArray seen = tnk["seen"].to<JsonArray>();
    {
      std::lock_guard<std::mutex> lock(alt_mtx_);
      for (uint8_t i = 0; i < tank_seen_n_; i++) {
        if (tank_seen_[i].name[0] == 0)
          continue;
        if (now_ms - tank_seen_[i].ms > 600000UL)
          continue;
        JsonObject o = seen.add<JsonObject>();
        o["name"] = tank_seen_[i].name;
        o["age"] = (int) ((now_ms - tank_seen_[i].ms) / 1000);
      }
    }
  }
  JsonArray ble = root["ble"].to<JsonArray>();
  for (const auto &b : ble_seen_) {
    if (b.mac == 0 || now_ms - b.ms > 600000UL)
      continue;
    JsonObject o = ble.add<JsonObject>();
    o["mac"] = mac_to_str(b.mac);
    put_num(o, "t", b.temp);
    put_num(o, "h", b.hum);
    o["b"] = b.batt;
    o["rssi"] = b.rssi;
  }
  JsonArray ow = root["ow"].to<JsonArray>();
#ifdef USE_UFH_ONE_WIRE
  for (size_t k = 0; k < ow_found_.size(); k++) {
    JsonObject o = ow.add<JsonObject>();
    o["a"] = hex64_to_str(ow_found_[k]);
    put_num(o, "t", k < ow_found_temp_.size() ? ow_found_temp_[k] : NAN);
  }
#endif
  std::string out;
  serializeJson(doc, out);
  return out;
}

static void src_to_json(JsonObject o, const SourceCfg &s) {
  o["type"] = s.type;
  o["param"] = s.param;
  o["key"] = s.key;
}

std::string UFHComponent::web_log_json() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  {
    std::lock_guard<std::mutex> lock(log_mtx_);
    for (uint8_t n = 0; n < log_count_; n++) {
      const EventEntry &e = log_[(log_head_ + 30 - 1 - n) % 30];
      JsonObject o = arr.add<JsonObject>();
      bool up = e.t & 0x80000000UL;
      o["t"] = up ? (e.t & 0x7FFFFFFFUL) : e.t;
      if (up)
        o["up"] = true;
      o["k"] = e.kind;
      if (e.zone)
        o["z"] = e.zone;
      if (e.a_x10 != INT16_MIN)
        o["a"] = e.a_x10 / 10.0f;
      if (e.b_x10 != INT16_MIN)
        o["b"] = e.b_x10 / 10.0f;
      if (e.txt[0])
        o["x"] = e.txt;
    }
  }
  std::string out;
  serializeJson(doc, out);
  return out;
}

std::string UFHComponent::web_config_json() {
  std::lock_guard<std::mutex> lock(mtx_);
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  root["configured"] = (bool) s_.configured;
  root["relays"] = (int) relays_.size();
  root["board"] = board_name_;
  // addresses for the Shelly webhook URLs shown in the wizard (name and IP)
  root["mdns"] = App.get_name() + ".local";
  for (auto &ip : network::get_ip_addresses()) {
    if (ip.is_ip4() && ip.is_set()) {
      char b[network::IP_ADDRESS_BUFFER_SIZE];
      ip.str_to(b);
      root["ip"] = std::string(b);
      break;
    }
  }
  root["features"]["ble"] =
#ifdef USE_UFH_BLE
      true;
#else
      false;
#endif
  root["features"]["one_wire"] =
#ifdef USE_UFH_ONE_WIRE
      true;
#else
      false;
#endif
  root["features"]["mqtt"] =
#ifdef USE_MQTT
      true;
#else
      false;
#endif
  root["zone_count"] = s_.zone_count;
  root["active_low"] = (bool) s_.relay_active_low;
  JsonArray roles = root["roles"].to<JsonArray>();
  for (uint8_t k = 0; k < relays_.size(); k++)
    roles.add(s_.relay_role[k]);
  root["house"] = s_.house_mode;
  root["pump_mode"] = s_.pump_mode;
  root["pump_start_s"] = s_.pump_start_s;
  root["pump_stop_s"] = s_.pump_stop_s;
  root["stale_min"] = s_.stale_min;
  root["min_cycle_min"] = s_.min_cycle_min;
  root["hyst_on"] = x10(s_.hyst_on_x10);
  root["hyst_off"] = x10(s_.hyst_off_x10);
  root["holiday"] = x10(s_.holiday_x10);
  JsonObject opt = root["opt"].to<JsonObject>();
  opt["pump_exercise"] = (bool) s_.opt_pump_exercise;
  opt["valve_exercise"] = (bool) s_.opt_valve_exercise;
  opt["window"] = (bool) s_.opt_window;
  opt["optimum_start"] = (bool) s_.opt_optimum_start;
  opt["pv"] = (bool) s_.opt_pv;
  opt["tariff"] = (bool) s_.opt_tariff;
  opt["altherma"] = (bool) s_.opt_altherma;
  JsonObject pv = root["pv"].to<JsonObject>();
  pv["delta"] = x10(s_.pv_delta_x10);
  pv["threshold_w"] = s_.pv_threshold_w;
  pv["topic"] = s_.pv_topic;
  pv["key"] = s_.pv_key;
  JsonObject tr = root["tariff"].to<JsonObject>();
  tr["s1"] = s_.tariff_s1;
  tr["e1"] = s_.tariff_e1;
  tr["s2"] = s_.tariff_s2;
  tr["e2"] = s_.tariff_e2;
  tr["boost"] = x10(s_.tariff_boost_x10);
  tr["cut"] = x10(s_.tariff_cut_x10);
  JsonObject mq = root["mqtt"].to<JsonObject>();
  mq["host"] = s_.mqtt_host;
  mq["port"] = s_.mqtt_port;
  mq["user"] = s_.mqtt_user;
  mq["pass"] = s_.mqtt_pass[0] ? "***" : "";
  JsonObject al = root["altherma"].to<JsonObject>();
  al["host"] = s_.altherma_host;
  al["flow"] = s_.altherma_flow;
  al["ret"] = s_.altherma_return;
  al["provider"] = x_.altherma_provider;
  {
    JsonArray seen = al["seen"].to<JsonArray>();
    std::lock_guard<std::mutex> lock(alt_mtx_);
    uint32_t now_ms = millis();
    for (uint8_t i = 0; i < alt_seen_n_; i++) {
      if (alt_seen_[i].name[0] == 0)
        continue;
      if (now_ms - alt_seen_[i].ms > 600000UL)
        continue;
      JsonObject o = seen.add<JsonObject>();
      o["name"] = alt_seen_[i].name;
      o["age"] = (int) ((now_ms - alt_seen_[i].ms) / 1000);
    }
  }
  JsonObject tk = root["tank"].to<JsonObject>();
  tk["enabled"] = (bool) x_.opt_tank;
  tk["provider"] = tank_link_.tank_provider;
  tk["remote"] = true;
  {
    JsonArray seen = tk["seen"].to<JsonArray>();
    std::lock_guard<std::mutex> lock(alt_mtx_);
    uint32_t now_ms2 = millis();
    for (uint8_t i = 0; i < tank_seen_n_; i++) {
      if (tank_seen_[i].name[0] == 0)
        continue;
      if (now_ms2 - tank_seen_[i].ms > 600000UL)
        continue;
      JsonObject o = seen.add<JsonObject>();
      o["name"] = tank_seen_[i].name;
      o["age"] = (int) ((now_ms2 - tank_seen_[i].ms) / 1000);
    }
  }
  JsonObject xo = root["extra"].to<JsonObject>();
  xo["boost"] = x10(x_.boost_x10);
  xo["boost_min"] = x_.boost_min;
  xo["outdoor"] = (bool) x_.opt_outdoor;
  xo["stop"] = (bool) x_.opt_stop;
  xo["alt_outdoor"] = x_.alt_outdoor;
  JsonObject no = root["net"].to<JsonObject>();
  no["count"] = n_.count;
  no["max"] = std::min<int>(MAX_EXT, MAX_ZONES - this->local_zone_count_());
  JsonArray nos = no["out"].to<JsonArray>();
  for (const auto &e : n_.out) {
    JsonObject eo = nos.add<JsonObject>();
    eo["type"] = e.type;
    eo["ch"] = e.channel;
    eo["host"] = e.host;
    eo["on"] = e.url_on;
    eo["off"] = e.url_off;
  }
  JsonObject mo = root["main"].to<JsonObject>();
  mo["mode"] = m_.mode;
  mo["fixed_eco"] = (bool) m_.fixed_eco;
  mo["comfort"] = x10(m_.comfort_x10);
  mo["eco"] = x10(m_.eco_x10);
  JsonArray msc = mo["sched"].to<JsonArray>();
  for (const auto &d : m_.sched) {
    JsonArray a = msc.add<JsonArray>();
    a.add(d.s1);
    a.add(d.e1);
    a.add(d.s2);
    a.add(d.e2);
  }
  JsonArray zs = root["zones"].to<JsonArray>();
  for (uint8_t i = 0; i < MAX_ZONES; i++) {
    const ZoneCfg &z = s_.zones[i];
    JsonObject o = zs.add<JsonObject>();
    o["name"] = z.name;
    o["leader"] = z.leader;
    o["follow"] = (bool) z.follow_main;
    o["mode"] = z.mode;
    o["pv_boost"] = (bool) z.pv_boost;
    o["comfort"] = x10(z.comfort_x10);
    o["eco"] = x10(z.eco_x10);
    o["manual"] = x10(z.manual_x10);
    o["floor_max"] = x10(z.floor_max_x10);
    src_to_json(o["room"].to<JsonObject>(), z.room);
    src_to_json(o["floor"].to<JsonObject>(), z.floor);
    JsonArray sc = o["sched"].to<JsonArray>();
    for (const auto &d : z.sched) {
      JsonArray a = sc.add<JsonArray>();
      a.add(d.s1);
      a.add(d.e1);
      a.add(d.s2);
      a.add(d.e2);
    }
  }
  std::string out;
  serializeJson(doc, out);
  return out;
}

static void src_from_json(SourceCfg &s, JsonVariantConst v, bool &changed) {
  if (!v.is<JsonObjectConst>())
    return;
  SourceCfg old = s;
  if (v["type"].is<int>())
    s.type = (uint8_t) std::min(v["type"].as<int>(), (int) SRC_DS18B20);
  copy_str(s.param, sizeof(s.param), v["param"]);
  copy_str(s.key, sizeof(s.key), v["key"]);
  if (memcmp(&old, &s, sizeof(s)) != 0)
    changed = true;
}

// Called from loop() with mtx_ held.
bool UFHComponent::web_apply_config(const std::string &body, std::string &error) {
  JsonDocument doc;
  if (deserializeJson(doc, body)) {
    error = "invalid JSON";
    return false;
  }
  JsonVariantConst root = doc.as<JsonVariantConst>();
  bool restart = false;
  auto u8 = [&](const char *k, uint8_t &dst, int lo, int hi, bool needs_restart) {
    if (root[k].is<int>() || root[k].is<bool>()) {
      int v = root[k].is<bool>() ? (int) root[k].as<bool>() : root[k].as<int>();
      v = std::max(lo, std::min(hi, v));
      if (dst != v && needs_restart)
        restart = true;
      dst = (uint8_t) v;
    }
  };
  auto u16 = [&](JsonVariantConst v, uint16_t &dst, int lo, int hi) {
    if (v.is<int>())
      dst = (uint16_t) std::max(lo, std::min(hi, v.as<int>()));
  };
  auto t10 = [&](JsonVariantConst v, int16_t &dst, float lo, float hi) {
    if (v.is<float>() || v.is<int>())
      dst = to_x10(std::max(lo, std::min(hi, v.as<float>())));
  };
  u8("zone_count", s_.zone_count, 1, (int) relays_.size(), true);
  u8("active_low", s_.relay_active_low, 0, 1, false);
  u8("house", s_.house_mode, 0, HOUSE_SUMMER, false);
  if (root["heating"].is<bool>())
    this->set_heating(root["heating"].as<bool>());
  u8("pump_mode", s_.pump_mode, 0, PUMP_OFF, false);
  if (root["roles"].is<JsonArrayConst>()) {
    uint8_t k = 0;
    for (JsonVariantConst v : root["roles"].as<JsonArrayConst>()) {
      if (k >= MAX_RELAYS)
        break;
      if (v.is<int>())
        s_.relay_role[k] = (uint8_t) std::max(0, std::min((int) ROLE_MAX, v.as<int>()));
      k++;
    }
  }
  u16(root["pump_start_s"], s_.pump_start_s, 0, 900);
  u16(root["pump_stop_s"], s_.pump_stop_s, 0, 900);
  u16(root["stale_min"], s_.stale_min, 30, 1440);
  u16(root["min_cycle_min"], s_.min_cycle_min, 0, 60);
  t10(root["hyst_on"], s_.hyst_on_x10, 0.0f, 2.0f);
  t10(root["hyst_off"], s_.hyst_off_x10, 0.0f, 2.0f);
  t10(root["holiday"], s_.holiday_x10, 5.0f, 15.0f);
  JsonVariantConst opt = root["opt"];
  if (opt.is<JsonObjectConst>()) {
    auto ob = [&](const char *k, uint8_t &dst, bool needs_restart) {
      if (opt[k].is<bool>()) {
        if (dst != (uint8_t) opt[k].as<bool>() && needs_restart)
          restart = true;
        dst = opt[k].as<bool>();
      }
    };
    ob("pump_exercise", s_.opt_pump_exercise, false);
    ob("valve_exercise", s_.opt_valve_exercise, false);
    ob("window", s_.opt_window, false);
    ob("optimum_start", s_.opt_optimum_start, false);
    ob("pv", s_.opt_pv, true);
    ob("tariff", s_.opt_tariff, false);
    ob("altherma", s_.opt_altherma, true);
  }
  JsonVariantConst pv = root["pv"];
  if (pv.is<JsonObjectConst>()) {
    t10(pv["delta"], s_.pv_delta_x10, 0.0f, 3.0f);
    if (pv["threshold_w"].is<int>())
      s_.pv_threshold_w = (int16_t) std::max(0, std::min(30000, pv["threshold_w"].as<int>()));
    char old[sizeof(s_.pv_topic)];
    memcpy(old, s_.pv_topic, sizeof(old));
    copy_str(s_.pv_topic, sizeof(s_.pv_topic), pv["topic"]);
    copy_str(s_.pv_key, sizeof(s_.pv_key), pv["key"]);
    if (memcmp(old, s_.pv_topic, sizeof(old)) != 0)
      restart = true;
  }
  JsonVariantConst tr = root["tariff"];
  if (tr.is<JsonObjectConst>()) {
    auto slot = [&](const char *k, uint8_t &dst) {
      if (tr[k].is<int>())
        dst = (uint8_t) std::max(0, std::min(144, tr[k].as<int>()));
    };
    slot("s1", s_.tariff_s1);
    slot("e1", s_.tariff_e1);
    slot("s2", s_.tariff_s2);
    slot("e2", s_.tariff_e2);
    t10(tr["boost"], s_.tariff_boost_x10, 0.0f, 3.0f);
    t10(tr["cut"], s_.tariff_cut_x10, 0.0f, 3.0f);
  }
  JsonVariantConst mq = root["mqtt"];
  if (mq.is<JsonObjectConst>()) {
    char before[sizeof(s_.mqtt_host) + sizeof(s_.mqtt_user) + sizeof(s_.mqtt_pass) + 2];
    memcpy(before, s_.mqtt_host, sizeof(s_.mqtt_host));
    copy_str(s_.mqtt_host, sizeof(s_.mqtt_host), mq["host"]);
    copy_str(s_.mqtt_user, sizeof(s_.mqtt_user), mq["user"]);
    if (mq["pass"].is<const char *>() && strcmp(mq["pass"].as<const char *>(), "***") != 0) {
      copy_str(s_.mqtt_pass, sizeof(s_.mqtt_pass), mq["pass"]);
      restart = true;
    }
    if (mq["port"].is<int>())
      s_.mqtt_port = (uint16_t) mq["port"].as<int>();
    if (memcmp(before, s_.mqtt_host, sizeof(s_.mqtt_host)) != 0)
      restart = true;
  }
  JsonVariantConst al = root["altherma"];
  if (al.is<JsonObjectConst>()) {
    char before_h[sizeof(s_.altherma_host)], before_f[sizeof(s_.altherma_flow)], before_r[sizeof(s_.altherma_return)];
    memcpy(before_h, s_.altherma_host, sizeof(before_h));
    memcpy(before_f, s_.altherma_flow, sizeof(before_f));
    memcpy(before_r, s_.altherma_return, sizeof(before_r));
    copy_str(s_.altherma_host, sizeof(s_.altherma_host), al["host"]);
    copy_str(s_.altherma_flow, sizeof(s_.altherma_flow), al["flow"]);
    copy_str(s_.altherma_return, sizeof(s_.altherma_return), al["ret"]);
    normalize_altherma_path(s_.altherma_flow, sizeof(s_.altherma_flow));
    normalize_altherma_path(s_.altherma_return, sizeof(s_.altherma_return));
    if (memcmp(before_h, s_.altherma_host, sizeof(before_h)) != 0 ||
        memcmp(before_f, s_.altherma_flow, sizeof(before_f)) != 0 ||
        memcmp(before_r, s_.altherma_return, sizeof(before_r)) != 0)
      restart = restart || s_.opt_altherma;
    if (al["provider"].is<const char *>())
      this->set_altherma_provider(al["provider"].as<const char *>());
  }
  JsonVariantConst tk = root["tank"];
  if (tk.is<JsonObjectConst>()) {
    if (tk["enabled"].is<bool>())
      this->set_tank_enabled(tk["enabled"].as<bool>());
    if (tk["provider"].is<const char *>())
      this->set_tank_provider(tk["provider"].as<const char *>());
  }
  if (root["zones"].is<JsonArrayConst>()) {
    uint8_t i = 0;
    for (JsonVariantConst zv : root["zones"].as<JsonArrayConst>()) {
      if (i >= MAX_ZONES)
        break;
      ZoneCfg &z = s_.zones[i];
      if (zv["name"].is<const char *>()) {
        char nn[sizeof(z.name)];
        strncpy(nn, zv["name"].as<const char *>(), sizeof(nn) - 1);
        nn[sizeof(nn) - 1] = 0;
        if (nn[0] && strcmp(nn, z.name) != 0) {
          memcpy(z.name, nn, sizeof(z.name));
          restart = true;
        }
      }
      if (zv["leader"].is<int>()) {
        uint8_t L = (uint8_t) std::max(0, std::min((int) MAX_ZONES, zv["leader"].as<int>()));
        if (L == i + 1)
          L = 0;
        if (L != z.leader)
          restart = true;
        z.leader = L;
      }
      if (zv["mode"].is<int>())
        z.mode = (uint8_t) std::max(0, std::min((int) ZONE_HEAT, zv["mode"].as<int>()));
      if (zv["boost"].is<bool>())
        this->boost_zone(i + 1, zv["boost"].as<bool>() ? 1 : 0);
      if (zv["follow"].is<bool>()) {
        z.follow_main = zv["follow"].as<bool>() && z.leader == 0;
        force_publish_ = true;
      }
      if (zv["pv_boost"].is<bool>())
        z.pv_boost = zv["pv_boost"].as<bool>();
      t10(zv["comfort"], z.comfort_x10, 5.0f, 30.0f);
      t10(zv["eco"], z.eco_x10, 5.0f, 30.0f);
      t10(zv["manual"], z.manual_x10, 5.0f, 30.0f);
      if (zv["floor_max"].is<float>() || zv["floor_max"].is<int>()) {
        float f = zv["floor_max"].as<float>();
        z.floor_max_x10 = f <= 0 ? 0 : to_x10(std::max(18.0f, std::min(40.0f, f)));
      }
      src_from_json(z.room, zv["room"], restart);
      src_from_json(z.floor, zv["floor"], restart);
      if (zv["sched"].is<JsonArrayConst>()) {
        uint8_t d = 0;
        for (JsonVariantConst dv : zv["sched"].as<JsonArrayConst>()) {
          if (d >= 7 || !dv.is<JsonArrayConst>())
            break;
          JsonArrayConst a = dv.as<JsonArrayConst>();
          if (a.size() >= 4) {
            auto c = [](JsonVariantConst x) { return (uint8_t) std::max(0, std::min(144, x.as<int>())); };
            z.sched[d] = {c(a[0]), c(a[1]), c(a[2]), c(a[3])};
          }
          d++;
        }
      }
      if (zv["sched"].is<JsonArrayConst>() || zv["mode"].is<int>() || zv["comfort"].is<float>() ||
          zv["comfort"].is<int>() || zv["eco"].is<float>() || zv["eco"].is<int>())
        zr_[i].override_active = false;
      // "target": change the target now (Auto: until the next schedule change)
      if (zv["target"].is<float>() || zv["target"].is<int>())
        this->ha_set_zone_target(i + 1, zv["target"].as<float>());
      i++;
    }
    if (this->fix_leaders_())
      restart = true;
  }
  JsonVariantConst xv = root["extra"];
  if (xv.is<JsonObjectConst>()) {
    t10(xv["boost"], x_.boost_x10, 0.5f, 5.0f);
    if (xv["boost_min"].is<int>())
      x_.boost_min = (uint16_t) std::max(10, std::min(480, xv["boost_min"].as<int>()));
    if (xv["outdoor"].is<bool>())
      x_.opt_outdoor = xv["outdoor"].as<bool>();
    if (xv["stop"].is<bool>())
      x_.opt_stop = xv["stop"].as<bool>();
    if (xv["alt_outdoor"].is<const char *>()) {
      char before[sizeof(x_.alt_outdoor)];
      memcpy(before, x_.alt_outdoor, sizeof(before));
      copy_str(x_.alt_outdoor, sizeof(x_.alt_outdoor), xv["alt_outdoor"]);
      normalize_altherma_path(x_.alt_outdoor, sizeof(x_.alt_outdoor));
      if (memcmp(before, x_.alt_outdoor, sizeof(before)) != 0 && s_.opt_altherma)
        restart = true;
    }
    this->save_extra_();
  }
  JsonVariantConst nv = root["net"];
  if (nv.is<JsonObjectConst>()) {
    NetCfg before = n_;
    if (nv["count"].is<int>())
      n_.count = (uint8_t) std::max(0, std::min((int) MAX_EXT, nv["count"].as<int>()));
    if (nv["out"].is<JsonArrayConst>()) {
      uint8_t e = 0;
      for (JsonVariantConst ev : nv["out"].as<JsonArrayConst>()) {
        if (e >= MAX_EXT)
          break;
        ExtOut &o = n_.out[e];
        if (ev["type"].is<int>())
          o.type = ev["type"].as<int>() == EXT_HTTP ? EXT_HTTP : EXT_SHELLY;
        if (ev["ch"].is<int>())
          o.channel = (uint8_t) std::max(0, std::min(15, ev["ch"].as<int>()));
        copy_str(o.host, sizeof(o.host), ev["host"]);
        copy_str(o.url_on, sizeof(o.url_on), ev["on"]);
        copy_str(o.url_off, sizeof(o.url_off), ev["off"]);
        e++;
      }
    }
    if (memcmp(&before, &n_, sizeof(n_)) != 0) {
      this->save_net_();
      restart = true;
    }
  }
  JsonVariantConst mv = root["main"];
  if (mv.is<JsonObjectConst>()) {
    if (mv["mode"].is<int>())
      m_.mode = (uint8_t) std::max(0, std::min((int) ZONE_HEAT, mv["mode"].as<int>()));
    if (mv["fixed_eco"].is<bool>())
      m_.fixed_eco = mv["fixed_eco"].as<bool>();
    t10(mv["comfort"], m_.comfort_x10, 5.0f, 30.0f);
    t10(mv["eco"], m_.eco_x10, 5.0f, 30.0f);
    if (mv["sched"].is<JsonArrayConst>()) {
      uint8_t d = 0;
      for (JsonVariantConst dv : mv["sched"].as<JsonArrayConst>()) {
        if (d >= 7 || !dv.is<JsonArrayConst>())
          break;
        JsonArrayConst a = dv.as<JsonArrayConst>();
        if (a.size() >= 4) {
          auto c = [](JsonVariantConst x) { return (uint8_t) std::max(0, std::min(144, x.as<int>())); };
          m_.sched[d] = {c(a[0]), c(a[1]), c(a[2]), c(a[3])};
        }
        d++;
      }
    }
    main_override_ = false;
    this->save_main_();
    force_publish_ = true;
  }
  if (root["configured"].is<bool>()) {
    bool c = root["configured"].as<bool>();
    if (c && !s_.configured)
      restart = true;  // first run: names and entities
    s_.configured = c;
  }
  // relay polarity changes apply immediately
  for (uint8_t k = 0; k < relays_.size(); k++)
    this->write_relay_(k, relay_state_[k]);
  this->save_settings_();
  this->log_event_(EV_SETTINGS);
  if (!restart) {
    this->start_sources_();
  }
  ESP_LOGI(TAG, "Settings saved%s", restart ? " (restart needed)" : "");
  if (restart || (root["restart"].is<bool>() && root["restart"].as<bool>()))
    restart_requested_ = true;
  return true;
}

// ---------------------------------------------------------------------------------------------
// Altherma (or any ESPHome device): leaving / return water, every 30 s, in its own task
// ---------------------------------------------------------------------------------------------
// result.err: -1 ok (or a valid NA reading), 0 no response, -2 unreadable body, else HTTP status.
struct AltHttpResult {
  float value;
  int err;
};

static bool alt_is_na_token(const char *s) {
  if (s == nullptr || !s[0])
    return true;
  return strcasecmp(s, "NA") == 0 || strcasecmp(s, "nan") == 0 || strcasecmp(s, "null") == 0 ||
         strcasecmp(s, "unavailable") == 0 || strcasecmp(s, "unknown") == 0;
}

static AltHttpResult http_get_value(const char *url) {
  AltHttpResult r{NAN, 0};
  if (url == nullptr || !url[0])
    return r;
  esp_http_client_config_t cfg = {};
  cfg.url = url;
  cfg.timeout_ms = 4000;
  esp_http_client_handle_t c = esp_http_client_init(&cfg);
  if (c == nullptr)
    return r;
  if (esp_http_client_open(c, 0) == ESP_OK) {
    esp_http_client_fetch_headers(c);
    int st = esp_http_client_get_status_code(c);
    char buf[512];
    int n = esp_http_client_read_response(c, buf, sizeof(buf) - 1);
    if (st < 200 || st >= 300) {
      r.err = st > 0 ? st : 0;
      ESP_LOGW(TAG, "Altherma HTTP %d from %s", r.err, url);
    } else if (n <= 0) {
      r.err = -2;
      ESP_LOGW(TAG, "Altherma empty body from %s", url);
    } else {
      buf[n] = 0;
      JsonDocument doc;
      if (deserializeJson(doc, buf)) {
        r.err = -2;
        ESP_LOGW(TAG, "Altherma bad JSON from %s", url);
      } else if (doc["value"].isNull()) {
        r.err = -1;
        r.value = NAN;
      } else if (doc["value"].is<float>() || doc["value"].is<int>()) {
        r.value = doc["value"].as<float>();
        r.err = -1;
      } else if (doc["value"].is<const char *>()) {
        const char *s = doc["value"].as<const char *>();
        if (alt_is_na_token(s)) {
          r.err = -1;
          r.value = NAN;
        } else {
          char *end = nullptr;
          float v = strtof(s, &end);
          if (end != s && !std::isnan(v)) {
            r.value = v;
            r.err = -1;
          } else {
            r.err = -1;
            r.value = NAN;
            ESP_LOGW(TAG, "Altherma value '%s' is not a number (%s)", s, url);
          }
        }
      } else {
        r.err = -2;
        ESP_LOGW(TAG, "Altherma unexpected 'value' type from %s", url);
      }
    }
  } else {
    r.err = 0;
    ESP_LOGW(TAG, "Altherma no response from %s", url);
  }
  esp_http_client_cleanup(c);
  return r;
}

void UFHComponent::altherma_task_(void *arg) {
  auto *self = static_cast<UFHComponent *>(arg);
  for (;;) {
    // started from setup(): no HTTP before the network (Ethernet) is up, or lwIP asserts
    while (!self->net_up_)
      vTaskDelay(pdMS_TO_TICKS(1000));
    AltHttpResult f = http_get_value(self->alt_url_flow_);
    AltHttpResult ret = http_get_value(self->alt_url_return_);
    AltHttpResult o = self->alt_url_outdoor_[0] ? http_get_value(self->alt_url_outdoor_) : AltHttpResult{NAN, -1};
    int err = -1;
    if (f.err > 0 || ret.err > 0 || o.err > 0)
      err = f.err > 0 ? f.err : (ret.err > 0 ? ret.err : o.err);
    else if (f.err == 0 || ret.err == 0)
      err = 0;
    else if (f.err == -2 || ret.err == -2 || o.err == -2)
      err = -2;
    self->alt_err_status_ = err;
    if (!std::isnan(f.value) || !std::isnan(ret.value)) {
      self->alt_flow_ = f.value;
      self->alt_return_ = ret.value;
      self->alt_outdoor_ = o.value;
      self->alt_ms_ = millis();
    }
    vTaskDelay(pdMS_TO_TICKS(30000));
  }
}

void UFHComponent::altherma_task_start_() {
  if (alt_task_started_)
    return;
  alt_task_started_ = true;
  xTaskCreate(&UFHComponent::altherma_task_, "ufh_alt", 6144, this, 1, nullptr);
  ESP_LOGI(TAG, "Altherma link: %s", alt_url_flow_);
}

// ---------------------------------------------------------------------------------------------
// External zones: the relay is on another device. Sent from its own task, again every 60 s.
// Shelly: "on" goes with toggle_after=300, so the Shelly closes the zone by itself if this
// controller goes away (failsafe).
// ---------------------------------------------------------------------------------------------
static bool http_get_ok(const char *url) {
  esp_http_client_config_t cfg = {};
  cfg.url = url;
  cfg.timeout_ms = 4000;
  esp_http_client_handle_t c = esp_http_client_init(&cfg);
  if (c == nullptr)
    return false;
  bool ok = false;
  if (esp_http_client_open(c, 0) == ESP_OK) {
    esp_http_client_fetch_headers(c);
    int st = esp_http_client_get_status_code(c);
    char buf[128];
    esp_http_client_read_response(c, buf, sizeof(buf));
    ok = st >= 200 && st < 300;
  }
  esp_http_client_cleanup(c);
  return ok;
}

void UFHComponent::ext_task_(void *arg) {
  auto *self = static_cast<UFHComponent *>(arg);
  int8_t sent[MAX_EXT];
  uint32_t last[MAX_EXT]{};
  for (auto &v : sent)
    v = -1;
  char url[200];
  for (;;) {
    while (!self->net_up_)
      vTaskDelay(pdMS_TO_TICKS(1000));
    uint8_t n = std::min<uint8_t>(self->n_.count, MAX_EXT);
    for (uint8_t e = 0; e < n; e++) {
      bool want = self->ext_want_[e];
      uint32_t now = millis();
      bool due = sent[e] != (int8_t) want || now - last[e] >= 60000UL ||
                 (!self->ext_ok_[e] && now - last[e] >= 10000UL);
      if (!due)
        continue;
      const ExtOut &o = self->n_.out[e];
      url[0] = 0;
      if (o.type == EXT_SHELLY) {
        if (o.host[0]) {
          if (want)
            snprintf(url, sizeof(url), "http://%s/rpc/Switch.Set?id=%u&on=true&toggle_after=300", o.host, o.channel);
          else
            snprintf(url, sizeof(url), "http://%s/rpc/Switch.Set?id=%u&on=false", o.host, o.channel);
        }
      } else {
        const char *u = want ? o.url_on : o.url_off;
        if (u[0])
          snprintf(url, sizeof(url), "%s%s", strncmp(u, "http", 4) == 0 ? "" : "http://", u);
      }
      bool ok = url[0] && http_get_ok(url);
      last[e] = millis();
      if (ok) {
        if (sent[e] != (int8_t) want)
          ESP_LOGI(TAG, "External zone %u: %s", e + 1, want ? "on" : "off");
        sent[e] = want;
        self->ext_fail_ms_[e] = 0;
      } else {
        if (self->ext_ok_[e] || self->ext_fail_ms_[e] == 0)
          ESP_LOGW(TAG, "External zone %u: no answer (%s)", e + 1, url[0] ? url : "no address");
        sent[e] = -1;
        if (self->ext_fail_ms_[e] == 0)
          self->ext_fail_ms_[e] = last[e] ? last[e] : 1;
      }
      self->ext_ok_[e] = ok;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void UFHComponent::ext_task_start_() {
  if (ext_task_started_)
    return;
  ext_task_started_ = true;
  xTaskCreate(&UFHComponent::ext_task_, "ufh_ext", 6144, this, 1, nullptr);
  ESP_LOGI(TAG, "External zones: %u", n_.count);
}

}  // namespace ufh
}  // namespace esphome
