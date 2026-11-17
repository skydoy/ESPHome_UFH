<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/img/logo-dark.png">
    <source media="(prefers-color-scheme: light)" srcset="docs/img/logo-light.png">
    <img src="docs/img/logo-light.png" alt="ESPHome UFH" width="256">
  </picture>
</p>

<p align="center"><b>Gostas? Ajuda-me a fazer algo ainda melhor.</b><br>
  <a href="https://ko-fi.com/skydoy"><img src="https://img.shields.io/badge/Ko--fi-Support%20me-FF5E5B?logo=ko-fi&logoColor=white" alt="Support me on Ko-fi"></a>
  <a href="https://buymeacoffee.com/skydoy"><img src="https://img.shields.io/badge/Buy%20Me%20a%20Coffee-Support%20me-FFDD00?logo=buymeacoffee&logoColor=black" alt="Buy Me a Coffee"></a>
</p>

<p align="center">
  <a href="https://github.com/skydoy/ESPHome_UFH/actions/workflows/publish.yml"><img src="https://github.com/skydoy/ESPHome_UFH/actions/workflows/publish.yml/badge.svg" alt="Build"></a>
  <a href="https://github.com/skydoy/ESPHome_UFH/actions/workflows/validate.yml"><img src="https://github.com/skydoy/ESPHome_UFH/actions/workflows/validate.yml/badge.svg" alt="Validate"></a>
  <a href="https://github.com/skydoy/ESPHome_UFH/releases/latest"><img src="https://img.shields.io/github/v/release/skydoy/ESPHome_UFH?sort=semver" alt="Release"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-PolyForm%20Noncommercial%201.0.0-blue" alt="License: PolyForm Noncommercial"></a>
</p>

<p align="center"><b>O teu piso radiante no Home Assistant. Zonas, bomba e pedido de calor, por cabo de rede. Sem YAML, sem nuvem.</b><br>
🇬🇧 <a href="README.md">Read in English</a></p>

<p align="center">
  <a href="https://skydoy.github.io/ESPHome_UFH/"><img src="https://img.shields.io/badge/Abrir%20o%20instalador%20web-1784F7?style=for-the-badge&logo=esphome&logoColor=white" alt="Abrir o instalador web" height="40"></a>
</p>

## Compatibilidade

**Waveshare ESP32-S3-ETH** (Ethernet W5500, PoE opcional) + **MCP23017** + **placa de 16 relés**: até 14 zonas, a bomba circuladora e um contacto de pedido de calor. Substitui controladores de rede de 16 saídas, como o Denkovi DAEnetIP4.  
Detalhes: [o que precisas](docs/getting-started.pt.md) · [ligações](docs/wiring.pt.md)

## Instalar em 3 passos

1. **Instalar o firmware** — Abre o **[instalador web](https://skydoy.github.io/ESPHome_UFH/)**, liga a placa por USB-C → **Instalar**.
2. **Ligar** — cabo de rede (PoE, ou 5 V no USB-C), MCP23017 e placa de relés ([ligações](docs/wiring.pt.md)).
3. **Adicionar** — Home Assistant → **Configurar**. Não há Wi-Fi para configurar.

## O que tens

- Um interruptor por zona (zona N = relé N); o estado mantém-se depois de reinícios e falhas de luz
- **Bomba** em Automático / Ligada / Desligada, com atrasos para os atuadores abrirem e fecharem
- **Pedido de calor** para a bomba de calor ou caldeira (pelo Home Assistant, ou pelo relé 13)
- Termóstatos na própria placa, opcionais
- Atualizações com um clique (ou automáticas à noite)
- Página web na rede local (`http://<ip-da-placa>/`)

## Documentação e mais

[Site de documentação](https://skydoy.github.io/ESPHome_UFH/docs/pt/) ·
[Configuração](docs/configuration.pt.md) ·
[Home Assistant](docs/home-assistant.pt.md) ·
[Problemas](docs/troubleshooting.pt.md) ·
[Segurança](docs/security.pt.md) ·
[Changelog](CHANGELOG.md) ·
[Reportar um problema](https://github.com/skydoy/ESPHome_UFH/issues)

> ⚠️ Atuadores, bombas e caldeiras funcionam a 230 V: a parte de alta tensão é para um eletricista.

[PolyForm Noncommercial 1.0.0](LICENSE) — grátis para uso pessoal e não comercial; o uso comercial não é permitido.  
[Política de segurança](SECURITY.md) · [Avisos de terceiros](THIRD_PARTY_NOTICES.md)
