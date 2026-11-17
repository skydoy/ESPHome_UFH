# Home Assistant

## Entities

| Entity | What it does |
|---|---|
| **Zone 1 … Zone N** (switch) | Opens / closes that zone (relay N) |
| **All Zones Off** (button) | Closes every zone |
| **Active Zones** | Which zones are on, e.g. `1, 4, 7` |
| **Pump Mode** (Auto / On / Off) | *Auto* follows the zones; *On* / *Off* force the pump |
| **Pump** | The pump relay is on |
| **Heat Demand** | At least one zone is on |
| Pump Start Delay / Pump Stop Delay | Seconds (config) |
| Heat Pump Control | The device drives relay 13 from Heat Demand (config, off by default) |
| Heat Pump Relay | State of relay 13 (diagnostic) |

## Asking the heat pump for heat (from Home Assistant)

Until *Heat Pump Control* is on, let an automation follow **Heat Demand**:

```yaml
triggers:
  - trigger: state
    entity_id: binary_sensor.esphome_ufh_heat_demand
actions:
  - action: "switch.turn_{{ trigger.to_state.state }}"
    target:
      entity_id: switch.altherma_heating   # your heat pump / boiler switch
```

When the heat pump is wired to relay 13, switch **Heat Pump Control** on and remove the automation.

## Actions

| Action | Data |
|---|---|
| `esphome.<device>_set_zone` | `zone: 3`, `on: true` |
| `esphome.<device>_all_zones_off` | — |

## Thermostats

The prebuilt firmware only switches relays: the thermostat logic stays in Home Assistant (e.g. a *Generic thermostat* per zone using `switch.<device>_zone_N`). To run the thermostats **on the device**, see [Configuration](configuration.md#thermostats-on-the-device).
