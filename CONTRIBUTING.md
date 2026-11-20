# Contributing

Thanks for helping!

- Keep the README short; details go in `docs/`. User docs exist in English and Portuguese:
  update both (`page.md` / `page.pt.md`, `README.md` / `README.pt.md`).
- Board-specific things (pins, ESP chip, Ethernet, I/O expander) live in `core/boards/`; heating
  logic in `ufh/`; generic services in `core/`. `devices/` only composes packages.
- Before a PR: `esphome config devices/ufh-p4-poe.yaml` and `devices/ufh-p4-poe.factory.yaml`
  pass, and `CHANGELOG.md` has a line under **Unreleased**.
- Say what you tested on real hardware (board, relay board, actuators), or "not tested on hardware".

## Repository layout
| Folder | What |
|---|---|
| `devices/` | `ufh-p4-poe.yaml` to adopt, `ufh-p4-poe.factory.yaml` = web-installer firmware |
| `core/boards/` | Pins, ESP chip, Ethernet, MCP23017, status LED |
| `ufh/` | Heating logic: controller (pump, heat demand), zones, optional thermostat |
| `core/` | Network services, diagnostics, web page, updates |
| `examples/` | Example adopted config with on-device thermostats |
| `static/` | Web installer page (published to GitHub Pages) |
| `docs/` | Guides (EN + PT); `docs/img/` logo |
| `tools/` | `make-logo.py` (regenerates the logo) |

## Releasing
1. Move the **Unreleased** notes to `## [x.y.z] - YYYY-MM-DD` and set `project_version: "x.y.z"`
   in `devices/ufh-p4-poe.yaml`.
2. Push to `main`. When `vx.y.z` does not exist yet, CI releases automatically: builds the
   firmware, creates the tag and the GitHub Release, publishes the installer and OTA manifest to
   GitHub Pages and moves the `stable` branch. Pushes that keep the same version only build.
   (Pushing a `vx.y.z` tag by hand still works.)
