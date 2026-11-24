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
| Optimum stop | Learns each room's cooling rate (zone closed) and closes up to 90 min before Comfort ends if the room stays within 0.5 °C until then |
| Solar surplus | +°C on the chosen zones while there is surplus (action `set_pv_surplus` or an MQTT topic) |
| Two-rate tariff | +°C in the cheap hours, −°C the rest of the day |
| ESPHome_Altherma link | From 0.6.0 the UFH listens on UDP 18511 and lists Altherma boards on the LAN; you pick one. Values older than 60 s are unknown. If UDP is quiet and you have set an HTTP address, flow / return / outdoor fall back to the other board's web page (defaults remain the ESPHome_Altherma 2.6.3 names on an ERLA D unit: `/sensor/Leaving%20Water%20Temp%20after%20BUH%20(R2T)`, `/sensor/Inlet%20Water%20Temp%20(R4T)` and `/sensor/Outdoor%20Air%20Temp%20(R1T)`). Paths you type are URL-encoded. Fault, error codes and compressor frequency are UDP-only. See [Buffer tank](tank.md). |
| Floor limit | Per zone: closes above X °C measured by a floor probe. If the probe goes silent while the zone is heating, the zone stays open until the learned heating rate says the room will reach the target, then closes and does not reopen until a fresh floor reading arrives. The *Warnings* and *Problem* entities in Home Assistant report this as “floor probe”. |

## External zones (over the network)

Zones whose relay is on another device. Set them in step 1 of the wizard (**External zones**); they come
after the board zones (with 11 zones on the board, the first external zone is zone 12). Up to 8, and up
to 16 zones in total.

| Device | How it is switched |
|---|---|
| Shelly (Plus, Pro, Gen3) | `http://<ip>/rpc/Switch.Set?id=<relay>&on=true&toggle_after=300` / `on=false`. The Shelly must not have a password. |
| HTTP URL (any device) | One URL to switch on, one to switch off. It must answer 200 OK. |

The command is sent on every change and repeated every minute. With a Shelly, "on" is sent with a
5-minute timer, so the Shelly closes the zone by itself if this controller goes away. If a device stops
answering for 2 minutes, the zone shows the warning **device offline**.

Each zone also shows its learned **Heating Rate** and **Cooling Rate** (°C/h) in its group on the device
page and in Home Assistant.

## House modes

**Home** (normal), **Away** (Eco everywhere), **Holiday** (frost protection, default 7 °C, including zones left Off), **Summer** (no frost protection; heating and pump off; actuator and pump anti-seize still run). A zone with no room sensor cannot do frost protection (there is no temperature to follow): in Holiday it stays off, and only valve anti-seize still runs.

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
