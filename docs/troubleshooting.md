# Troubleshooting

| Problem | Try |
|---|---|
| Zones never heat | Finish the setup wizard (zones stay off before). Check the warnings in the panel. |
| “sensor” warning | No reading for longer than *Sensor stale after*. Check the Shelly action URL (zone number) or raise the time. |
| Relays work the wrong way round | Wizard → Relays → switch on with HIGH / LOW. |
| Pump does not start | It waits *Pump start delay* (120 s) after the first zone opens; check the relay function in the wizard. |
| BLE sensor not listed | Press its button; BTHome v2 without encryption only. |
| Status LED red / blue | Red: no network. Blue: network, no Home Assistant (fine: it still heats). |
