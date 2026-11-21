#pragma once
// ESPHome UFH: underfloor heating controller.
//
// One generic firmware for every installation: the number of zones, names, relay functions,
// temperature sources, schedules and options are set at runtime by the wizard on the device
// web page (stored in NVS, see settings.h). The board package only declares the relay outputs.
//
// The device decides (thermostats run here); Home Assistant only changes targets, modes and
// schedules. Without Home Assistant everything keeps working.

#include <mutex>
#include <string>
#include <vector>
#include <deque>

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/core/string_ref.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/web_server_base/web_server_base.h"

#ifdef USE_UFH_BLE
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#endif
#ifdef USE_UFH_ONE_WIRE
#include "esphome/components/one_wire/one_wire_bus.h"
#endif

#include "settings.h"

namespace esphome {
namespace ufh {

class UFHComponent;

// ---------------------------------------------------------------------------------------------
// Per-zone entities. They can be renamed at boot (zone names from the wizard) and hidden when
// the zone does not exist. Renaming changes the entity names in Home Assistant, so name the
// zones in the wizard before adding the device to Home Assistant.
// ---------------------------------------------------------------------------------------------
class UFHZoneClimate : public climate::Climate, public Component {
 public:
  void set_parent(UFHComponent *p) { parent_ = p; }
  void set_zone(uint8_t z) { zone_ = z; }
  uint8_t zone() const { return zone_; }
  void rename(const char *name) { this->name_ = decltype(this->name_)(StringRef(name)); }
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  UFHComponent *parent_{nullptr};
  uint8_t zone_{0};
};

enum ZoneSensorType : uint8_t { ZS_RUNTIME_TODAY = 0, ZS_RUNTIME_TOTAL = 1, ZS_DUTY_24H = 2, ZS_TEMPERATURE = 3 };

class UFHZoneSensor : public sensor::Sensor {
 public:
  void set_zone(uint8_t z) { zone_ = z; }
  void set_kind(uint8_t k) { kind_ = k; }
  uint8_t zone() const { return zone_; }
  uint8_t kind() const { return kind_; }
  void rename(const char *name) { this->name_ = decltype(this->name_)(StringRef(name)); }

 protected:
  uint8_t zone_{0};
  uint8_t kind_{0};
};

class UFHZoneBinarySensor : public binary_sensor::BinarySensor {
 public:
  void set_zone(uint8_t z) { zone_ = z; }
  uint8_t zone() const { return zone_; }
  void rename(const char *name) { this->name_ = decltype(this->name_)(StringRef(name)); }

 protected:
  uint8_t zone_{0};
};

// "Follows Main" switch of a zone: its target then comes from the main thermostat ("Geral").
class UFHZoneSwitch : public switch_::Switch {
 public:
  void set_parent(UFHComponent *p) { parent_ = p; }
  void set_zone(uint8_t z) { zone_ = z; }
  uint8_t zone() const { return zone_; }
  void rename(const char *name) { this->name_ = decltype(this->name_)(StringRef(name)); }

 protected:
  void write_state(bool state) override;
  UFHComponent *parent_{nullptr};
  uint8_t zone_{0};
};

// Live state of one zone (not persisted).
struct ZoneRt {
  float temp{NAN};
  float hum{NAN};
  int8_t batt{-1};
  uint32_t temp_ms{0};
  bool has_temp{false};
  float floor_temp{NAN};
  uint32_t floor_ms{0};
  bool floor_block{false};
  bool demand{false};        // thermostat wants heat
  bool relay{false};         // relay output state
  uint32_t relay_changed_ms{0};
  uint32_t window_until_ms{0};
  float ref_temp{NAN};       // window detection reference
  uint32_t ref_ms{0};
  bool override_active{false};
  int16_t override_x10{0};
  uint16_t override_until_slot{0};  // minute-of-week / 10 when the override ends
  float target{NAN};
  bool comfort_now{false};
  bool early_start{false};
  // learning (optimum start)
  uint32_t heat_start_ms{0};
  float heat_start_temp{NAN};
  // counters
  uint32_t today_s{0};
  uint16_t hour_s[24]{};     // seconds on, per hour of the last 24 h (ring indexed by hour)
  uint32_t warn{0};          // WARN_* bits
  // name storage for renamed entities
  std::string name_climate, name_heating, name_today, name_total, name_duty, name_temp, name_follow;
};

enum WarnBits : uint32_t {
  WARN_STALE = 1 << 0,
  WARN_BATTERY = 1 << 1,
  WARN_NOT_HEATING = 1 << 2,
  WARN_FLOOR = 1 << 3,
  WARN_WINDOW = 1 << 4,
  WARN_NO_SENSOR = 1 << 5,
};

struct BleSeen {
  uint64_t mac;
  float temp;
  float hum;
  int8_t batt;
  int8_t rssi;
  uint32_t ms;
};

struct PendingReading {
  uint8_t zone;      // 1-based
  bool floor;
  float temp;
  float hum;
  int8_t batt;
};

class UFHComponent : public Component
#ifdef USE_UFH_BLE
    ,
                     public esp32_ble_tracker::ESPBTDeviceListener
#endif
{
 public:
  // --- configuration from codegen ---
  void add_relay(output::BinaryOutput *o) { relays_.push_back(o); }
  void set_time(time::RealTimeClock *t) { time_ = t; }
  void set_default_active_low(bool v) { default_active_low_ = v; }
  void set_board_name(const char *n) { board_name_ = n; }
  void set_web_base(web_server_base::WebServerBase *b) { web_base_ = b; }
  void add_climate(UFHZoneClimate *c) { climates_.push_back(c); }
  void add_zone_sensor(UFHZoneSensor *s) { zone_sensors_.push_back(s); }
  void add_zone_binary_sensor(UFHZoneBinarySensor *b) { zone_bins_.push_back(b); }
  void add_zone_switch(UFHZoneSwitch *s) { zone_switches_.push_back(s); }
#ifdef USE_UFH_ONE_WIRE
  void set_one_wire_bus(one_wire::OneWireBus *b) { ow_bus_ = b; }
#endif

  // --- Component ---
  void setup() override;
  void loop() override;
  void dump_config() override;
  // after the relay outputs (and the I/O expander) are set up, before the API lists entities
  float get_setup_priority() const override { return setup_priority::HARDWARE - 1.0f; }
  void on_shutdown() override;

#ifdef USE_UFH_BLE
  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;
#endif

  // --- used by the YAML entities (template lambdas) ---
  bool pump_on() const { return pump_out_; }
  bool heat_demand() const { return any_relay_; }
  bool demand_relay_on() const { return demand_out_; }
  uint8_t house_mode() const { return s_.house_mode; }
  uint8_t pump_mode() const { return s_.pump_mode; }
  uint16_t pump_start_s() const { return s_.pump_start_s; }
  uint16_t pump_stop_s() const { return s_.pump_stop_s; }
  void set_pump_start_s(uint16_t v);
  void set_pump_stop_s(uint16_t v);
  void set_house_mode(uint8_t m);
  void set_pump_mode(uint8_t m);
  bool problem() const { return !warnings_text_.empty(); }
  const std::string &warnings_text() const { return warnings_text_; }
  const std::string &active_zones_text() const { return active_text_; }
  float pump_today_h() const { return pump_today_s_ / 3600.0f; }
  float pump_total_h() const { return p_.pump_total_s / 3600.0f; }
  bool configured() const { return s_.configured; }
  uint8_t zone_count() const { return s_.zone_count; }
  void all_zones_off();
  // Home Assistant actions
  void ha_set_zone_temperature(int zone, float t);
  void ha_set_floor_temperature(int zone, float t);
  void ha_set_pv_surplus(bool on);
  void ha_set_zone_mode(int zone, int mode);
  void ha_set_zone_target(int zone, float t);

  // --- used by the zone entities ---
  void climate_control(uint8_t zone, const climate::ClimateCall &call);  // zone 0 = main thermostat
  void set_follow_main(int zone, bool on);
  uint8_t relay_count() const { return relays_.size(); }

  // --- web (called from the HTTP server task; they lock mtx_) ---
  void web_reading(const PendingReading &r);
  std::string web_state_json();
  std::string web_config_json();
  bool web_apply_config(const std::string &body, std::string &error);
  void web_request_restart() { restart_requested_ = true; }
  void web_ds18b20_scan() { ow_scan_requested_ = true; }

 protected:
  friend class UFHWebHandler;
  void load_settings_();
  void save_settings_();
  void save_persist_();
  void save_main_();
  float main_target_(const ESPTime &now);
  void main_control_(const climate::ClimateCall &call);
  void publish_main_(UFHZoneClimate *c, bool force);
  void apply_entities_();
  void start_sources_();
  void tick_();
  void update_zone_(uint8_t i, uint32_t now_ms, const ESPTime &now);
  float target_for_(uint8_t i, const ESPTime &now);
  bool in_comfort_(const DaySchedule *sched, uint16_t slot_of_week) const;
  int minutes_to_next_comfort_(const DaySchedule *sched, uint16_t slot_of_week) const;
  uint16_t next_switch_slot_(const DaySchedule *sched, uint16_t slot_of_week) const;
  void update_outputs_(uint32_t now_ms, const ESPTime &now);
  void write_relay_(uint8_t idx, bool on);
  void exercise_tick_(uint32_t now_ms, const ESPTime &now);
  void publish_entities_();
  void accept_reading_(const PendingReading &r, uint32_t now_ms);
  void per_second_(uint32_t now_ms, const ESPTime &now);
  float tariff_offset_(const ESPTime &now) const;
  uint8_t effective_zone_count_() const;
#ifdef USE_MQTT
  void mqtt_subscribe_();
  void mqtt_payload_(int zone, bool floor, const std::string &key, const std::string &payload);
#endif
#ifdef USE_UFH_ONE_WIRE
  void ow_tick_(uint32_t now_ms);
#endif
  void altherma_task_start_();
  static void altherma_task_(void *arg);

  std::vector<output::BinaryOutput *> relays_;
  std::vector<UFHZoneClimate *> climates_;
  std::vector<UFHZoneSensor *> zone_sensors_;
  std::vector<UFHZoneBinarySensor *> zone_bins_;
  std::vector<UFHZoneSwitch *> zone_switches_;
  time::RealTimeClock *time_{nullptr};
  web_server_base::WebServerBase *web_base_{nullptr};
  bool default_active_low_{false};
  const char *board_name_{""};

  Settings s_{};
  Persist p_{};
  ESPPreferenceObject pref_settings_;
  ESPPreferenceObject pref_persist_;
  // main thermostat ("Geral")
  MainCfg m_{};
  ESPPreferenceObject pref_main_;
  bool main_override_{false};      // Comfort / Eco chosen by hand, until the next schedule change
  bool main_override_eco_{false};
  uint16_t main_override_until_{0};
  bool main_comfort_now_{true};
  bool force_publish_{false};
  bool sw_published_[MAX_ZONES]{};
  ZoneRt zr_[MAX_ZONES];
  bool relay_state_[MAX_RELAYS]{};

  // outputs / pump
  bool any_relay_{false};
  uint32_t any_since_ms_{0};
  uint32_t none_since_ms_{0};
  bool pump_out_{false};
  bool demand_out_{false};
  uint32_t pump_today_s_{0};
  bool pv_surplus_{false};
  uint32_t pv_ms_{0};

  // exercise state machine
  uint8_t ex_state_{0};
  uint32_t ex_ms_{0};

  // timing
  uint32_t last_tick_ms_{0};
  uint32_t last_sec_ms_{0};
  uint32_t last_persist_ms_{0};
  int last_day_{-1};
  int last_hour_{-1};

  std::string warnings_text_;
  std::string active_text_;
  bool restart_requested_{false};
  bool restored_{false};
  bool mqtt_started_{false};
  bool web_registered_{false};
  bool relays_rewritten_{false};
  uint32_t restart_at_ms_{0};

  // cross-thread inbox (HTTP task -> loop)
  std::mutex mtx_;
  std::deque<PendingReading> inbox_;
  std::string pending_cfg_;
  bool cfg_pending_{false};

  // BLE
  BleSeen ble_seen_[12]{};
  uint64_t ble_mac_[MAX_ZONES][2]{};  // room / floor MAC per zone (0 = none)

#ifdef USE_UFH_ONE_WIRE
  one_wire::OneWireBus *ow_bus_{nullptr};
  uint64_t ow_addr_[MAX_ZONES][2]{};
  std::vector<uint64_t> ow_found_;
  std::vector<float> ow_found_temp_;
  uint32_t ow_last_ms_{0};
  bool ow_converting_{false};
  bool ow_scan_requested_{true};
#else
  bool ow_scan_requested_{false};
#endif

  // Altherma link (background task)
  volatile float alt_flow_{NAN};
  volatile float alt_return_{NAN};
  volatile uint32_t alt_ms_{0};
  bool alt_task_started_{false};
  char alt_url_flow_[160]{};
  char alt_url_return_[160]{};
};

extern UFHComponent *global_ufh;
void register_web_handler(UFHComponent *p, web_server_base::WebServerBase *base);

// helpers shared by the source files
uint64_t parse_mac(const char *s);
uint64_t parse_hex64(const char *s);
std::string mac_to_str(uint64_t mac);
std::string hex64_to_str(uint64_t v);
bool bthome_parse(const uint8_t *data, size_t len, float &temp, float &hum, int8_t &batt);

}  // namespace ufh
}  // namespace esphome
