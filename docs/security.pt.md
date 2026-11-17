# Segurança

O firmware pré-compilado **não tem chave da API, palavra-passe de OTA nem de página web**, para qualquer Home Assistant o poder adotar. Depois de o adotares, junta à tua configuração e instala uma vez:

- `api: encryption: key:` (o Device Builder normalmente já gera uma)
- `ota: - platform: esphome password:`
- `web_server: auth:` se outras pessoas usarem a tua rede

As atualizações vêm do GitHub Pages por HTTPS (com o certificado verificado). Comunica problemas em privado: [aviso de segurança](https://github.com/skydoy/ESPHome_UFH/security/advisories/new).
