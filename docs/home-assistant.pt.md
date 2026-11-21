# Home Assistant

Opcional: a placa aquece sozinha. O Home Assistant junta controlo remoto, histórico e automações.

## Entidades

| Entidade | |
|---|---|
| **<zona>** (clima) | Desligado / Aquecer (alvo fixo) / Automático (horário). Em Automático, um alvo novo vale até à próxima mudança do horário |
| <zona> Heating | Relé da zona |
| <zona> Temperature | Temperatura da divisão usada pelo termóstato |
| <zona> Runtime Today / Total, Duty 24h | Estatísticas (equilíbrio entre zonas) |
| <zona> Follows Main | Liga a zona ao termóstato geral (a temperatura passa a vir da geral) |
| <zona> Boost | Botão: +X °C durante Y min (carrega outra vez para parar) |
| <zona> Last Reading | Hora da última temperatura recebida (ex.: do Shelly) |
| **Main Thermostat** (clima) | Termóstato geral: Conforto / Eco (predefinições), horário próprio; Conforto/Eco à mão vale até à próxima mudança |
| Boost Temperature / Duration | Quanto e quanto tempo dura o boost |
| Outdoor Compensation, Outdoor Temperature | Arranque antecipado mais cedo com frio lá fora |
| House Mode | Casa / Ausente / Férias / Verão |
| Pump Mode | Automático / Ligada / Desligada |
| Pump, Heat Demand, Problem, Warnings, Active Zones | Estado |
| Pump Runtime Today / Total | Estatísticas |
| Pump Start / Stop Delay | Atraso da bomba depois do pedido dos termóstatos (s) |
| Heat Pump Min On / Off Time | Proteção contra ciclos curtos da bomba de calor (min, 0 = desligada) |
| Flow / Return Temperature, Flow Return Difference | Água da bomba de calor (ligação ao ESPHome_Altherma) |

## Ações

| Ação | Dados |
|---|---|
| `esphome.<device>_set_zone_temperature` | `zone`, `temperature` (fonte “Sensor do Home Assistant”) |
| `esphome.<device>_set_floor_temperature` | `zone`, `temperature` |
| `esphome.<device>_set_pv_surplus` | `surplus: true/false` (opção “Excedente solar”) |
| `esphome.<device>_set_zone_mode` | `zone`, `mode` (0 desligado, 1 automático, 2 aquecer) |
| `esphome.<device>_set_zone_target` | `zone`, `temperature` |
| `esphome.<device>_all_zones_off` | — |
| `esphome.<device>_boost_zone` | `zone`, `start: true/false` |
| `esphome.<device>_set_outdoor_temperature` | `temperature` (se não usares o ESPHome_Altherma) |

## Blueprints

- `esphome_ufh_temperature_from_ha.yaml` — envia um sensor de temperatura do Home Assistant para uma zona.
- `esphome_ufh_update_available.yaml` — aviso quando sai firmware novo.
- `esphome_ufh_warnings.yaml` — aviso no telemóvel quando aparece um problema (sensor sem dados, pilha, zona que não aquece, chão, ida/retorno) e quando fica resolvido.
