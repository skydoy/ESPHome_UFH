# FAQ

**Precisa do Home Assistant para continuar a aquecer?**
Não. O estado das zonas fica guardado na placa e mantém-se depois de reinícios e falhas de luz; a lógica da bomba corre na placa. As decisões dos termóstatos ficam no Home Assistant, a não ser que juntes os [termóstatos na placa](configuration.md#thermostats-on-the-device).

**Wi-Fi?**
Não, só Ethernet (mais fiável num quadro elétrico). O firmware pré-compilado não liga o Wi-Fi.

**Posso usar as minhas sondas de temperatura?**
Sim, DS18B20 no GPIO15, vê [Configuração](configuration.md#thermostats-on-the-device).

**Quantas zonas?**
Até 14 com um MCP23017 / placa de 16 relés (mais a bomba e o pedido de calor).

**É grátis?**
Para uso pessoal e não comercial, sim ([licença](licence.md)).
