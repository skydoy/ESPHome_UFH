# Wiring

!!! danger "Mains voltage"
    Thermal actuators, pumps and boiler controls usually run on **230 V**. Switch off the breaker and leave the mains side to a qualified electrician.

## Option A — relays on the ESP pins (up to 13)

For relay boards whose inputs work with **3.3 V** logic, such as the Denkovi boards driven by a DAEnetIP4 (its outputs were 0 / 3.3 V). The ESP replaces the DAEnetIP4: the relay board, the 230 V wiring and the numbering stay.

| Relay | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| ESP GPIO | 1 | 2 | 15 | 16 | 17 | 18 | 38 | 39 | 40 | 41 | 42 | 47 | 48 |

Also connect **GND** of the ESP to GND of the relay board. Solder the wires from the top of the board (the PoE module sits on top too). In the wizard choose **relays switch on with HIGH**.
A 1-Wire bus for DS18B20 probes is on **GPIO44** (4.7 kΩ pull-up to 3.3 V).

## Option B — MCP23017 + 16-channel relay board

| ESP32-S3-ETH | MCP23017 |
|---|---|
| 3V3 / GND | VCC / GND |
| GPIO16 / GPIO17 | SDA / SCL |

MCP23017 A0-A2 to GND (address `0x20`). **GPA0…GPB7 = relays 1…16.** On opto-isolated boards remove the **JD-VCC** jumper, feed **VCC** from 3.3 V and **JD-VCC** from the relay supply; choose **LOW** in the wizard. DS18B20 bus: **GPIO15**.

## Relays after the zones

| Function (wizard) | Connect |
|---|---|
| Circulation pump | pump (through a contactor if it draws more than the relay rating) |
| Heat demand | heat pump / boiler **room thermostat input** (dry contact) |
| Pump + heat demand | one relay for both |

Power: the PoE module (~13 W) powers the ESP only. The relay board keeps its own supply.
