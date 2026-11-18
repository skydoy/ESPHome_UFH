#pragma once
// Persistent settings of the UFH controller (one fixed-size blob in NVS).
// Everything the installation wizard (device web page) configures lives here, so one generic
// firmware serves every installation. Change SETTINGS_VERSION when the layout changes.
#include <cstdint>
#include <cstring>

namespace esphome {
namespace ufh {

static constexpr uint8_t MAX_ZONES = 16;
static constexpr uint8_t MAX_RELAYS = 16;
static constexpr uint32_t SETTINGS_MAGIC = 0x55464831;  // "UFH1"
static constexpr uint16_t SETTINGS_VERSION = 1;

enum SourceType : uint8_t {
  SRC_NONE = 0,     // no sensor: the zone only follows its mode (Off / Heat = always on)
  SRC_WEBHOOK = 1,  // GET /ufh/temp?zone=N&t=21.5 (Shelly Plus/Gen3 H&T actions, any HTTP client)
  SRC_HA = 2,       // Home Assistant pushes it (action set_zone_temperature / blueprint)
  SRC_MQTT = 3,     // MQTT topic (+ optional JSON path)
  SRC_BTHOME = 4,   // BTHome v2 BLE advertisement (Shelly BLU H&T, pvvx/ATC Xiaomi...)
  SRC_DS18B20 = 5,  // 1-Wire probe on the board
};

enum RelayRole : uint8_t {
  ROLE_NONE = 0,
  ROLE_PUMP = 1,         // circulation pump
  ROLE_DEMAND = 2,       // heat demand contact (heat pump / boiler)
  ROLE_PUMP_DEMAND = 3,  // one relay does both
};

enum HouseMode : uint8_t { HOUSE_HOME = 0, HOUSE_AWAY = 1, HOUSE_HOLIDAY = 2, HOUSE_SUMMER = 3 };
enum ZoneMode : uint8_t { ZONE_OFF = 0, ZONE_AUTO = 1, ZONE_HEAT = 2 };
enum PumpMode : uint8_t { PUMP_AUTO = 0, PUMP_ON = 1, PUMP_OFF = 2 };

struct SourceCfg {
  uint8_t type;
  char param[63];  // MQTT topic / BLE MAC "AA:BB:CC:DD:EE:FF" / DS18B20 address "28ff..."
  char key[24];    // MQTT: JSON path ("tC", "temperature", "AM2301.Temperature"); empty = plain number
};

// Schedule: per day (0 = Monday) two Comfort periods, in 10-minute steps (0..144).
// A period with start == end is unused.
struct DaySchedule {
  uint8_t s1, e1, s2, e2;
};

struct ZoneCfg {
  char name[24];
  uint8_t leader;  // 0 = own thermostat; k = this relay follows zone k (rooms with several loops)
  uint8_t mode;    // ZoneMode
  uint8_t pv_boost;
  uint8_t reserved0;
  int16_t comfort_x10;
  int16_t eco_x10;
  int16_t manual_x10;   // target in Heat mode
  int16_t floor_max_x10;  // 0 = no floor limit
  SourceCfg room;
  SourceCfg floor;
  DaySchedule sched[7];
  uint8_t reserved[4];
};

struct Settings {
  uint32_t magic;
  uint16_t version;
  uint8_t configured;  // wizard finished
  uint8_t zone_count;  // zones 1..zone_count are relays 1..zone_count
  uint8_t relay_active_low;
  uint8_t relay_role[MAX_RELAYS];  // for relays after the zones
  uint8_t house_mode;
  uint8_t pump_mode;
  uint8_t reserved0;
  uint16_t pump_start_s;
  uint16_t pump_stop_s;
  uint16_t stale_min;   // minutes without a reading -> zone closes + warning
  uint16_t min_cycle_min;
  int16_t hyst_on_x10;   // heat when temp <= target - hyst_on
  int16_t hyst_off_x10;  // stop when temp >= target + hyst_off
  int16_t holiday_x10;   // frost protection target
  // options (all off by default)
  uint8_t opt_pump_exercise;
  uint8_t opt_valve_exercise;
  uint8_t opt_window;
  uint8_t opt_optimum_start;
  uint8_t opt_pv;
  uint8_t opt_tariff;
  uint8_t opt_altherma;
  uint8_t opt_warnings;
  // solar surplus
  int16_t pv_delta_x10;
  int16_t pv_threshold_w;  // MQTT numeric payload above this = surplus (W); booleans also accepted
  char pv_topic[63];
  char pv_key[24];
  // two-rate tariff: cheap periods every day (10-minute steps)
  uint8_t tariff_s1, tariff_e1, tariff_s2, tariff_e2;
  int16_t tariff_boost_x10;  // added in cheap hours
  int16_t tariff_cut_x10;    // removed in expensive hours
  // MQTT broker (only used when a source or the solar signal uses MQTT)
  char mqtt_host[48];
  uint16_t mqtt_port;
  char mqtt_user[32];
  char mqtt_pass[48];
  // ESPHome_Altherma (or any ESPHome device): leaving / return water temperature (web_server GET)
  char altherma_host[48];
  char altherma_flow[80];
  char altherma_return[80];
  uint8_t reserved[16];
  ZoneCfg zones[MAX_ZONES];
};

// Counters and learned values, saved every hour (separate from the settings).
struct Persist {
  uint32_t magic;
  uint32_t zone_total_s[MAX_ZONES];
  uint32_t pump_total_s;
  float rate_c_per_h[MAX_ZONES];  // learned heating rate (optimum start)
  uint32_t last_exercise_epoch;
  uint32_t last_demand_epoch;
  float last_temp[MAX_ZONES];        // last room reading, reused after a restart
  uint32_t last_epoch[MAX_ZONES];
};

void default_settings(Settings &s, uint8_t relay_count, bool active_low);

}  // namespace ufh
}  // namespace esphome
