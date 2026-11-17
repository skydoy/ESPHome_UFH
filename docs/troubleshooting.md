# Troubleshooting

| Problem | Try |
|---|---|
| Not found in Home Assistant | Check the network LEDs on the RJ45 and the board's IP in your router. Add it by hand: ESPHome integration → host = IP. |
| Status LED red | No network link (cable, PoE, switch port). Blue = network but no Home Assistant. Green = all good. |
| A zone switches but the relay does not click | Relay power supply on JD-VCC? Check the MCP23017 wiring (SDA GPIO16, SCL GPIO17) and the `i2c` scan in the logs (should find `0x20`). |
| Relays work the wrong way round (on when off) | Set `relay_inverted: "false"` ([Configuration](configuration.md)). |
| Relays chatter or stay half on | Use VCC = 3.3 V with the JD-VCC jumper removed ([Wiring](wiring.md)). |
| Pump does not start | *Pump Mode* = Auto? It waits *Pump Start Delay* (default 120 s) after the first zone turns on. |
| Red and green colours swapped on the LED | Change `channel_colors` to `RGB` in `core/boards/waveshare-esp32-s3-eth.yaml`. |

Logs: `http://<device-ip>/` or the Device Builder.
