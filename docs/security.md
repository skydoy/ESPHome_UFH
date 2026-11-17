# Security

The prebuilt firmware has **no API key, OTA password or web password**, so any Home Assistant can adopt it. After adopting it, add to your own config and install once:

- `api: encryption: key:` (the Device Builder usually generates one)
- `ota: - platform: esphome password:`
- `web_server: auth:` if other people use your network

Updates come from GitHub Pages over HTTPS (certificate verified). Report problems privately: [security advisory](https://github.com/skydoy/ESPHome_UFH/security/advisories/new).
