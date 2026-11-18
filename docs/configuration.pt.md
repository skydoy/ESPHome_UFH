# Configuração (assistente)

Abre `http://<ip-da-placa>/`. O painel **Piso radiante** mostra o diagrama em tempo real; **Definições** abre o assistente:

1. **Relés** — número de zonas, se os relés ligam com sinal ALTO ou BAIXO e o que faz cada relé extra (bomba, pedido de calor, as duas).
2. **Zonas e sensores** — nome, *termóstato* (próprio, ou *mesma divisão que* outra zona), sensor de temperatura, limite do pavimento opcional.
3. **Temperaturas e horários** — modo (Automático / Fixo / Desligado), Conforto e Eco em °C, dois períodos de Conforto por dia.
4. **Opções** — atrasos da bomba, tempo até "sensor sem dados", temperatura anti-gelo das férias e os extras opcionais.
5. **Gravar** — a placa reinicia para aplicar os nomes.

## Fontes de temperatura

| Fonte | Como |
|---|---|
| **Shelly H&T (Wi-Fi)** | App Shelly → sensor → *Ações* → nova ação, evento **Temperatura mudou**, URL `http://<ip-da-placa>/ufh/temp?zone=N&t=${ev.tC}` (o assistente mostra-o pronto a copiar). |
| **Sensor do Home Assistant** | Importa o blueprint `blueprints/automation/esphome_ufh_temperature_from_ha.yaml`, uma automação por zona. |
| **MQTT** | Tópico + campo JSON (Shelly: `tC`, Zigbee2MQTT: `temperature`; vazio para um número simples). O broker configura-se em *Opções*. |
| **Bluetooth BTHome** | Shelly BLU H&T, Xiaomi com firmware pvvx/ATC (BTHome v2, sem encriptação). O assistente lista os que ouve. |
| **DS18B20** | Sondas no pino 1-Wire (GPIO44 direto / GPIO15 MCP23017). O assistente lista as encontradas. |

Os Shelly H&T a pilhas só enviam quando a temperatura muda: se uma divisão fica estável muitas horas, aumenta o *Sensor sem dados após* (6 h por omissão) ou liga o sensor por USB (passa a enviar a cada 5 min). Quando um sensor fica sem dados, a zona fecha e aparece um aviso.

## Opções

| Opção | O que faz |
|---|---|
| Anti-bloqueio da bomba | Segunda às 10:00, sem aquecimento há 24 h: abre uma zona e roda a bomba 1 min |
| Anti-bloqueio dos atuadores | À mesma hora abre e fecha todas as zonas |
| Janela aberta | Queda de 1 °C em 10 min fecha a zona 30 min |
| Arranque antecipado | Aprende quanto cada zona demora a aquecer e começa até 4 h mais cedo |
| Excedente solar | +°C nas zonas escolhidas enquanto houver excedente (ação `set_pv_surplus` ou tópico MQTT) |
| Tarifa bi-horária | +°C nas horas de vazio, −°C no resto do dia |
| Ligação ao ESPHome_Altherma | Lê a ida e o retorno da outra placa para o diagrama |
| Limite do pavimento | Por zona: fecha acima de X °C medidos por uma sonda no chão |

## Modos da casa

**Casa** (normal), **Ausente** (Eco em todas as zonas), **Férias** (anti-gelo, 7 °C por omissão), **Verão** (aquecimento e bomba desligados; o anti-bloqueio continua).

## No teu YAML (placa adotada)

```yaml
substitutions:
  timezone: Europe/Lisbon          # os horários usam SNTP; o Home Assistant também o acerta
  relay1_pin: GPIO1                # placa direta: mudar um relé de GPIO
ethernet:
  manual_ip: { static_ip: 192.168.1.50, gateway: 192.168.1.1, subnet: 255.255.255.0 }
web_server:
  auth: { username: admin, password: !secret web_password }
```
