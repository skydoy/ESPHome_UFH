// HTTP endpoints of the UFH controller (served by the device web server, port 80):
//   GET  /ufh/temp?zone=N&t=21.5[&h=55][&b=80][&floor=1]  temperature from Shelly H&T actions etc.
//   GET  /ufh/state                                        live state (web page mockup)
//   GET  /ufh/config                                       settings (wizard)
//   POST /ufh/config   (application/json)                  save settings
//   POST /ufh/restart                                      restart the device
//   GET  /ufh/scan                                          search the 1-Wire bus again
// Runs in the HTTP server task: everything shared with loop() is guarded by mtx_.
#include "ufh.h"

#include <cmath>
#include <cstdlib>

#include "esphome/core/application.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/json/json_util.h"

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
  root["house"] = s_.house_mode;
  root["pump_mode"] = s_.pump_mode;
  root["pump"] = pump_out_;
  root["demand"] = demand_out_;
  root["any"] = any_relay_;
  root["pv"] = pv_surplus_;
  root["exercise"] = ex_state_;
  root["warnings"] = warnings_text_;
  root["active_low"] = (bool) s_.relay_active_low;
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
  for (uint8_t i = 0; i < s_.zone_count && i < relays_.size(); i++) {
    const ZoneCfg &z = s_.zones[i];
    const ZoneRt &r = zr_[i];
    JsonObject o = zs.add<JsonObject>();
    o["n"] = z.name;
    o["m"] = z.mode;
    o["lead"] = z.leader;
    o["src"] = z.room.type;
    put_num(o, "t", r.has_temp ? r.temp : NAN);
    put_num(o, "h", r.hum);
    put_num(o, "f", r.floor_temp);
    put_num(o, "tg", r.target);
    o["age"] = r.has_temp ? (int) ((now_ms - r.temp_ms) / 60000) : -1;
    o["b"] = r.batt;
    o["r"] = relay_state_[i];
    o["w"] = r.warn;
    o["early"] = r.early_start;
    o["ovr"] = r.override_active;
    o["today"] = r.today_s / 60;
    uint32_t sum = 0;
    for (auto h : r.hour_s)
      sum += h;
    o["duty"] = (int) (sum / 864);
    o["rate"] = roundf(p_.rate_c_per_h[i] * 100) / 100;
  }
  JsonObject pmp = root["pump_h"].to<JsonObject>();
  pmp["today"] = pump_today_s_ / 60;
  pmp["total"] = p_.pump_total_s / 3600;
  if (s_.opt_altherma) {
    JsonObject a = root["altherma"].to<JsonObject>();
    put_num(a, "flow", alt_flow_);
    put_num(a, "ret", alt_return_);
    a["age"] = alt_ms_ ? (int) ((now_ms - alt_ms_) / 1000) : -1;
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

std::string UFHComponent::web_config_json() {
  std::lock_guard<std::mutex> lock(mtx_);
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  root["configured"] = (bool) s_.configured;
  root["relays"] = (int) relays_.size();
  root["board"] = board_name_;
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
  JsonArray zs = root["zones"].to<JsonArray>();
  for (uint8_t i = 0; i < relays_.size() && i < MAX_ZONES; i++) {
    const ZoneCfg &z = s_.zones[i];
    JsonObject o = zs.add<JsonObject>();
    o["name"] = z.name;
    o["leader"] = z.leader;
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
  u8("pump_mode", s_.pump_mode, 0, PUMP_OFF, false);
  if (root["roles"].is<JsonArrayConst>()) {
    uint8_t k = 0;
    for (JsonVariantConst v : root["roles"].as<JsonArrayConst>()) {
      if (k >= MAX_RELAYS)
        break;
      if (v.is<int>())
        s_.relay_role[k] = (uint8_t) std::max(0, std::min((int) ROLE_PUMP_DEMAND, v.as<int>()));
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
    copy_str(s_.altherma_host, sizeof(s_.altherma_host), al["host"]);
    copy_str(s_.altherma_flow, sizeof(s_.altherma_flow), al["flow"]);
    copy_str(s_.altherma_return, sizeof(s_.altherma_return), al["ret"]);
    restart = restart || s_.opt_altherma;
  }
  if (root["zones"].is<JsonArrayConst>()) {
    uint8_t i = 0;
    for (JsonVariantConst zv : root["zones"].as<JsonArrayConst>()) {
      if (i >= MAX_ZONES || i >= relays_.size())
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
      zr_[i].override_active = false;
      i++;
    }
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
  ESP_LOGI(TAG, "Settings saved%s", restart ? " (restart needed)" : "");
  if (restart || (root["restart"].is<bool>() && root["restart"].as<bool>()))
    restart_requested_ = true;
  return true;
}

// ---------------------------------------------------------------------------------------------
// Altherma (or any ESPHome device): leaving / return water, every 30 s, in its own task
// ---------------------------------------------------------------------------------------------
static float http_get_value(const char *url) {
  esp_http_client_config_t cfg = {};
  cfg.url = url;
  cfg.timeout_ms = 4000;
  esp_http_client_handle_t c = esp_http_client_init(&cfg);
  if (c == nullptr)
    return NAN;
  float v = NAN;
  if (esp_http_client_open(c, 0) == ESP_OK) {
    esp_http_client_fetch_headers(c);
    char buf[512];
    int n = esp_http_client_read_response(c, buf, sizeof(buf) - 1);
    if (n > 0) {
      buf[n] = 0;
      JsonDocument doc;
      if (!deserializeJson(doc, buf) && (doc["value"].is<float>() || doc["value"].is<int>()))
        v = doc["value"].as<float>();
    }
  }
  esp_http_client_cleanup(c);
  return v;
}

void UFHComponent::altherma_task_(void *arg) {
  auto *self = static_cast<UFHComponent *>(arg);
  for (;;) {
    float f = http_get_value(self->alt_url_flow_);
    float r = http_get_value(self->alt_url_return_);
    if (!std::isnan(f) || !std::isnan(r)) {
      self->alt_flow_ = f;
      self->alt_return_ = r;
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

}  // namespace ufh
}  // namespace esphome
