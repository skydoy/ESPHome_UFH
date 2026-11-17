<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/img/logo-dark.png">
    <source media="(prefers-color-scheme: light)" srcset="docs/img/logo-light.png">
    <img src="docs/img/logo-light.png" alt="ESPHome UFH" width="256">
  </picture>
</p>

<p align="center"><b>Like it? Help me build something better.</b><br>
  <a href="https://ko-fi.com/skydoy"><img src="https://img.shields.io/badge/Ko--fi-Support%20me-FF5E5B?logo=ko-fi&logoColor=white" alt="Support me on Ko-fi"></a>
  <a href="https://buymeacoffee.com/skydoy"><img src="https://img.shields.io/badge/Buy%20Me%20a%20Coffee-Support%20me-FFDD00?logo=buymeacoffee&logoColor=black" alt="Buy Me a Coffee"></a>
</p>

<p align="center">
  <a href="https://github.com/skydoy/ESPHome_UFH/actions/workflows/publish.yml"><img src="https://github.com/skydoy/ESPHome_UFH/actions/workflows/publish.yml/badge.svg" alt="Build"></a>
  <a href="https://github.com/skydoy/ESPHome_UFH/actions/workflows/validate.yml"><img src="https://github.com/skydoy/ESPHome_UFH/actions/workflows/validate.yml/badge.svg" alt="Validate"></a>
  <a href="https://github.com/skydoy/ESPHome_UFH/releases/latest"><img src="https://img.shields.io/github/v/release/skydoy/ESPHome_UFH?sort=semver" alt="Release"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-PolyForm%20Noncommercial%201.0.0-blue" alt="License: PolyForm Noncommercial"></a>
</p>

<p align="center"><b>Your underfloor heating in Home Assistant. Zones, pump and heat demand, over a network cable. No YAML, no cloud.</b><br>
🇵🇹 <a href="README.pt.md">Ler em português</a></p>

<p align="center">
  <a href="https://skydoy.github.io/ESPHome_UFH/"><img src="https://img.shields.io/badge/Open%20the%20web%20installer-1784F7?style=for-the-badge&logo=esphome&logoColor=white" alt="Open the web installer" height="40"></a>
</p>

## Compatibility

**Waveshare ESP32-S3-ETH** (W5500 Ethernet, optional PoE) + **MCP23017** + **16-channel relay board**: up to 14 zones, the circulation pump and a heat demand contact. Replaces 16-output network controllers such as the Denkovi DAEnetIP4.  
Details: [what you need](docs/getting-started.md) · [wiring](docs/wiring.md)

## Install in 3 steps

1. **Flash** — Open the **[web installer](https://skydoy.github.io/ESPHome_UFH/)**, plug the board in by USB-C → **Install**.
2. **Connect** — network cable (PoE, or 5 V on USB-C), MCP23017 and relay board ([wiring](docs/wiring.md)).
3. **Add** — Home Assistant → **Configure**. There is no Wi-Fi to set up.

## What you get

- One switch per zone (zone N = relay N); the state survives restarts and power cuts
- **Pump** in Auto / On / Off, with delays for the actuators to open and close
- **Heat demand** for the heat pump or boiler (through Home Assistant, or relay 13)
- Optional on-device thermostats
- One-click firmware updates (or automatic at night)
- Device web page on your LAN (`http://<device-ip>/`)

## Docs & more

[Documentation site](https://skydoy.github.io/ESPHome_UFH/docs/) ·
[Configuration](docs/configuration.md) ·
[Home Assistant](docs/home-assistant.md) ·
[Troubleshooting](docs/troubleshooting.md) ·
[Security](docs/security.md) ·
[Changelog](CHANGELOG.md) ·
[Report a problem](https://github.com/skydoy/ESPHome_UFH/issues)

> ⚠️ Actuators, pumps and boilers run on 230 V: leave the mains side to a qualified electrician.

[PolyForm Noncommercial 1.0.0](LICENSE) — free for personal and non-commercial use; commercial use not permitted.  
[Security policy](SECURITY.md) · [Third-party notices](THIRD_PARTY_NOTICES.md)
