# Problemas

| Problema | Experimenta |
|---|---|
| As zonas nunca aquecem | Termina o assistente (antes disso ficam desligadas). Vê os avisos no painel. |
| Aviso “sensor sem dados” | Sem leitura há mais do que *Sensor sem dados após*. Confirma o URL da ação no Shelly (número da zona) ou aumenta o tempo. |
| Aviso “sonda do chão” | A sonda do pavimento não responde. Se a zona estava a aquecer, mantém-se aberta até à hora prevista pela taxa de aquecimento aprendida; depois fecha e só volta a abrir quando a sonda responder. |
| Relés ao contrário | Assistente → Relés → ligam com sinal ALTO / BAIXO. |
| A bomba não arranca | Espera o *atraso a ligar a bomba* (120 s) depois de abrir a primeira zona; confirma a função do relé no assistente. |
| LED vermelho / azul | Vermelho: sem rede. Azul: rede sem Home Assistant (continua a aquecer). |
