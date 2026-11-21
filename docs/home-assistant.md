# Home Assistant

Optional: the device heats on its own. Home Assistant adds remote control, history and automations.

## Entities

| Entity | |
|---|---|
| **<zone>** (climate) | Off / Heat (fixed target) / Auto (schedule). In Auto a new target holds until the next schedule change |
| <zone> Heating | Relay of the zone |
| <zone> Temperature | Room temperature used by the thermostat |
| <zone> Follows Main | Links the zone to the main thermostat (its target then comes from the main one) |
| <zone> Boost | Button: +X °C for Y min (press again to stop) |
| <zone> Last Reading | Time of the last temperature received (e.g. from the Shelly) |
| **Main Thermostat** (climate) | Comfort / Eco (presets), own schedule; Comfort/Eco by hand holds until the next change |
| Boost Temperature / Duration | How much and how long the boost lasts |
| Outdoor Compensation, Outdoor Temperature | Optimum start begins earlier when it is cold outside |
| <zone> Runtime Today / Total, Duty 24h | Statistics (balance between zones) |
| House Mode | Home / Away / Holiday / Summer |
| Pump Mode | Auto / On / Off |
| Pump, Heat Demand, Problem, Warnings, Active Zones | State |
| Pump Runtime Today / Total | Statistics |
| Pump Start / Stop Delay | Pump delay after the thermostats' demand (s) |
| Heat Pump Min On / Off Time | Heat pump anti short-cycle (min, 0 = off) |
| Flow / Return Temperature, Flow Return Difference | Heat pump water (ESPHome_Altherma link) |

## Actions

| Action | Data |
|---|---|
| `esphome.<device>_set_zone_temperature` | `zone`, `temperature` (source “Home Assistant sensor”) |
| `esphome.<device>_set_floor_temperature` | `zone`, `temperature` |
| `esphome.<device>_set_pv_surplus` | `surplus: true/false` (option “Solar surplus”) |
| `esphome.<device>_set_zone_mode` | `zone`, `mode` (0 off, 1 auto, 2 heat) |
| `esphome.<device>_set_zone_target` | `zone`, `temperature` |
| `esphome.<device>_all_zones_off` | — |
| `esphome.<device>_boost_zone` | `zone`, `on: true/false` |
| `esphome.<device>_set_outdoor_temperature` | `temperature` (if you do not use ESPHome_Altherma) |

## Blueprints

- `esphome_ufh_temperature_from_ha.yaml` — sends a Home Assistant temperature sensor to a zone.
- `esphome_ufh_update_available.yaml` — notification when a new firmware is out.
- `esphome_ufh_warnings.yaml` — phone notification when a problem shows up (sensor silent, battery, zone not heating, floor, flow/return) and when it is fixed.
