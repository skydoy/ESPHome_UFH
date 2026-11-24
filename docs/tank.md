# Buffer tank

From 0.6.0 the buffer tank is a **second board**, «UFH tanque», in the same repository and the same installer. Use it when the tank is too far from the zones box to run cables. Both boards always ship with the same version.

## Two boards

| Board | What it does |
| --- | --- |
| **UFH zones** | Zones, circulator, heat-demand contact (relay 12). Sends `ufh_heat_demand` and `ufh_open_zones` over UDP. Shows the tank temperature, pump and heater on its panel once you pick the tank board. |
| **UFH tank** | Next to the tank: pump power relay, emergency heater relay, DS18B20 in the middle of the tank (GPIO11), optional PWM for pump speed. Listens to the UFH and to the Altherma. Sends tank temperature and pump/heater state back. |

In the installer choose **UFH zonas** or **UFH tanque**, then flash. On each board, the configurator lists the peers it hears; you pick them (two lists on the tank board: UFH and Altherma).

All three devices talk unencrypted `packet_transport` on **UDP port 18511**, every 15 s, on the same trusted LAN.

## Wiring (tank board, PoE-P4)

| Pin | Function |
| --- | --- |
| GPIO44 (Hat2-Bus pin 2) | Pump power relay (230 V) |
| GPIO43 (Hat2-Bus pin 4) | Emergency heater relay |
| GPIO10 (SDIO pin 2) | PWM signal to the pump (optional) |
| GPIO11 (SDIO pin 3) | DS18B20, mid-tank, same as the zones 1-Wire bus |

**A mechanical thermostat in series with the heater is required.** There is no mixing valve: water from the tank goes to the screed. Software cutoff at 45 °C is only a second level. Do not raise it.

## Pump (Grundfos UPML 25-95 130)

The pump **power relay** turns on whenever the UFH reports a heat demand, even with a cold tank. It follows the same overrun as the circulator after the last zone closes. It stays off in Summer (except a brief anti-seize sent as demand). **If UFH data is stale for 60 s, the pump stops** (zone actuators on the other board will likely close).

Speed is optional, by **PWM**, from the number of open zones (configurable min/max speed and how many zones map to maximum). With no demand the relay is off, so there is no PWM either.

The official range lists the **UPML 25-95 130 as AUTO** (internal control, no PWM cable). PWM in the UPML family is typically the 25-105, **profile A (heating)**. Check the nameplate field «Control signal». Profile A is inverted: a higher duty cycle means a lower speed. With no signal a profile-A pump **runs at maximum speed** (Grundfos [Medium UPM data booklet](https://api.grundfos.com/literature/Grundfosliterature-5439385.pdf)). If the plate says AUTO, leave the PWM wire disconnected; the relay on/off is enough.

## Emergency heater

The heater switches on **by itself only** when all of these are true:

- the UFH link is fresh (under 60 s) and there is heat demand
- the tank probe has a fresh reading
- tank temperature is at or below **target − 3 °C** (default target 40 °C)
- UDP from the chosen Altherma board is fresh and `altherma_fault` is on

It never auto-starts because of a stale or unknown fault, or a compressor sitting at 0.

There is **no time limit**. Software cutoff is **45 °C** (auto and manual). Minimum on time 2 min, minimum off time 5 min, unless the cutoff trips. If the probe fails, or UFH data is stale, the heater stays off (auto and manual).

### Manual heater

If the compressor stays at 0 while there is heat demand, the tank board warns you after 15 minutes (configurable, 5–120) and you turn the heater on by hand, on the tank web panel or in Home Assistant. The 45 °C cutoff still applies. Manual starts off after a reboot.

## Altherma

The tank board lists Altherma boards heard on the LAN and you pick one, the same way the zones board does. Fault, compressor frequency and outdoor temperature come over UDP only. The zones board can still use the 0.5.3 HTTP fallback for flow, return and outdoor if UDP is quiet; that fallback never starts the heater.

Error bytes are decoded on the zones board (55 → E7, 147 → U3).

See also [Configuration](configuration.md), [Wiring](wiring.md), [Security](security.md) and [Home Assistant](home-assistant.md).
