# FAQ

**Does it need Home Assistant to keep heating?**
No. Zone states are kept on the device and survive restarts and power cuts; the pump logic runs on the device. Thermostat decisions are in Home Assistant unless you add the [on-device thermostats](configuration.md#thermostats-on-the-device).

**Wi-Fi?**
No, Ethernet only (more reliable in an electrical cabinet). The prebuilt firmware does not enable Wi-Fi.

**Can I use my own temperature probes?**
Yes, DS18B20 on GPIO15, see [Configuration](configuration.md#thermostats-on-the-device).

**How many zones?**
Up to 14 on one MCP23017 / 16-relay board (plus pump and heat demand).

**Is it free?**
For personal and non-commercial use, yes ([licence](licence.md)).
