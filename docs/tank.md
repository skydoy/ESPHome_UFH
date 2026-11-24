# Buffer tank

Optional module in the UFH firmware (0.6.0). Off by default: existing pump, heat-demand and zone behaviour does not change until you turn it on in the configurator.

## What it adds

A new recirculation pump and an emergency electric heater on a buffer tank, plus a DS18B20 in the middle of the tank (same 1-Wire bus as the floor probes, **GPIO11**).

Recommended roles on the MCP23017 board (16 relays):

| Relay | Role |
| --- | --- |
| 1–11 | Zones |
| 12 | Heat demand (unchanged) |
| 13 | Tank pump |
| 14 | Emergency heater |

The official board with 13 direct GPIO relays cannot keep 11 zones, demand on 12, the tank pump on 13 **and** a heater on 14. Drop a zone, or combine demand with the existing circulator, before you assign the heater.

## Pump

The tank pump starts **as soon as any zone calls for heat**, even if the tank is still cold. It uses the same overrun as the circulation pump (`pump_stop_s`) after the last zone closes. It also runs during the weekly pump anti-seize. It stays off in Summer (except anti-seize). The existing circulation-pump role does not change.

## Emergency heater

The heater switches on **by itself only** when all of these are true:

- the tank module is on
- a zone is calling for heat and the house is not in Summer
- the tank probe has a fresh reading
- tank temperature is at or below **target − 3 °C** (default target 40 °C)
- UDP from the chosen Altherma board is fresh (under 60 s) and `altherma_fault` is on

It never auto-starts because of the HTTP fallback, a stale or unknown fault, or a compressor sitting at 0.

There is **no time limit**. Software cutoff is **45 °C** (auto and manual). Minimum on time 2 min, minimum off time 5 min, unless the cutoff trips.

**A mechanical thermostat in series with the heater is required.** The software cutoff is only a second level. There is no mixing valve between the tank and the manifold, so water from the tank goes to the screed. Do not raise the cutoff above 45 °C.

### Manual heater

If the compressor stays at 0 while there is heat demand, the UFH warns you after 15 minutes (configurable, 5–120) and you turn the heater on by hand: **Manual Heater** on the web panel or in Home Assistant. The 45 °C cutoff still applies. If you turn it off during a fault, auto will not turn it back on until the fault clears or you turn manual on again. Manual starts off after a reboot.

## Probe

Pick the tank DS18B20 in the configurator (the wizard lists probes on GPIO11). Without a valid, fresh reading the heater stays off and the panel shows “tank probe”.

## Altherma over UDP

The UFH listens on **UDP 18511** for unencrypted `packet_transport` packets from ESPHome_Altherma. It lists boards heard on the LAN; you pick one. Until you pick, temperatures stay unknown and the heater never auto-starts.

If UDP is quiet for 60 s and you have set the Altherma HTTP address, **flow, return and outdoor** fall back to the 0.5.3 HTTP link. Fault, error codes and compressor frequency are UDP-only.

On the Altherma board, add the UDP package in the Device Builder (no encryption) and install from there. Both boards must be on the same trusted LAN: the values travel in the clear.

Error bytes are decoded to service codes (55 → E7, 147 → U3). `altherma_fault` is on when either indoor or outdoor error is not 0.

See also [Configuration](configuration.md), [Wiring](wiring.md), [Security](security.md) and [Home Assistant](home-assistant.md).
