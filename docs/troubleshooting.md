# Troubleshooting

| Problem | Try |
|---|---|
| Zones never heat | Finish the setup wizard (zones stay off before). Check the warnings in the panel. |
| “sensor” warning | No reading for longer than *Sensor stale after*. Check the Shelly action URL (zone number) or raise the time. |
| “floor probe” warning | The floor probe is silent. If the zone was heating, it stays open until the learned heating rate says the room will reach the target, then stays closed until the probe reports again. |
| Relays work the wrong way round | Wizard → Relays → switch on with HIGH / LOW. |
| Pump does not start | It waits *Pump start delay* (120 s) after the first zone opens; check the relay function in the wizard. |
| Altherma temperatures show as NA, or warning “Altherma link: HTTP 404” | The other board's web server (ESPHome_Altherma 2.6.3) expects the exact entity name. The defaults are `/sensor/Leaving%20Water%20Temp%20after%20BUH%20(R2T)`, `/sensor/Inlet%20Water%20Temp%20(R4T)` and `/sensor/Outdoor%20Air%20Temp%20(R1T)` (ERLA D). Confirm a path in the browser: `http://<altherma-ip>/sensor/Inlet%20Water%20Temp%20(R4T)` should return a number. |
| Status LED red / blue | Red: no network. Blue: network, no Home Assistant (fine: it still heats). |
