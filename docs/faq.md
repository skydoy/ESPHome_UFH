# FAQ

**Does it need Home Assistant?** No. Thermostats, schedules, pump and heat demand run on the device.

**Wi-Fi?** No, Ethernet (Wi-Fi is not enabled).

**My sensors?** Shelly H&T (Wi-Fi actions), MQTT, Home Assistant or DS18B20 — chosen per zone.

**How many zones?** Up to 13 on the ESP pins, 16 with an MCP23017 (minus the relays used for pump / heat demand).

**Is it free?** For personal and non-commercial use ([licence](licence.md)).

## Can I add a buffer tank to this firmware?

Yes, as an optional module from 0.6.0. See [Buffer tank](tank.md). It stays off until you enable it. The existing heat-demand relay does not become the tank pump.
