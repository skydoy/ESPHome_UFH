# Security

The prebuilt firmware has **no API key, OTA password or web password**, so any Home Assistant can adopt it. After adopting it, add to your own config and install once:

- `api: encryption: key:` (the Device Builder usually generates one)
- `ota: - platform: esphome password:`
- `web_server: auth:` if other people use your network

Updates come from GitHub Pages over HTTPS (certificate verified). Report problems privately: [security advisory](https://github.com/skydoy/ESPHome_UFH/security/advisories/new).

The Altherma UDP link (port 18511) is **unencrypted**. Anyone on the same LAN can read flow, return, outdoor temperature, compressor frequency and fault flags, and could spoof a fault or a clear packet. Only enable it on a trusted network (the same switch as the Altherma board). The HTTP fallback to the Altherma web page is also unauthenticated on the LAN, like the rest of the UFH web server.
