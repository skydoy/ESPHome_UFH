# Tanque de inércia

A partir da 0.6.0 o tanque de inércia é uma **segunda placa**, «UFH tanque», no mesmo repositório e no mesmo instalador. Usa-a quando o tanque fica longe da caixa das zonas e não há como passar cabos. As duas placas saem sempre com a mesma versão.

## Duas placas

| Placa | O que faz |
| --- | --- |
| **UFH zonas** | Zonas, circuladora, contacto de pedido de calor (relé 12). Envia por UDP o `ufh_heat_demand` e o `ufh_open_zones`. Mostra no painel a temperatura, a bomba e a resistência do tanque, depois de escolheres a placa do tanque. |
| **UFH tanque** | Ao lado do tanque: relé da bomba, relé da resistência de emergência, DS18B20 no meio do tanque (GPIO11), PWM opcional para a velocidade da bomba. Ouve a UFH e a Altherma. Envia de volta a temperatura e o estado da bomba e da resistência. |

No instalador escolhes **UFH zonas** ou **UFH tanque** e gravas. Em cada placa, o configurador lista as que ouve; tu escolhes (na placa do tanque há duas listas: UFH e Altherma).

As três placas falam `packet_transport` **sem encriptação**, na **porta UDP 18511**, a cada 15 s, na mesma rede de confiança.

## Ligações (placa do tanque, PoE-P4)

| Pino | Função |
| --- | --- |
| GPIO44 (Hat2-Bus pino 2) | Relé da alimentação da bomba (230 V) |
| GPIO43 (Hat2-Bus pino 4) | Relé da resistência de emergência |
| GPIO10 (SDIO pino 2) | Sinal PWM para a bomba (opcional) |
| GPIO11 (SDIO pino 3) | DS18B20, no meio do tanque, o mesmo 1-Wire das zonas |

**Um termóstato mecânico em série com a resistência é obrigatório.** Não há válvula misturadora: a água do tanque vai para a betonilha. O corte por software a 45 °C é só o segundo nível. Não o subas.

## Bomba (Grundfos UPML 25-95 130)

O **relé de alimentação** da bomba liga sempre que a UFH envia um pedido de calor, mesmo com o tanque frio. Segue o mesmo over-run da circuladora depois de fechar a última zona. Fica parada no Verão (excepto um anti-bloqueio breve enviado como pedido). **Se os dados da UFH ficarem velhos durante 60 s, a bomba para** (os atuadores das zonas, na outra caixa, provavelmente fecham).

A velocidade é opcional, por **PWM**, conforme o número de zonas abertas (velocidade mín./máx. e quantas zonas correspondem ao máximo, configuráveis). Sem pedido, o relé está desligado e não há PWM.

Na gama oficial, a **UPML 25-95 130 aparece como AUTO** (controlo interno, sem cabo PWM). O PWM na família UPML é sobretudo o da 25-105, **perfil A (aquecimento)**. Confirma na chapa o campo «Control signal». O perfil A é invertido: um duty cycle maior quer dizer uma velocidade menor. Sem sinal, uma bomba de perfil A **corre à velocidade máxima** ([data booklet Medium UPM da Grundfos](https://api.grundfos.com/literature/Grundfosliterature-5439385.pdf)). Se a chapa disser AUTO, deixa o fio de PWM desligado; o relé on/off chega.

## Resistência de emergência

A resistência só entra **sozinha** se tudo isto for verdade:

- a ligação à UFH está fresca (menos de 60 s) e há pedido de calor
- a sonda do tanque tem uma leitura recente
- a temperatura está no **alvo − 3 °C** ou abaixo (alvo por omissão 40 °C)
- o UDP da placa Altherma escolhida está fresco e o `altherma_fault` está ligado

Nunca entra sozinha por causa de uma avaria antiga ou desconhecida, nem só do compressor a 0.

**Não tem limite de tempo.** O corte por software é **45 °C** (automático e manual). Mínimo ligada 2 min, mínimo desligada 5 min, excepto se o corte disparar. Se a sonda falhar, ou os dados da UFH ficarem velhos, a resistência fica desligada (automática e manual).

### Resistência manual

Se o compressor estiver a 0 com pedido de calor, a placa do tanque avisa-te ao fim de 15 minutos (ajustável, 5–120) e és tu que a ligas à mão, no painel da placa do tanque ou no Home Assistant. O corte a 45 °C vale sempre. Depois de um reinício, o manual começa desligado.

## Altherma

A placa do tanque lista as placas Altherma que ouve na rede e tu escolhes uma, como na placa das zonas. A avaria, a frequência do compressor e a temperatura exterior vêm só por UDP. A placa das zonas pode continuar a usar a reserva HTTP da 0.5.3 para a ida, o retorno e o exterior se o UDP estiver calado; essa reserva nunca liga a resistência.

Os bytes de erro são descodificados na placa das zonas (55 → E7, 147 → U3).

Ver também [Configuração](configuration.md), [Ligações](wiring.md), [Segurança](security.md) e [Home Assistant](home-assistant.md).
