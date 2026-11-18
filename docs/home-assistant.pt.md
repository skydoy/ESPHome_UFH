# Home Assistant

Opcional: a placa aquece sozinha. O Home Assistant junta controlo remoto, histórico e automações.

## Entidades

| Entidade | |
|---|---|
| **<zona>** (clima) | Desligado / Aquecer (alvo fixo) / Automático (horário). Em Automático, um alvo novo vale até à próxima mudança do horário |
| <zona> Heating | Relé da zona |
| <zona> Temperature | Temperatura da divisão usada pelo termóstato |
| <zona> Runtime Today / Total, Duty 24h | Estatísticas (equilíbrio entre zonas) |
| House Mode | Casa / Ausente / Férias / Verão |
| Pump Mode | Automático / Ligada / Desligada |
| Pump, Heat Demand, Problem, Warnings, Active Zones | Estado |
| Pump Runtime Today / Total | Estatísticas |

## Ações

| Ação | Dados |
|---|---|
| `esphome.<device>_set_zone_temperature` | `zone`, `temperature` (fonte “Sensor do Home Assistant”) |
| `esphome.<device>_set_floor_temperature` | `zone`, `temperature` |
| `esphome.<device>_set_pv_surplus` | `surplus: true/false` (opção “Excedente solar”) |
| `esphome.<device>_set_zone_mode` | `zone`, `mode` (0 desligado, 1 automático, 2 aquecer) |
| `esphome.<device>_set_zone_target` | `zone`, `temperature` |
| `esphome.<device>_all_zones_off` | — |

## Blueprints

- `esphome_ufh_temperature_from_ha.yaml` — envia um sensor de temperatura do Home Assistant para uma zona.
- `esphome_ufh_update_available.yaml` — aviso quando sai firmware novo.
