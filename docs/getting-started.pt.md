# Primeiros passos

## O que precisas

| Peça | Notas |
|---|---|
| **Waveshare ESP32-S3-ETH** | ESP32-S3, Ethernet W5500, USB-C. Com o **PoE Module (B)** se o teu switch tiver PoE. |
| **Relés** | Ou a tua **placa de relés com entradas de 3,3 V** (por exemplo a da Denkovi do DAEnetIP4) ligada diretamente ao ESP — até 13 relés — ou um **MCP23017** + qualquer placa de 16 relés — até 16 relés. |
| **Sensores de temperatura** (opcional) | Shelly H&T (Wi-Fi), Shelly BLU H&T / BTHome (Bluetooth), qualquer sensor MQTT ou do Home Assistant, ou sondas DS18B20. Sem sensor, a zona segue só o horário. |
| Home Assistant (opcional) | Não é preciso para aquecer: termóstatos, horários e bomba correm na placa. |

## Como funciona

- **Zona N = relé N.** Cada zona tem o seu termóstato na placa (horário Conforto / Eco, ou temperatura fixa).
- Os relés a seguir às zonas comandam a **bomba circuladora** e/ou pedem calor à **bomba de calor / caldeira**. A bomba arranca com atraso (os atuadores demoram 2-4 min a abrir) e para depois de fechar a última zona.
- **Divisões com vários circuitos:** uma zona pode seguir outra (mesmo sensor, mesmo termóstato).
- Tudo se configura num **assistente** na página web da placa. Um só firmware para todas as instalações.
- Extras opcionais: deteção de janela aberta, arranque antecipado, excedente solar, tarifa bi-horária, anti-bloqueio da bomba e dos atuadores, limite do pavimento, ligação ao ESPHome_Altherma.
