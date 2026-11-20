# Configuration (setup wizard)

Open `http://<device-ip>/`. The **Underfloor heating** panel shows the live diagram; **Settings** opens the wizard:

1. **Relays** — number of zones, whether the relays switch on with HIGH or LOW, and what each extra relay does (pump, heat demand, both).
2. **Zones & sensors** — name, *thermostat* (own, or *same room as* another zone), temperature sensor, optional floor limit.
3. **Temperatures & schedules** — mode (Auto / Fixed / Off), Comfort and Eco °C, two Comfort periods per day.
4. **Options** — pump delays, stale-sensor time, holiday frost temperature, and the optional extras.
5. **Save** — the device restarts to apply the names.

## Temperature sources

| Source | How |
|---|---|
| **Shelly H&T (Wi-Fi)** | Shelly app → sensor → *Actions* → new action, event **Temperature changed**, URL `http://<device-ip>/ufh/temp?zone=N&t=${ev.tC}` (the wizard shows it ready to copy). |
| **Home Assistant sensor** | Import the blueprint `blueprints/automation/esphome_ufh_temperature_from_ha.yaml`, one automation per zone. |
| **MQTT** | Topic + JSON field (Shelly: `tC`, Zigbee2MQTT: `temperature`; empty for a plain number). Set the broker in *Options*. |
| **DS18B20** | Probes on the 1-Wire pin (GPIO11, SDIO header pin 3). The wizard lists the probes found. |

Battery Shelly H&T only report when the temperature changes: if a room is stable for hours, raise *Sensor stale after* (default 6 h) or power the sensor by USB (it then reports every 5 min). When a sensor goes stale the zone closes and a warning is raised.

## Options

| Option | What it does |
|---|---|
| Pump anti-seize | Mondays 10:00 with no heating for 24 h: opens a zone and runs the pump 1 min |
| Actuator anti-seize | Same time: opens and closes every zone |
| Open window | A 1 °C drop within 10 min closes the zone for 30 min |
| Optimum start | Learns each zone's heating rate and starts up to 4 h earlier |
| Solar surplus | +°C on the chosen zones while there is surplus (action `set_pv_surplus` or an MQTT topic) |
| Two-rate tariff | +°C in the cheap hours, −°C the rest of the day |
| ESPHome_Altherma link | Reads flow / return water from the other board for the diagram |
| Floor limit | Per zone: closes above X °C measured by a floor probe |

## House modes

**Home** (normal), **Away** (Eco everywhere), **Holiday** (frost protection, default 7 °C), **Summer** (heating and pump off; anti-seize still runs).

## In your own YAML (adopted device)

```yaml
substitutions:
  timezone: Europe/Lisbon          # schedules use SNTP; Home Assistant also sets it
  relay1_pin: GPIO44               # direct board: move a relay to another GPIO
ethernet:
  manual_ip: { static_ip: 192.168.1.50, gateway: 192.168.1.1, subnet: 255.255.255.0 }
web_server:
  auth: { username: admin, password: !secret web_password }
```
