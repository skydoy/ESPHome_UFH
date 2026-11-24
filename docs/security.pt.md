# Segurança

O firmware pré-compilado **não tem chave da API, palavra-passe de OTA nem de página web**, para qualquer Home Assistant o poder adotar. Depois de o adotares, junta à tua configuração e instala uma vez:

- `api: encryption: key:` (o Device Builder normalmente já gera uma)
- `ota: - platform: esphome password:`
- `web_server: auth:` se outras pessoas usarem a tua rede

As atualizações vêm do GitHub Pages por HTTPS (com o certificado verificado). Comunica problemas em privado: [aviso de segurança](https://github.com/skydoy/ESPHome_UFH/security/advisories/new).

A ligação UDP à Altherma (porta 18511) vai **sem encriptação**. Quem estiver na mesma rede pode ler a ida, o retorno, a temperatura exterior, a frequência do compressor e as avarias, e pode forjar um pacote de avaria ou de “tudo bem”. Só a ligues numa rede de confiança (o mesmo switch da placa Altherma). A reserva HTTP para a página web da Altherma também não tem autenticação na rede local, como o resto do servidor web da UFH.
