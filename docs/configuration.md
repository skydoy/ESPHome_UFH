# Configuration

Adopt the device in the **ESPHome Device Builder** first (Home Assistant → ESPHome → *Adopt*). Your adopted YAML imports `devices/ufh-s3-eth.yaml@stable`; add the lines below to it and press **Install**.

## Number of zones and relays

```yaml
substitutions:
  zone_count: "8"     # 1-14, zone N = relay N
  pump_relay: "9"     # after the last zone
  heat_relay: "10"
```

## Relay board that switches ON with a HIGH input

```yaml
substitutions:
  relay_inverted: "false"
```

## MCP23017 at another address

```yaml
substitutions:
  mcp23017_address: "0x21"
```

## Fixed IP address

```yaml
ethernet:
  manual_ip:
    static_ip: 192.168.1.50
    gateway: 192.168.1.1
    subnet: 255.255.255.0
    dns1: 192.168.1.1
```

## Thermostats on the device

Optional: one climate entity per zone that switches its relay by itself, so heating keeps working when Home Assistant is down (the temperature still comes from a Home Assistant sensor):

```yaml
packages:
  thermostat1: !include
    file: github://skydoy/ESPHome_UFH/ufh/thermostat-zone.yaml@stable
    vars: { zone: 1, room: "Living Room", temperature_entity: sensor.living_room_temperature }
```

Presets Comfort 21 °C, Eco 18 °C, Away 12 °C; minimum 5 min on / 5 min off per zone. Full example: [`examples/my-ufh-with-thermostats.yaml`](https://github.com/skydoy/ESPHome_UFH/blob/main/examples/my-ufh-with-thermostats.yaml).

Local DS18B20 probes can go on **GPIO15** (1-Wire, reserved in the board package): add a `one_wire` bus and `dallas_temp` sensors and point the thermostat's `sensor:` at them.

## Web page login / no web page

```yaml
web_server:
  auth:
    username: admin
    password: !secret web_password
```

To remove the page: `web_server: !remove`.
