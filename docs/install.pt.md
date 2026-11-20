# Instalar

1. Abre o **[instalador web](https://skydoy.github.io/ESPHome_UFH/)** no **Chrome** ou no **Edge** num computador.
2. Escolhe a placa: **relés nos pinos do ESP** (até 13, por exemplo a placa de relés Denkovi) ou **MCP23017** (até 16).
3. Liga a Unit PoE-P4 pela porta **USB-C de programação** (a que não diz “Host”) e carrega em **Instalar**. Se a porta não aparecer, mantém carregado o botão da lateral até o LED verde interior acender e solta-o.
4. Liga o **cabo de rede** (PoE, ou uma porta normal e 5 V no USB-C).
5. Abre `http://<ip-da-placa>/` (o IP aparece no router ou no Home Assistant em *Descobertos*) e faz o **assistente de configuração**. As zonas ficam desligadas até o terminares.
6. Opcional: adiciona a placa ao Home Assistant (**Definições → Dispositivos e serviços → Descobertos → Configurar**). Dá nomes às zonas no assistente antes: mudar nomes depois cria entidades novas no Home Assistant.
