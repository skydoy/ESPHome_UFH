# Changelog

All notable changes to the firmware and installer. Format:
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), versions follow [SemVer](https://semver.org/).

## [Unreleased]

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
