# Wiring

!!! danger "Mains voltage"
    Thermal actuators, pumps and boiler controls usually run on **230 V**. Switch off the circuit breaker before working on them, and have a qualified electrician do the mains side.

## Logic side (low voltage)

| Waveshare ESP32-S3-ETH | MCP23017 module |
|---|---|
| 3V3 | VCC |
| GND | GND |
| GPIO16 | SDA |
| GPIO17 | SCL |

MCP23017 A0, A1, A2 to GND (address `0x20`), RESET to VCC. Channel **GPA0 = relay 1 … GPB7 = relay 16**.

## MCP23017 → relay board

Most 16-channel relay boards switch a relay **on with a LOW input** (`relay_inverted: "true"`, the default).

1. **Remove the JD-VCC jumper.**
2. **VCC** (input / opto side) → **3.3 V** of the ESP board. This lets the 3.3 V MCP23017 drive the inputs cleanly.
3. **JD-VCC** + GND (coil side) → the **relay power supply** (5 V or 12 V, to match the coils).
4. **IN1 … IN16** → MCP23017 **GPA0 … GPB7**.

Without the jumper the ESP side and the relay coils stay isolated from each other.

## Relays (high voltage)

| Relay | Connect |
|---|---|
| 1 … 11 | Live of each zone's thermal actuator(s) through the relay's **COM / NO** |
| 12 | Circulation pump (through a contactor if the pump draws more than the relay rating) |
| 13 | Heat pump / boiler **room thermostat input** (dry contact, COM / NO) |

Thermal actuators are usually **NC** (normally closed): powered = open = heating that zone.

## Replacing a Denkovi DAEnetIP4

Keep the same order: Denkovi output N → relay N. Zones 1-11 stay on outputs 1-11 and the pump on output 12, so the field wiring does not change. Remove the Home Assistant integration that talked to the Denkovi (SNMP) and use the new ESPHome entities instead.
