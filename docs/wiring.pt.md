# Ligações

!!! danger "Tensão da rede"
    Os atuadores térmicos, as bombas e os comandos das caldeiras funcionam normalmente a **230 V**. Desliga o disjuntor antes de mexer e deixa a parte de 230 V para um eletricista.

## Parte lógica (baixa tensão)

| Waveshare ESP32-S3-ETH | Módulo MCP23017 |
|---|---|
| 3V3 | VCC |
| GND | GND |
| GPIO16 | SDA |
| GPIO17 | SCL |

A0, A1 e A2 do MCP23017 a GND (endereço `0x20`), RESET a VCC. Canal **GPA0 = relé 1 … GPB7 = relé 16**.

## MCP23017 → placa de relés

A maior parte das placas de 16 relés liga um relé com a **entrada em LOW** (`relay_inverted: "true"`, a opção por omissão).

1. **Tira o jumper JD-VCC.**
2. **VCC** (lado das entradas / optoacopladores) → **3,3 V** da placa ESP. Assim o MCP23017 a 3,3 V comanda bem as entradas.
3. **JD-VCC** + GND (lado das bobinas) → a **fonte dos relés** (5 V ou 12 V, conforme as bobinas).
4. **IN1 … IN16** → **GPA0 … GPB7** do MCP23017.

Sem o jumper, o lado do ESP e as bobinas dos relés ficam isolados um do outro.

## Relés (alta tensão)

| Relé | Liga |
|---|---|
| 1 … 11 | Fase do(s) atuador(es) térmico(s) de cada zona, pelos contactos **COM / NA** |
| 12 | Bomba circuladora (através de um contactor se a bomba puxar mais do que o relé aguenta) |
| 13 | Entrada de **termóstato ambiente** da bomba de calor / caldeira (contacto seco, COM / NA) |

Os atuadores térmicos costumam ser **NF** (normalmente fechados): alimentado = aberto = a aquecer essa zona.

## Substituir um Denkovi DAEnetIP4

Mantém a mesma ordem: saída N do Denkovi → relé N. As zonas 1-11 ficam nas saídas 1-11 e a bomba na saída 12, por isso as ligações no quadro não mudam. Remove do Home Assistant a integração que falava com o Denkovi (SNMP) e passa a usar as entidades novas do ESPHome.
