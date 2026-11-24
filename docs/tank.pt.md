# Tanque

Módulo opcional no firmware da UFH (0.6.0). Vem desligado: a bomba, o pedido de calor e as zonas ficam como estavam até o ligares no configurador.

## O que acrescenta

Uma bomba de recirculação nova e uma resistência eléctrica de emergência num tanque pulmão, mais uma DS18B20 no meio do tanque (o mesmo barramento 1-Wire das sondas do chão, **GPIO11**).

Funções recomendadas na placa MCP23017 (16 relés):

| Relé | Função |
| --- | --- |
| 1–11 | Zonas |
| 12 | Pedido de calor (sem mudanças) |
| 13 | Bomba do tanque |
| 14 | Resistência de emergência |

A placa oficial com 13 relés em GPIO directo não consegue ficar com 11 zonas, o pedido no 12, a bomba do tanque no 13 **e** a resistência no 14. Tira uma zona, ou junta o pedido à circuladora que já tens, antes de atribuíres a resistência.

## Bomba

A bomba do tanque arranca **assim que alguma zona pede calor**, mesmo com o tanque frio. Usa o mesmo tempo de paragem da circuladora (`pump_stop_s`) depois de fechar a última zona. Também corre no anti-bloqueio semanal da bomba. No Verão fica parada (excepto no anti-bloqueio). A função da circuladora actual não muda.

## Resistência de emergência

A resistência só entra **sozinha** quando tudo isto é verdade:

- o módulo do tanque está ligado
- alguma zona pede calor e a casa não está em Verão
- a sonda do tanque tem uma leitura recente
- a temperatura do tanque está no **alvo − 3 °C** ou abaixo (alvo por omissão 40 °C)
- o UDP da placa Altherma escolhida está fresco (menos de 60 s) e o `altherma_fault` está ligado

Nunca arranca sozinha por causa da reserva HTTP, de uma avaria antiga ou desconhecida, ou do compressor a 0.

**Não tem limite de tempo.** O corte por software é **45 °C** (automático e manual). Mínimo ligada 2 min, mínimo desligada 5 min, excepto se o corte disparar.

**É obrigatório um termóstato mecânico em série com a resistência.** O corte por software é só um segundo nível. Não há válvula misturadora entre o tanque e o coletor, por isso a água do tanque vai para a betonilha. Não subas o corte acima de 45 °C.

### Resistência manual

Se o compressor ficar a 0 com pedido de calor, a UFH avisa-te ao fim de 15 minutos (configurável, 5–120) e és tu que ligas a resistência à mão: **Resistência manual** no painel web ou no Home Assistant. O corte a 45 °C continua a valer. Se a desligares durante uma avaria, o automático não a volta a ligar até a avaria passar ou voltares a ligar o interruptor manual. Depois de um reinício, o manual começa desligado.

## Sonda

No configurador escolhe a DS18B20 do tanque (o assistente lista as sondas no GPIO11). Sem uma leitura válida e recente a resistência fica desligada e o painel mostra “sonda do tanque”.

## Altherma por UDP

A UFH escuta na **porta UDP 18511** os pacotes `packet_transport` sem encriptação da ESPHome_Altherma. Mostra as placas que ouve na rede e tu escolhes uma. Até escolheres, as temperaturas ficam desconhecidas e a resistência nunca arranca sozinha.

Se o UDP ficar 60 s sem dados e tiveres posto o endereço HTTP da Altherma, a **ida, o retorno e o exterior** passam a vir pela ligação HTTP da 0.5.3. A avaria, os códigos de erro e a frequência do compressor só vêm por UDP.

Na placa Altherma, acrescenta o pacote UDP no Device Builder (sem encriptação) e instala a partir daí. As duas placas têm de estar na mesma rede de confiança: os valores viajam em claro.

Os bytes de erro são descodificados para códigos de serviço (55 → E7, 147 → U3). O `altherma_fault` fica ligado quando o erro interior ou o exterior é diferente de 0.

Vê também [Configuração](configuration.md), [Ligações](wiring.md), [Segurança](security.md) e [Home Assistant](home-assistant.md).
