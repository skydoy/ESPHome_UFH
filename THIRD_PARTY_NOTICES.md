# Third-party notices

This repository's firmware, configs, installer and docs are under
[PolyForm Noncommercial 1.0.0](LICENSE) (Copyright (c) 2026 Tiago Febra / skydoy).

All firmware configuration in this repository is original work. Nothing from other projects is
copied here.

## ESPHome runtime in the firmware
The firmware `.bin` files are built with [ESPHome](https://github.com/esphome/esphome). ESPHome's
C++ runtime, which is compiled into every image, is licensed under
[GPL-3.0](https://github.com/esphome/esphome/blob/dev/LICENSE) (its Python tooling is MIT). That
part keeps its own licence. The PolyForm Noncommercial 1.0.0 licence of this repository covers
this project's own code, configuration, installer and docs: you may use, change and share them,
but not for commercial purposes. This note is informational, not legal advice.

## Also downloaded at build time or in the browser (not stored here)
- **ESP-IDF** (Espressif and contributors), Apache-2.0 / LGPL-2.1: frameworks
  that ESPHome downloads and links into the firmware.
- **ESP Web Tools** (Open Home Foundation), Apache-2.0: the web installer's Install button, loaded from a CDN.
- **ESPHome web_server v3 front end** (ESPHome), loaded from the ESPHome CDN by the board's web page.

Waveshare, Denkovi and Daikin Altherma are trademarks of their owners. This is an independent
community project, not affiliated with or endorsed by any of them.
