# Troubleshooting

| Problem | Try |
|---|---|
| Zones never heat | Finish the setup wizard (zones stay off before). Check the warnings in the panel. |
| “sensor” warning | No reading for longer than *Sensor stale after*. Check the Shelly action URL (zone number) or raise the time. |
| “floor probe” warning | The floor probe is silent. If the zone was heating, it stays open until the learned heating rate says the room will reach the target, then stays closed until the probe reports again. |
| Relays work the wrong way round | Wizard → Relays → switch on with HIGH / LOW. |
| Pump does not start | It waits *Pump start delay* (120 s) after the first zone opens; check the relay function in the wizard. |
| Status LED red / blue | Red: no network. Blue: network, no Home Assistant (fine: it still heats). |
