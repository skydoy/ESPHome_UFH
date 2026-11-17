# Primeiros passos

## O que precisas

| Peça | Notas |
|---|---|
| **Waveshare ESP32-S3-ETH** | ESP32-S3, Ethernet W5500, USB-C. Compra a versão **com o módulo PoE** se o teu switch tiver PoE. |
| **Módulo MCP23017** (I2C, 16 E/S) | Endereço `0x20` (A0-A2 a GND). Alimentado pelos 3,3 V da placa. |
| **Placa de 16 relés** | Com optoacopladores e **jumper JD-VCC** (vê [Ligações](wiring.md)). Bobinas de 5 V ou 12 V. |
| **Fonte para os relés** | Fonte de calha DIN de 5 V ou 12 V. O PoE (802.3af, ~13 W) **não** chega para 16 bobinas. |
| Cabo de rede | Switch/injetor PoE, ou uma porta normal e 5 V no USB-C. |
| Home Assistant | Com a integração ESPHome (é adicionada sozinha quando a placa é encontrada). |

## Como funciona

- **Zona N = relé N.** Ligar a *Zona N* alimenta o(s) atuador(es) térmico(s) dessa zona.
- **Bomba** (relé 12 por omissão): em *Automático* arranca quando há pelo menos uma zona ligada, depois do *Atraso a ligar a bomba* (os atuadores demoram 2-4 min a abrir), e para o *Atraso a desligar a bomba* depois de fechar a última zona.
- **Pedido de calor** fica ligado enquanto houver uma zona ligada. Usa-o no Home Assistant para pedir calor à bomba de calor ou caldeira, ou deixa a placa ligar o relé 13 sozinha (*Controlo da bomba de calor*).
- O estado das zonas mantém-se depois de um reinício ou de uma falha de luz, por isso o aquecimento continua mesmo com o Home Assistant em baixo.

Distribuição por omissão (muda-a em [Configuração](configuration.md)):

| Relés | Uso |
|---|---|
| 1-11 | Zonas 1-11 |
| 12 | Bomba circuladora |
| 13 | Pedido de calor (contacto seco para a bomba de calor / caldeira) |
| 14-16 | Livres |
