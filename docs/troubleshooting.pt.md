# Problemas

| Problema | Experimenta |
|---|---|
| As zonas nunca aquecem | Termina o assistente (antes disso ficam desligadas). Vê os avisos no painel. |
| Aviso “sensor sem dados” | Sem leitura há mais do que *Sensor sem dados após*. Confirma o URL da ação no Shelly (número da zona) ou aumenta o tempo. |
| Aviso “sonda do chão” | A sonda do chão não responde. Se a zona estava a aquecer, mantém-se aberta até à hora prevista pela taxa de aquecimento aprendida; depois fecha e só volta a abrir quando a sonda responder. |
| Relés ao contrário | Assistente → Relés → ligam com sinal ALTO / BAIXO. |
| A bomba não arranca | Espera o *atraso a ligar a bomba* (120 s) depois de abrir a primeira zona; confirma a função do relé no assistente. |
| As temperaturas do Altherma aparecem como NA, ou aviso “Ligação Altherma: HTTP 404” | O servidor web da outra placa (ESPHome_Altherma 2.6.3) exige o nome exacto do sensor. Os caminhos por omissão são `/sensor/Leaving%20Water%20Temp%20after%20BUH%20(R2T)`, `/sensor/Inlet%20Water%20Temp%20(R4T)` e `/sensor/Outdoor%20Air%20Temp%20(R1T)` (ERLA D). Confirma um caminho no browser: `http://<ip-do-altherma>/sensor/Inlet%20Water%20Temp%20(R4T)` deve devolver um número. |
| LED vermelho / azul | Vermelho: sem rede. Azul: rede sem Home Assistant (continua a aquecer). |
