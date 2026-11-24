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
| **DS18B20** | Sondas no pino 1-Wire (GPIO11, ficha SDIO pino 3). O assistente lista as encontradas. |

Os Shelly H&T a pilhas só enviam quando a temperatura muda: se uma divisão fica estável muitas horas, aumenta o *Sensor sem dados após* (6 h por omissão) ou liga o sensor por USB (passa a enviar a cada 5 min). Quando um sensor fica sem dados, a zona fecha e aparece um aviso.

## Opções

| Opção | O que faz |
|---|---|
| Anti-bloqueio da bomba | Segunda às 10:00, sem aquecimento há 24 h: abre uma zona e roda a bomba 1 min |
| Anti-bloqueio dos atuadores | À mesma hora abre e fecha todas as zonas |
| Janela aberta | Queda de 1 °C em 10 min fecha a zona 30 min |
| Arranque antecipado | Aprende quanto cada zona demora a aquecer e começa até 4 h mais cedo |
| Paragem antecipada | Aprende quanto cada divisão arrefece com a zona fechada e fecha até 90 min antes do fim do Conforto se não descer mais de 0,5 °C até lá |
| Excedente solar | +°C nas zonas escolhidas enquanto houver excedente (ação `set_pv_surplus` ou tópico MQTT) |
| Tarifa bi-horária | +°C nas horas de vazio, −°C no resto do dia |
| Ligação ao ESPHome_Altherma | A partir da 0.6.0 a UFH escuta na porta UDP 18511 e lista as placas Altherma na rede; tu escolhes uma. Valores com mais de 60 s ficam desconhecidos. Se o UDP estiver calado e tiveres posto um endereço HTTP, a ida / o retorno / o exterior passam a vir da página web da outra placa (os caminhos por omissão continuam a ser os nomes da ESPHome_Altherma 2.6.3 numa ERLA D: `/sensor/Leaving%20Water%20Temp%20after%20BUH%20(R2T)`, `/sensor/Inlet%20Water%20Temp%20(R4T)` e `/sensor/Outdoor%20Air%20Temp%20(R1T)`). Os caminhos que escreves são codificados no URL. A avaria, os códigos de erro e a frequência do compressor só vêm por UDP. |
| Ligação ao tanque | A partir da 0.6.0 a placa das zonas lista as placas do tanque na rede e tu escolhes uma. Envia o pedido de calor e o número de zonas abertas na UDP 18511; a temperatura, a bomba e a resistência voltam. Vê [Tanque](tank.md). |
| Limite do pavimento | Por zona: fecha acima de X °C medidos por uma sonda do chão. Se a sonda deixar de responder enquanto a zona está a aquecer, a zona mantém-se aberta até à hora prevista pela taxa de aquecimento aprendida; depois fecha e só volta a abrir quando houver uma leitura válida da sonda do chão. As entidades *Avisos* e *Problema* no Home Assistant mostram “sonda do chão”. |

## Zonas externas (pela rede)

Zonas cujo relé está noutro aparelho. Configuram-se no passo 1 do assistente (**Zonas externas**); ficam a
seguir às zonas da placa (com 11 zonas na placa, a primeira zona externa é a zona 12). Até 8, e até 16
zonas no total.

| Aparelho | Como é comandado |
|---|---|
| Shelly (Plus, Pro, Gen3) | `http://<ip>/rpc/Switch.Set?id=<relé>&on=true&toggle_after=300` / `on=false`. O Shelly não pode ter palavra-passe. |
| URL HTTP (qualquer aparelho) | Um URL para ligar e outro para desligar. Tem de responder 200 OK. |

A ordem é enviada a cada mudança e repetida a cada minuto. Com um Shelly, o "ligar" vai com um temporizador
de 5 minutos: se esta placa deixar de responder, o Shelly fecha a zona sozinho. Se um aparelho não responder
durante 2 minutos, a zona mostra o aviso **aparelho sem resposta**.

Cada zona mostra também, no seu grupo da página da placa e no Home Assistant, a **velocidade de aquecimento**
e de **arrefecimento** aprendidas (°C/h).

## Modos da casa

**Casa** (normal), **Ausente** (Eco em todas as zonas), **Férias** (anti-gelo, 7 °C por omissão, inclusive nas zonas em Off), **Verão** (sem anti-gelo; aquecimento e bomba desligados; o anti-bloqueio da bomba e dos atuadores continua). Uma zona sem sonda de ambiente não pode fazer anti-gelo (não há temperatura): em Férias fica desligada, e só o anti-bloqueio da válvula continua.

## No teu YAML (placa adotada)

```yaml
substitutions:
  timezone: Europe/Lisbon          # os horários usam SNTP; o Home Assistant também o acerta
  relay1_pin: GPIO44               # placa direta: mudar um relé de GPIO
ethernet:
  manual_ip: { static_ip: 192.168.1.50, gateway: 192.168.1.1, subnet: 255.255.255.0 }
web_server:
  auth: { username: admin, password: !secret web_password }
```
