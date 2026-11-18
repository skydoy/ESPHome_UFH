# Home Assistant

Optional: the device heats on its own. Home Assistant adds remote control, history and automations.

## Entities

| Entity | |
|---|---|
| **<zone>** (climate) | Off / Heat (fixed target) / Auto (schedule). In Auto a new target holds until the next schedule change |
| <zone> Heating | Relay of the zone |
| <zone> Temperature | Room temperature used by the thermostat |
| <zone> Runtime Today / Total, Duty 24h | Statistics (balance between zones) |
| House Mode | Home / Away / Holiday / Summer |
| Pump Mode | Auto / On / Off |
| Pump, Heat Demand, Problem, Warnings, Active Zones | State |
| Pump Runtime Today / Total | Statistics |

## Actions

| Action | Data |
|---|---|
| `esphome.<device>_set_zone_temperature` | `zone`, `temperature` (source “Home Assistant sensor”) |
| `esphome.<device>_set_floor_temperature` | `zone`, `temperature` |
| `esphome.<device>_set_pv_surplus` | `surplus: true/false` (option “Solar surplus”) |
| `esphome.<device>_set_zone_mode` | `zone`, `mode` (0 off, 1 auto, 2 heat) |
| `esphome.<device>_set_zone_target` | `zone`, `temperature` |
| `esphome.<device>_all_zones_off` | — |

## Blueprints

- `esphome_ufh_temperature_from_ha.yaml` — sends a Home Assistant temperature sensor to a zone.
- `esphome_ufh_update_available.yaml` — notification when a new firmware is out.
