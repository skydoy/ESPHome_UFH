// HTTP endpoints of the UFH tank board (same paths as the zones board, role="tank"):
//   GET  /ufh/state, GET /ufh/config, POST /ufh/config, POST /ufh/restart, GET /ufh/scan
#include "ufh_tank.h"

#include <cmath>
#include <cstring>

#include "esphome/core/application.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/json/json_util.h"

namespace esphome {
namespace ufh_tank {

static const char *const TAG = "ufh_tank.web";

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

class TankWebHandler : public AsyncWebHandler {
 public:
  explicit TankWebHandler(TankComponent *p) : p_(p) {}

  bool canHandle(AsyncWebServerRequest *request) const override {
    char buf[AsyncWebServerRequest::URL_BUF_SIZE];
    StringRef url = request->url_to(buf);
    return url.size() >= 5 && strncmp(url.c_str(), "/ufh/", 5) == 0;
  }

  void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) override {
    if (index == 0) {
      body_.clear();
      if (total > 8000)
        return;
      body_.reserve(total);
    }
    if (body_.size() + len <= 8000)
      body_.append((const char *) data, len);
  }

  void handleRequest(AsyncWebServerRequest *request) override {
    char buf[AsyncWebServerRequest::URL_BUF_SIZE];
    StringRef url = request->url_to(buf);
    std::string u(url.c_str(), url.size());
    bool post = request->method() == HTTP_POST;
    if (u == "/ufh/state") {
      request->send(200, "application/json", p_->web_state_json().c_str());
      return;
    }
    if (u == "/ufh/log") {
      request->send(200, "application/json", "[]");
      return;
    }
    if (u == "/ufh/config" && !post) {
      request->send(200, "application/json", p_->web_config_json().c_str());
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
  TankComponent *p_;
  std::string body_;
};

void register_web_handler(TankComponent *p, web_server_base::WebServerBase *base) {
  base->init();
  base->add_handler(new TankWebHandler(p));  // NOLINT(cppcoreguidelines-owning-memory)
}

std::string TankComponent::web_state_json() {
  std::lock_guard<std::mutex> lock(mtx_);
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  uint32_t now_ms = millis();
  root["role"] = "tank";
  root["configured"] = (bool) cfg_.configured;
  root["warnings"] = warnings_text_;
  root["pwm_pin"] = pwm_pin_;
  JsonObject tnk = root["tank"].to<JsonObject>();
  put_num(tnk, "t", this->tank_temp());
  tnk["pump"] = pump_on_;
  tnk["heater"] = heater_on_;
  tnk["manual"] = heater_manual_on_;
  tnk["target"] = this->target();
  tnk["cut"] = this->cut();
  tnk["blocked"] = heater_auto_blocked_;
  put_num(tnk, "pwm", pwm_duty_);
  put_num(tnk, "speed", pwm_speed_);
  JsonObject u = root["ufh"].to<JsonObject>();
  u["provider"] = cfg_.ufh_provider;
  u["fresh"] = this->ufh_fresh();
  u["age"] = this->ufh_age_s();
  u["demand"] = this->ufh_fresh() ? (ufh_demand_ == 1) : false;
  if (this->open_zones() >= 0)
    u["zones"] = this->open_zones();
  else
    u["zones"] = nullptr;
  JsonObject a = root["altherma"].to<JsonObject>();
  a["provider"] = cfg_.altherma_provider;
  a["fresh"] = this->altherma_fresh();
  a["age"] = this->altherma_age_s();
  a["fault"] = this->heat_pump_fault_state();
  put_num(a, "freq", this->compressor_freq());
  put_num(a, "out", this->outdoor_temp());
  a["comp_zero"] = (bool) comp_zero_warn_;
  {
    std::lock_guard<std::mutex> lock(udp_mtx_);
    JsonArray su = u["seen"].to<JsonArray>();
    JsonArray sa = a["seen"].to<JsonArray>();
    for (uint8_t i = 0; i < seen_n_; i++) {
      if (seen_[i].name[0] == 0 || now_ms - seen_[i].ms > 600000UL)
        continue;
      auto add = [&](JsonArray dst) {
        JsonObject o = dst.add<JsonObject>();
        o["name"] = seen_[i].name;
        o["age"] = (int) ((now_ms - seen_[i].ms) / 1000);
      };
      if (seen_[i].is_ufh)
        add(su);
      if (seen_[i].is_alt)
        add(sa);
    }
  }
  std::string out;
  serializeJson(doc, out);
  return out;
}

std::string TankComponent::web_config_json() {
  std::lock_guard<std::mutex> lock(mtx_);
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  uint32_t now_ms = millis();
  root["role"] = "tank";
  root["configured"] = (bool) cfg_.configured;
  root["board"] = board_name_;
  root["active_low"] = (bool) cfg_.relay_active_low;
  root["pwm_pin"] = pwm_pin_;
  JsonObject tk = root["tank"].to<JsonObject>();
  tk["target"] = this->target();
  tk["cut"] = this->cut();
  tk["hyst"] = cfg_.tank_hyst_x10 / 10.0f;
  tk["comp_zero_min"] = cfg_.comp_zero_min;
  tk["probe"] = cfg_.tank_probe;
  tk["manual"] = heater_manual_on_;
  tk["pwm_min"] = cfg_.pwm_min_speed;
  tk["pwm_max"] = cfg_.pwm_max_speed;
  tk["pwm_zones"] = cfg_.pwm_zones_max;
  tk["pwm_invert"] = (bool) cfg_.pwm_invert;
  JsonObject u = root["ufh"].to<JsonObject>();
  u["provider"] = cfg_.ufh_provider;
  JsonObject a = root["altherma"].to<JsonObject>();
  a["provider"] = cfg_.altherma_provider;
  {
    std::lock_guard<std::mutex> ulock(udp_mtx_);
    JsonArray su = u["seen"].to<JsonArray>();
    JsonArray sa = a["seen"].to<JsonArray>();
    for (uint8_t i = 0; i < seen_n_; i++) {
      if (seen_[i].name[0] == 0 || now_ms - seen_[i].ms > 600000UL)
        continue;
      auto add = [&](JsonArray dst) {
        JsonObject o = dst.add<JsonObject>();
        o["name"] = seen_[i].name;
        o["age"] = (int) ((now_ms - seen_[i].ms) / 1000);
      };
      if (seen_[i].is_ufh)
        add(su);
      if (seen_[i].is_alt)
        add(sa);
    }
  }
#ifdef USE_UFH_ONE_WIRE
  JsonArray probes = root["probes"].to<JsonArray>();
  for (size_t i = 0; i < ow_found_.size(); i++) {
    JsonObject o = probes.add<JsonObject>();
    o["addr"] = hex64_to_str(ow_found_[i]);
    put_num(o, "t", i < ow_found_temp_.size() ? ow_found_temp_[i] : NAN);
  }
#endif
  std::string out;
  serializeJson(doc, out);
  return out;
}

bool TankComponent::web_apply_config(const std::string &body, std::string &error) {
  JsonDocument doc;
  if (deserializeJson(doc, body)) {
    error = "invalid JSON";
    return false;
  }
  JsonObjectConst root = doc.as<JsonObjectConst>();
  if (root["configured"].is<bool>() && root["configured"].as<bool>())
    cfg_.configured = 1;
  if (root["active_low"].is<bool>()) {
    cfg_.relay_active_low = root["active_low"].as<bool>() ? 1 : 0;
    default_active_low_ = cfg_.relay_active_low;
  }
  JsonVariantConst tk = root["tank"];
  if (tk.is<JsonObjectConst>()) {
    if (tk["target"].is<float>() || tk["target"].is<int>())
      this->set_target(tk["target"].as<float>());
    if (tk["cut"].is<float>() || tk["cut"].is<int>())
      this->set_cut(tk["cut"].as<float>());
    if (tk["comp_zero_min"].is<int>())
      this->set_comp_zero_min((uint16_t) tk["comp_zero_min"].as<int>());
    if (tk["manual"].is<bool>())
      this->set_heater_manual(tk["manual"].as<bool>());
    uint8_t mn = cfg_.pwm_min_speed, mx = cfg_.pwm_max_speed, zm = cfg_.pwm_zones_max;
    bool inv = cfg_.pwm_invert != 0;
    if (tk["pwm_min"].is<int>())
      mn = (uint8_t) tk["pwm_min"].as<int>();
    if (tk["pwm_max"].is<int>())
      mx = (uint8_t) tk["pwm_max"].as<int>();
    if (tk["pwm_zones"].is<int>())
      zm = (uint8_t) tk["pwm_zones"].as<int>();
    if (tk["pwm_invert"].is<bool>())
      inv = tk["pwm_invert"].as<bool>();
    this->set_pwm_speeds(mn, mx, zm, inv);
    if (tk["probe"].is<const char *>()) {
      copy_str(cfg_.tank_probe, sizeof(cfg_.tank_probe), tk["probe"]);
#ifdef USE_UFH_ONE_WIRE
      ow_tank_addr_ = cfg_.tank_probe[0] ? parse_hex64(cfg_.tank_probe) : 0;
#endif
    }
  }
  if (root["ufh"].is<JsonObjectConst>() && root["ufh"]["provider"].is<const char *>())
    this->set_ufh_provider(root["ufh"]["provider"].as<const char *>());
  if (root["altherma"].is<JsonObjectConst>() && root["altherma"]["provider"].is<const char *>())
    this->set_altherma_provider(root["altherma"]["provider"].as<const char *>());
  if (root["configured"].is<bool>() || root["tank"].is<JsonObjectConst>() || root["ufh"].is<JsonObjectConst>() ||
      root["altherma"].is<JsonObjectConst>()) {
    cfg_.configured = 1;
    this->save_settings_();
  }
  (void) TAG;
  error.clear();
  return true;
}

}  // namespace ufh_tank
}  // namespace esphome
