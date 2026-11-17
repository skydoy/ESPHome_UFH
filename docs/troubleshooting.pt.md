# Problemas

| Problema | Experimenta |
|---|---|
| O Home Assistant não encontra a placa | Vê os LEDs de rede no RJ45 e o IP da placa no router. Adiciona-a à mão: integração ESPHome → host = IP. |
| LED de estado vermelho | Sem ligação de rede (cabo, PoE, porta do switch). Azul = rede mas sem Home Assistant. Verde = tudo bem. |
| A zona liga mas o relé não estala | A fonte dos relés está no JD-VCC? Verifica as ligações do MCP23017 (SDA GPIO16, SCL GPIO17) e o scan `i2c` nos registos (deve encontrar `0x20`). |
| Os relés funcionam ao contrário | Põe `relay_inverted: "false"` ([Configuração](configuration.md)). |
| Os relés vibram ou ficam meio ligados | Usa VCC = 3,3 V e tira o jumper JD-VCC ([Ligações](wiring.md)). |
| A bomba não arranca | O *Modo da bomba* está em Automático? Espera o *Atraso a ligar a bomba* (120 s por omissão) depois de ligar a primeira zona. |
| Vermelho e verde trocados no LED | Muda `channel_colors` para `RGB` em `core/boards/waveshare-esp32-s3-eth.yaml`. |

Registos: `http://<ip-da-placa>/` ou o Device Builder.
