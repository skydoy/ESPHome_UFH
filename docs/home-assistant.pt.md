# Home Assistant

## Entidades

| Entidade | O que faz |
|---|---|
| **Zona 1 … Zona N** (interruptor) | Abre / fecha essa zona (relé N) |
| **Desligar todas as zonas** (botão) | Fecha todas as zonas |
| **Zonas ativas** | Que zonas estão ligadas, por exemplo `1, 4, 7` |
| **Modo da bomba** (Automático / Ligada / Desligada) | *Automático* segue as zonas; *Ligada* / *Desligada* forçam a bomba |
| **Bomba** | O relé da bomba está ligado |
| **Pedido de calor** | Há pelo menos uma zona ligada |
| Atraso a ligar / desligar a bomba | Segundos (configuração) |
| Controlo da bomba de calor | A placa liga o relé 13 conforme o pedido de calor (configuração, desligado por omissão) |
| Relé da bomba de calor | Estado do relé 13 (diagnóstico) |

No Home Assistant as entidades aparecem com os nomes em inglês (*Zone 1*, *Pump Mode*…). Podes mudar-lhes o nome lá.

## Pedir calor à bomba de calor (a partir do Home Assistant)

Enquanto o *Controlo da bomba de calor* estiver desligado, usa uma automação que siga o **Pedido de calor**:

```yaml
triggers:
  - trigger: state
    entity_id: binary_sensor.esphome_ufh_heat_demand
actions:
  - action: "switch.turn_{{ trigger.to_state.state }}"
    target:
      entity_id: switch.altherma_heating   # o interruptor da tua bomba de calor / caldeira
```

Quando a bomba de calor estiver ligada ao relé 13, liga o **Controlo da bomba de calor** e apaga a automação.

## Ações

| Ação | Dados |
|---|---|
| `esphome.<device>_set_zone` | `zone: 3`, `on: true` |
| `esphome.<device>_all_zones_off` | — |

## Termóstatos

O firmware pré-compilado só liga relés: a lógica dos termóstatos fica no Home Assistant (por exemplo um *Generic thermostat* por zona a usar `switch.<device>_zone_N`). Para correr os termóstatos **na própria placa**, vê [Configuração](configuration.md#thermostats-on-the-device).
