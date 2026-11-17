# Getting started

## What you need

| Part | Notes |
|---|---|
| **Waveshare ESP32-S3-ETH** | ESP32-S3, W5500 Ethernet, USB-C. Get the version **with the PoE module** if your switch has PoE. |
| **MCP23017 module** (I2C, 16 I/O) | Address `0x20` (A0-A2 to GND). Powered from the board's 3.3 V. |
| **16-channel relay board** | Opto-isolated, **with a JD-VCC jumper** (see [Wiring](wiring.md)). 5 V or 12 V coils. |
| **Power supply for the relays** | 5 V or 12 V DIN-rail supply. PoE (802.3af, ~13 W) is **not** enough for 16 relay coils. |
| Network cable | PoE switch/injector, or a normal port plus 5 V on USB-C. |
| Home Assistant | With the ESPHome integration (added automatically when the device is found). |

## How it works

- **Zone N = relay N.** Turning *Zone N* on powers that zone's thermal actuator(s).
- **Pump** (relay 12 by default): in *Auto* it starts when at least one zone is on, after *Pump Start Delay* (actuators take 2-4 min to open), and stops *Pump Stop Delay* after the last zone closes.
- **Heat Demand** is on while any zone is on. Use it in Home Assistant to ask the heat pump or boiler for heat, or let the device switch relay 13 by itself (*Heat Pump Control*).
- Zone states survive a restart or a power cut, so heating carries on even if Home Assistant is down.

Default layout (change it in [Configuration](configuration.md)):

| Relays | Use |
|---|---|
| 1-11 | Zones 1-11 |
| 12 | Circulation pump |
| 13 | Heat demand (dry contact for the heat pump / boiler) |
| 14-16 | Free |
