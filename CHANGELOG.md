# Changelog

All notable changes to the firmware and installer. Format:
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), versions follow [SemVer](https://semver.org/).

## [Unreleased]
### Added
- **Main thermostat ("Geral")**: Comfort / Eco temperatures and its own weekly schedule. Each zone
  has a **Follows Main** switch (in its group on the web page and in Home Assistant); zones that
  follow take their target from the main thermostat and cannot be changed one by one. Choosing
  Comfort or Eco by hand (climate presets) holds until the next schedule change. House modes,
  open window and floor limit still apply. Existing settings are kept (stored separately).
- **Pump group** on the web page (opens / closes like the zones): pump, heat demand, pump mode,
  runtime, and the relay delays after the thermostats' demand as two settings you can change
  directly (**Pump Start Delay** / **Pump Stop Delay**, 0–900 s, also in Home Assistant).
- **Heat pump anti short-cycle**: minimum on / off time (0–60 min, 0 = off) of the relay with the
  heat demand function, in the pump group and in Home Assistant.
- **Last reading** per zone: header shows "x min ago"; new Home Assistant sensor "Zone N Last Reading"
  (timestamp, about 2 min resolution).
- Main thermostat group opens / closes like the zones, with a summary (Comfort/Eco, target,
  zones following). Zones that follow the main thermostat show "follows main" instead of a slider.
- **Boost** per zone: button (web page, Home Assistant, action `boost_zone`) adds Boost Temperature
  (default +1 °C) for Boost Duration (default 60 min), then goes back by itself.
- **Heat pump water** in the pump group: flow, return and their difference (ESPHome_Altherma link);
  warning when the difference stays below 1 °C or above 10 °C for 20 min with the pump running.
- **Outdoor compensation** (optional): optimum start begins earlier when it is cold outside (up to
  twice as early at -10 °C). Outdoor temperature from ESPHome_Altherma or the action
  `set_outdoor_temperature`.
- **Weekly chart** in the Balance window: hours heating per zone for the last 7 days.
- Blueprint `esphome_ufh_warnings.yaml`: phone notification for warnings (and when they clear).

## [0.3.2] - 2026-11-21
### Changed
- Web page: each zone has its own group, in number order (Z1, Z2 … Z11), closed by default.
  The header shows the zone name, temperature → target, heating and warnings; click to open.
  Inside, rows are short (Thermostat, Heating, Temperature, Runtime). Open zones are remembered.

## [0.3.1] - 2026-11-21
### Fixed
- Build error with ESPHome 2026.11 (`no match for operator=` on `ProgmemStringRef` in `ufh.h`):
  renaming the zone entities now works on ESPHome 2026.10 and 2026.11. Firmware built with 2026.11.0.

## [0.3.0] - 2026-11-20
### Changed
- **New board: M5Stack Unit PoE-P4** (ESP32-P4, IP101 Ethernet, PoE, in its own case) replaces
  the Waveshare ESP32-S3-ETH. Installer firmwares are now `p4-poe-direct` (13 relays on the
  Hat2-Bus / SDIO headers) and `p4-poe` (MCP23017 on the Grove port, 16 relays).
- Relay pins: Hat2-Bus G44 G43 G21 G42 G20 G41 G19 G40 G39 G23 G22, SDIO G13 G12;
  1-Wire on G11; MCP23017 on G53 (SDA) / G54 (SCL).
### Removed
- Bluetooth BTHome sensors (the ESP32-P4 has no radio). The source is hidden in the wizard on
  boards without Bluetooth; the code stays for future boards.
### Added
- Wiring diagrams (images) in the docs: relays on the ESP pins, MCP23017, mains side.
### Fixed
- Dark theme: installer step buttons (Back / Start over) and the web panel labels were hard to read.

## [0.2.0] - 2026-11-18
### Added
- **Thermostats on the device** (works without Home Assistant): per-zone Comfort / Eco schedule
  (two periods per day), fixed target or off; Home Assistant climate entities per zone.
- **Setup wizard** on the device web page: number of zones, relay polarity, function of the
  extra relays (pump, heat demand, both), zone names, rooms with several loops, temperature
  sources, schedules and options. One firmware for every installation.
- **Temperature sources** per zone: Shelly H&T actions (`/ufh/temp` webhook), Home Assistant
  (action + blueprint), MQTT topic, Bluetooth BTHome v2, DS18B20 probes.
- **Live diagram** in the web page (manifold, zones, pump, heat pump), zone view, schedule editor,
  zone balance report.
- New board **direct GPIO**: up to 13 relays straight on the ESP pins (e.g. Denkovi relay boards).
- House modes (Home / Away / Holiday / Summer), runtime and duty counters, warnings.
- Optional: open-window detection, optimum start, solar surplus, two-rate tariff, pump and
  actuator anti-seize, floor temperature limit, ESPHome_Altherma flow / return.
### Changed
- The zone switches of 0.1.0 are replaced by climate entities; the pump and heat demand logic
  moved into the `ufh` component (`components/ufh`).

## [0.1.0] - 2026-11-17
### Added
- First release for the **Waveshare ESP32-S3-ETH** (W5500 Ethernet, optional PoE) with an
  MCP23017 and a 16-channel relay board (`devices/ufh-s3-eth.yaml`, factory build
  `devices/ufh-s3-eth.factory.yaml`).
- 1-14 zones (`zone_count`, zone N = relay N) with state restored after restarts and power cuts.
- Circulation pump (`pump_relay`, default 12): *Pump Mode* Auto / On / Off, *Pump Start Delay*
  (default 120 s) and *Pump Stop Delay* (default 60 s).
- *Heat Demand* sensor; optional heat demand relay (`heat_relay`, default 13) behind
  *Heat Pump Control* (off by default).
- *All Zones Off*, *Active Zones*, actions `set_zone` and `all_zones_off`.
- Optional on-device thermostat package `ufh/thermostat-zone.yaml` and example config.
- Status LED (green / blue / red), diagnostics, report-only update check (adopted builds),
  OTA from GitHub Pages with nightly Auto Update (factory builds).
- Web installer, device web page with PT/EN toggle, MkDocs documentation (PT/EN), CI.

[Unreleased]: https://github.com/skydoy/ESPHome_UFH/compare/v0.3.2...HEAD
[0.3.2]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.3.2
[0.3.1]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.3.1
[0.3.0]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.3.0
[0.2.0]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.2.0
[0.1.0]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.1.0
