# FAQ

**Precisa do Home Assistant?** Não. Termóstatos, horários, bomba e pedido de calor correm na placa.

**Wi-Fi?** Não, Ethernet (o Wi-Fi não está ativo).

**Os meus sensores?** Shelly H&T (ações Wi-Fi), MQTT, Home Assistant ou DS18B20 — escolhidos por zona.

**Quantas zonas?** Até 13 nos pinos do ESP, 16 com um MCP23017 (menos os relés da bomba / pedido de calor).

**É grátis?** Para uso pessoal e não comercial ([licença](licence.md)).

## Posso acrescentar um tanque de inércia a este firmware?

Sim, a partir da 0.6.0, como **segunda placa** «UFH tanque», gravada no mesmo instalador. Vê [Tanque](tank.md). A placa das zonas só fala com ela por UDP; o relé de pedido de calor continua como está.
