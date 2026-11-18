# Ligações

!!! danger "Tensão da rede"
    Atuadores térmicos, bombas e comandos de caldeira funcionam a **230 V**. Desliga o disjuntor e deixa a parte de 230 V para um eletricista.

## Opção A — relés nos pinos do ESP (até 13)

Para placas de relés com entradas de **3,3 V**, como as Denkovi comandadas por um DAEnetIP4 (as saídas dele eram de 0 / 3,3 V). O ESP substitui o DAEnetIP4: a placa de relés, as ligações de 230 V e a numeração ficam iguais.

| Relé | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| GPIO do ESP | 1 | 2 | 15 | 16 | 17 | 18 | 38 | 39 | 40 | 41 | 42 | 47 | 48 |

Liga também o **GND** do ESP ao GND da placa de relés. Solda os fios por cima da placa (o módulo PoE também fica por cima). No assistente escolhe **relés ligam com sinal ALTO**.
O barramento 1-Wire para sondas DS18B20 fica no **GPIO44** (resistência de 4,7 kΩ para os 3,3 V).

## Opção B — MCP23017 + placa de 16 relés

| ESP32-S3-ETH | MCP23017 |
|---|---|
| 3V3 / GND | VCC / GND |
| GPIO16 / GPIO17 | SDA / SCL |

A0-A2 do MCP23017 a GND (endereço `0x20`). **GPA0…GPB7 = relés 1…16.** Nas placas com optoacoplador tira o jumper **JD-VCC**, alimenta o **VCC** com 3,3 V e o **JD-VCC** com a fonte dos relés; no assistente escolhe **BAIXO**. Barramento DS18B20: **GPIO15**.

## Relés a seguir às zonas

| Função (assistente) | Liga |
|---|---|
| Bomba circuladora | bomba (com contactor se puxar mais do que o relé aguenta) |
| Pedido de calor | **entrada de termóstato ambiente** da bomba de calor / caldeira (contacto seco) |
| Bomba + pedido de calor | um só relé para as duas |

Alimentação: o módulo PoE (~13 W) alimenta só o ESP. A placa de relés continua com a fonte dela.
