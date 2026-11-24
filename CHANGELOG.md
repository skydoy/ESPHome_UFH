# Changelog

All notable changes to the firmware and installer. Format:
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), versions follow [SemVer](https://semver.org/).

## [0.6.0] - Unreleased

### Added
- Second board **UFH tanque** (M5Stack Unit PoE-P4) in the same repo and installer:
  pump relay, emergency heater relay, DS18B20 on GPIO11, optional PWM on GPIO10.
  One installer, board choice «UFH zonas» / «UFH tanque». Both boards share the
  version; 0.6.0 is both.
- Unencrypted UDP `packet_transport` on port 18511 between the three boards. The zones
  board sends `ufh_heat_demand` and `ufh_open_zones`. The tank board sends `tank_temp`,
  `tank_pump` and `tank_heater` back. Each board lists peers and you pick them by name
  (the tank board has two lists: UFH and Altherma). Values older than 60 s are unknown.
- HTTP fallback from 0.5.3 on the zones board: if UDP is quiet for 60 s and an HTTP
  address is set, flow, return and outdoor temperatures come from the Altherma
  web_server as before. The emergency heater never uses HTTP or stale UDP to switch on.
- Heat-pump fault and error codes (E7, U3, …) on the zones board; compressor frequency;
  “Altherma Link” / “Tank Link” text.
- Tank logic: pump starts whenever the UFH reports heat demand, even with a cold tank;
  auto heater only on a fresh `altherma_fault`; target 40 °C, 3 °C hysteresis, software
  cutoff 45 °C, no time limit. Mechanical thermostat in series required (no mixing
  valve). If UFH data is stale for 60 s, pump and heater stop. If the probe fails, the
  heater stays off. Compressor at 0 with demand: warning after 15 min, manual heater
  on the tank board. PWM speed by number of open zones, profile A (inverted) by
  default; only if the pump nameplate says PWM (the UPML 25-95 130 is listed as AUTO).

### Changed
- Firmware version 0.6.0. The unreleased tank-in-UFH relays 13/14 on the zones board
  are removed; old NVS roles are migrated to unused. Existing zone behaviour is
  unchanged until you turn on the tank link and pick a tank board, and until an
  Altherma board is chosen.

## [0.5.3] - 2026-11-24
### Fixed
- Holiday frost protection (default 7 °C) also applies to zones left Off. Summer still has
  no frost protection; pump and actuator anti-seize keep running in every house mode,
  including Summer.
- A silent floor probe no longer clears an over-temperature block. If the zone is heating,
  it stays open until the learned heating rate says the room will reach the target, then
  closes and does not reopen until a fresh floor reading arrives. The *Warnings* and
  *Problem* entities in Home Assistant report this as “floor probe”.
- A zone with no room sensor now stays off in Holiday (it cannot do frost protection);
  only valve anti-seize still runs.
- Compiler warning in `set_heating` (enumerated and non-enumerated type in a conditional).
- Web installer: Portuguese links open the Portuguese docs; PT/EN and theme pills set
  `aria-pressed` in the static HTML.
- ESPHome_Altherma link: default HTTP paths now use the 2.6.3 entity names of an ERLA D
  unit (spaces encoded as `%20`). Saved paths that still had the old snake_case defaults
  are updated on boot. User-entered paths are URL-encoded. The reader accepts a numeric
  or string `value`, treats `NA`/null as no reading, and raises a warning (and a log
  line) when the response is not HTTP 2xx.
- Portuguese docs footer is localised (“Projeto comunitário independente”). The floor
  probe is called “sonda do chão” throughout the Portuguese docs and the web panel
  (Home Assistant entity object_ids are unchanged).
- Docs on a narrow screen no longer truncate the header title; the House modes YAML
  example scrolls horizontally; long HTTP paths in `code` wrap instead of overflowing.
- Installer and docs dark themes use the brand background `#0b1b3f`.

### Changed
- Documentation can be published on its own (`publish-docs.yml`) without rebuilding or
  replacing the live installer, OTA manifests or firmware images.
- `publish.yml` deploys firmware, manifests and checksums only when a `v*` tag is
  pushed. A push to `main` or a manual `workflow_dispatch` still builds the images as
  artifacts, but does not create a release or publish them to GitHub Pages.
- `SECURITY.md` lists the latest `0.5.x` release as supported. `CONTRIBUTING.md` (and
  `CONTRIBUTING.pt.md`) describe the current layout, the required commit author, and
  tag-only releases.

## [0.5.2] - 2026-11-23
### Changed
- The Altherma reader is started from the main loop once the network is up (never from setup).
- The wizard shows the Shelly webhook URL with the device name (.local) and with its IP.
- Web installer: "Erase device" is now a choice (untick it to keep zones and settings).

## [0.5.1] - 2026-11-23
### Fixed
- Boot loop (`assert failed: xQueueSemaphoreTake`) with the Altherma link set: the Altherma reader
  (and the external zones) made HTTP requests before Ethernet was up. They now wait for the network.
- The "Heating" and "Outdoor Compensation" switches no longer switch at boot (Heating could put the
  house in Summer on every restart).

## [0.5.0] - 2026-11-22
### Added
- **External zones**: zones whose relay is on another device over the network — Shelly Gen2/Gen3/Pro
  (with a 5-minute auto-off failsafe) or any HTTP URL (one to switch on, one to switch off). Up to 8,
  after the board zones (16 zones in total). Warning "device offline" after 2 minutes without answer.
- **Learned cooling rate** per zone (°C/h lost with the zone closed) and new option **Optimum stop**:
  closes up to 90 min before Comfort ends when the room stays within 0.5 °C until then.
- Every zone group shows its learned **Heating Rate** and **Cooling Rate** (also in Home Assistant).
- Docs: **Replacing a DAEnetIP4** — photo guide for the Denkovi 12-relay board (DAE-PB/Ro12/Di8/Ai8):
  which sockets to use (JP1, JP2), wiring diagram to the Unit PoE-P4, 12 V supply, multimeter checks.

### Fixed
- Two zones set as "same room as" each other (e.g. Z8 → Z11 and Z11 → Z8) were both left off.
  Loops and chains are now fixed on start and on save (the lower zone gets its own thermostat back),
  and the installer only offers zones that have their own thermostat.

## [0.4.1] - 2026-11-21
### Fixed
- The controller rebooted every 15 minutes when no Home Assistant was connected (ESPHome API
  `reboot_timeout`). It now runs on its own without rebooting.

## [0.4.0] - 2026-11-21
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
- **Heating** switch (and a button on the panel): off = everything stopped for the summer (house
  mode Summer, the weekly pump anti-seize keeps running); on = back to the previous house mode.
- **Event log** on the panel: last 30 events (zone opened / closed with temperature → target,
  pump / heat pump relay, boost, warnings, house mode, main thermostat, settings). RAM only.
- Blueprint `esphome_ufh_presence.yaml`: Away when everyone leaves, Home when someone arrives.

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

[Unreleased]: https://github.com/skydoy/ESPHome_UFH/compare/v0.6.0...HEAD
[0.6.0]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.6.0
[0.5.3]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.5.3
[0.5.2]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.5.2
[0.5.1]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.5.1
[0.5.0]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.5.0
[0.4.1]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.4.1
[0.4.0]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.4.0
[0.3.2]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.3.2
[0.3.1]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.3.1
[0.3.0]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.3.0
[0.2.0]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.2.0
[0.1.0]: https://github.com/skydoy/ESPHome_UFH/releases/tag/v0.1.0
