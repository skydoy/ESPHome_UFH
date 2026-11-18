# Getting started

## What you need

| Part | Notes |
|---|---|
| **Waveshare ESP32-S3-ETH** | ESP32-S3, W5500 Ethernet, USB-C. With the **PoE Module (B)** if your switch has PoE. |
| **Relays** | Either your existing **relay board with 3.3 V inputs** (e.g. the Denkovi board of a DAEnetIP4) wired straight to the ESP — up to 13 relays — or an **MCP23017** + any 16-channel relay board — up to 16 relays. |
| **Temperature sensors** (optional) | Shelly H&T (Wi-Fi), Shelly BLU H&T / BTHome (Bluetooth), any MQTT or Home Assistant sensor, or DS18B20 probes. Without sensors a zone just follows its schedule. |
| Home Assistant (optional) | Not needed to heat: thermostats, schedules and the pump run on the device. |

## How it works

- **Zone N = relay N.** Each zone has its own thermostat on the device (Comfort / Eco schedule, or a fixed target).
- The relays after the zones drive the **circulation pump** and/or ask the **heat pump / boiler** for heat. The pump starts after a delay (actuators take 2-4 min to open) and stops after the last zone closes.
- **Rooms with several loops:** a zone can follow another zone (same sensor, same thermostat).
- Everything is set in a **setup wizard** on the device web page. One firmware for every installation.
- Optional extras: open-window detection, optimum start, solar surplus, two-rate tariff, anti-seize for the pump and the actuators, floor temperature limit, link to ESPHome_Altherma.
