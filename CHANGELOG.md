# Changelog

All notable changes to the firmware and installer. Format:
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), versions follow [SemVer](https://semver.org/).

## [Unreleased]

## [0.2.0] - unreleased
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

[Unreleased]: https://github.com/skydoy/ESPHome_UFH/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.1.0
