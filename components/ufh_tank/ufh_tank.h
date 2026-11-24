#pragma once
#include <mutex>
#include <string>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/web_server_base/web_server_base.h"
#ifdef USE_UFH_ONE_WIRE
#include "esphome/components/one_wire/one_wire_bus.h"
#endif

#include "settings.h"

namespace esphome {
namespace ufh_tank {

class TankComponent : public Component {
 public:
  void set_pump(output::BinaryOutput *o) { pump_out_ = o; }
  void set_heater(output::BinaryOutput *o) { heater_out_ = o; }
  void set_pwm(output::FloatOutput *o) { pwm_out_ = o; }
  void set_time(time::RealTimeClock *t) { time_ = t; }
  void set_default_active_low(bool v) { default_active_low_ = v; }
  void set_board_name(const char *n) { board_name_ = n ? n : ""; }
  void set_pwm_pin(int p) { pwm_pin_ = p; }
  void set_web_base(web_server_base::WebServerBase *b) { web_base_ = b; }
#ifdef USE_UFH_ONE_WIRE
  void set_one_wire_bus(one_wire::OneWireBus *b) { ow_bus_ = b; }
#endif

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE - 1.0f; }
  void on_shutdown() override;

  bool pump_on() const { return pump_on_; }
  bool heater_on() const { return heater_on_; }
  bool heater_manual() const { return heater_manual_on_; }
  void set_heater_manual(bool on);
  float tank_temp() const;
  float pwm_duty() const { return pwm_duty_; }
  float pwm_speed() const { return pwm_speed_; }
  float target() const { return cfg_.tank_target_x10 / 10.0f; }
  float cut() const { return cfg_.tank_cut_x10 / 10.0f; }
  uint16_t comp_zero_min() const { return cfg_.comp_zero_min; }
  void set_target(float v);
  void set_cut(float v);
  void set_comp_zero_min(uint16_t v);
  void set_pwm_speeds(uint8_t min_s, uint8_t max_s, uint8_t zones_max, bool invert);
  bool compressor_zero_warn() const { return comp_zero_warn_; }
  bool heat_pump_fault() const;
  int heat_pump_fault_state() const;
  float compressor_freq() const;
  float outdoor_temp() const;
  bool ufh_fresh() const;
  bool altherma_fresh() const;
  int ufh_age_s() const;
  int altherma_age_s() const;
  bool ufh_heat_demand() const { return ufh_fresh() && ufh_demand_; }
  int open_zones() const { return ufh_fresh() ? (int) ufh_zones_ : -1; }
  const char *ufh_provider() const { return cfg_.ufh_provider; }
  const char *altherma_provider() const { return cfg_.altherma_provider; }
  void set_ufh_provider(const char *name);
  void set_altherma_provider(const char *name);
  bool problem() const { return !warnings_text_.empty(); }
  const std::string &warnings_text() const { return warnings_text_; }
  bool configured() const { return cfg_.configured != 0; }
  int pwm_pin() const { return pwm_pin_; }

  std::string web_state_json();
  std::string web_config_json();
  bool web_apply_config(const std::string &body, std::string &error);
  void web_request_restart() { restart_requested_ = true; }
  void web_ds18b20_scan() { ow_scan_requested_ = true; }

 protected:
  friend class TankWebHandler;
  void load_settings_();
  void save_settings_();
  void write_pump_(bool on);
  void write_heater_(bool on);
  void write_pwm_(float duty_pct);
  void tick_(uint32_t now_ms);
  void update_outputs_(uint32_t now_ms);
  void udp_task_start_();
  static void udp_task_(void *arg);
  void udp_on_packet_(const uint8_t *data, size_t len);
  void udp_send_(int sock);
#ifdef USE_UFH_ONE_WIRE
  void ow_tick_(uint32_t now_ms);
#endif

  output::BinaryOutput *pump_out_{nullptr};
  output::BinaryOutput *heater_out_{nullptr};
  output::FloatOutput *pwm_out_{nullptr};
  time::RealTimeClock *time_{nullptr};
  web_server_base::WebServerBase *web_base_{nullptr};
  bool default_active_low_{false};
  std::string board_name_;
  int pwm_pin_{10};

  ESPPreferenceObject pref_;
  TankCfg cfg_{};
  std::mutex mtx_;

  bool pump_on_{false};
  bool heater_on_{false};
  bool heater_manual_on_{false};
  bool heater_auto_blocked_{false};
  uint32_t heater_since_ms_{0};
  uint32_t tank_cut_warn_ms_{0};
  uint32_t comp_zero_since_ms_{0};
  bool comp_zero_warn_{false};
  float pwm_duty_{0};
  float pwm_speed_{0};

  volatile float tank_temp_{NAN};
  volatile uint32_t tank_temp_ms_{0};
#ifdef USE_UFH_ONE_WIRE
  one_wire::OneWireBus *ow_bus_{nullptr};
  uint64_t ow_tank_addr_{0};
  std::vector<uint64_t> ow_found_;
  std::vector<float> ow_found_temp_;
  uint32_t ow_last_ms_{0};
  bool ow_converting_{false};
  bool ow_scan_requested_{true};
#else
  bool ow_scan_requested_{false};
#endif

  static constexpr uint8_t MAX_SEEN = 8;
  static constexpr uint32_t UDP_STALE_MS = 60000UL;
  struct Seen {
    char name[32];
    uint32_t ms;
    bool is_ufh;
    bool is_alt;
  };
  Seen seen_[MAX_SEEN]{};
  uint8_t seen_n_{0};
  std::mutex udp_mtx_;

  volatile int8_t ufh_demand_{-1};
  volatile float ufh_zones_{NAN};
  volatile uint32_t ufh_ms_{0};
  volatile float alt_outdoor_{NAN};
  volatile float alt_freq_{NAN};
  volatile int8_t alt_fault_{-1};
  volatile uint32_t alt_ms_{0};

  bool udp_task_started_{false};
  bool net_up_{false};
  bool web_registered_{false};
  bool restart_requested_{false};
  uint32_t restart_at_ms_{0};
  uint32_t last_tick_ms_{0};
  uint32_t last_tx_ms_{0};
  bool tx_dirty_{true};
  std::string warnings_text_;
  std::string pending_cfg_;
  bool cfg_pending_{false};
};

extern TankComponent *global_tank;
void register_web_handler(TankComponent *p, web_server_base::WebServerBase *base);
uint64_t parse_hex64(const char *s);
std::string hex64_to_str(uint64_t v);

}  // namespace ufh_tank
}  // namespace esphome
