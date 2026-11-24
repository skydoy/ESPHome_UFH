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
  ROLE_TANK_PUMP = 4,    // buffer-tank recirculation pump
  ROLE_TANK_HEATER = 5,  // emergency tank heater
};
static constexpr uint8_t ROLE_MAX = ROLE_TANK_HEATER;

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
  uint8_t follow_main;  // 1 = target comes from the main thermostat ("Geral"); was reserved (0)
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
  // heat pump anti short-cycle (relays with the heat demand function); 0 = off. Was reserved (0).
  uint16_t hp_min_on_s;
  uint16_t hp_min_off_s;
  uint8_t reserved[12];
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

// Main thermostat ("Geral"): Comfort / Eco temperatures and its own weekly schedule. Zones with
// follow_main take their target from it. Stored in its own NVS entry (the Settings blob keeps
// its layout, so existing installations keep their configuration).
static constexpr uint32_t MAIN_MAGIC = 0x55464D31;  // "UFM1"
struct MainCfg {
  uint32_t magic;
  uint8_t mode;       // ZoneMode: Off (following zones off) / Auto (schedule) / Heat (fixed)
  uint8_t fixed_eco;  // Heat mode: 0 = always Comfort, 1 = always Eco
  uint8_t reserved0[2];
  int16_t comfort_x10;
  int16_t eco_x10;
  DaySchedule sched[7];
  uint8_t reserved[8];
};

// Newer options (boost, outdoor compensation). Own NVS entry, with room to grow.
static constexpr uint32_t EXTRA_MAGIC = 0x55465831;  // "UFX1"
struct ExtraCfg {
  uint32_t magic;
  int16_t boost_x10;   // added to the target while a zone boost runs
  uint16_t boost_min;  // boost duration
  uint8_t opt_outdoor;  // optimum start starts earlier when it is cold outside
  uint8_t prev_house;   // house mode to go back to when the "Heating" switch is turned on again
  uint8_t opt_stop;     // optimum stop: end Comfort early using the learned cooling rate (was reserved, 0)
  uint8_t reserved0;
  char alt_outdoor[80];  // ESPHome_Altherma path of the outdoor temperature
  // Was reserved[64]. Layout stays 64 bytes so existing NVS blobs still load.
  char altherma_provider[32];  // chosen packet_transport hostname; empty = use nothing yet
  uint8_t opt_tank;            // optional buffer-tank module
  uint8_t reserved_tank;
  int16_t tank_target_x10;     // default 40.0 °C
  int16_t tank_cut_x10;        // default 45.0 °C (software cutoff)
  int16_t tank_hyst_x10;       // default 3.0 °C
  uint16_t comp_zero_min;      // minutes of demand+compressor 0 before the warning
  char tank_probe[22];         // DS18B20 address (hex, colons optional)
};
static_assert(sizeof(ExtraCfg) == 156, "ExtraCfg size must stay 156 so NVS blobs still load");

// Minutes of heating per zone for the last days (weekly chart). Own NVS entry, saved once a day.
static constexpr uint32_t WEEK_MAGIC = 0x55465731;  // "UFW1"
struct WeekLog {
  uint32_t magic;
  uint16_t min[MAX_ZONES][7];  // ring, idx = slot of "yesterday"
  uint8_t idx;
  uint8_t reserved[3];
};

// Zones whose relay is on another device (Shelly, any HTTP URL). They come after the zones of the
// board: zones zone_count+1 .. zone_count+count. Own NVS entry.
static constexpr uint32_t NET_MAGIC = 0x55464E31;  // "UFN1"
static constexpr uint8_t MAX_EXT = 8;
enum ExtType : uint8_t {
  EXT_SHELLY = 0,  // Shelly Gen2/Gen3/Pro: GET http://host/rpc/Switch.Set?id=ch&on=..&toggle_after=..
  EXT_HTTP = 1,    // any device: one URL to switch on, one to switch off
};
struct ExtOut {
  uint8_t type;
  uint8_t channel;  // Shelly switch id (0 = first relay)
  uint8_t reserved[2];
  char host[48];      // Shelly IP / host name
  char url_on[100];   // generic HTTP
  char url_off[100];
};
struct NetCfg {
  uint32_t magic;
  uint8_t count;  // external zones (0..MAX_EXT)
  uint8_t reserved[3];
  ExtOut out[MAX_EXT];
  uint8_t reserved2[32];
};

// Learned values added after Persist (its layout keeps the runtime counters). Saved with it.
static constexpr uint32_t LEARN_MAGIC = 0x55464C31;  // "UFL1"
struct Learn {
  uint32_t magic;
  float cool_c_per_h[MAX_ZONES];  // learned cooling rate with the zone closed (optimum stop)
  uint8_t reserved[64];
};

void default_settings(Settings &s, uint8_t relay_count, bool active_low);
void default_main(MainCfg &m);
void default_extra(ExtraCfg &x);

}  // namespace ufh
}  // namespace esphome
