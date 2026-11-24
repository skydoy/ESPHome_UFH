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
   in `devices/ufh-p4-poe.yaml` and `devices/ufh-p4-poe-direct.yaml`.
2. Merge into `main`. A push to `main` only validates and builds artifacts; it does not
   create a tag, a GitHub Release, or publish firmware to GitHub Pages.
3. When you are ready, create and push the tag `vx.y.z` on that commit (it must match
   `project_version`). CI then builds the firmware, creates the GitHub Release, publishes
   the installer and OTA manifests to GitHub Pages, and moves the `stable` branch.
   Docs-only updates use `publish-docs.yml` and leave the live firmware as it is.

Portuguese version: [CONTRIBUTING.pt.md](CONTRIBUTING.pt.md).
