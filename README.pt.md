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

**M5Stack Unit PoE-P4** (Ethernet + PoE, já vem em caixa) com:
- **relés diretamente nos pinos do ESP** — até 13 (placas com entradas de 3,3 V, por exemplo a placa de relés Denkovi de um DAEnetIP4), ou
- **MCP23017** + qualquer placa de 16 relés — até 16.

Detalhes: [o que precisas](docs/getting-started.pt.md) · [ligações](docs/wiring.pt.md)

## Instalar em 3 passos

1. **Instalar o firmware** — Abre o **[instalador web](https://skydoy.github.io/ESPHome_UFH/)**, escolhe **UFH zonas** ou **UFH tanque**, liga-a por USB-C → **Instalar**.
2. **Ligar** — cabo de rede (PoE, ou 5 V no USB-C) e os relés ([ligações](docs/wiring.pt.md)).
3. **Configurar** — abre `http://<ip-da-placa>/` e segue o **assistente** (zonas, sensores, horários). O Home Assistant é opcional.

## O que tens

- **Termóstatos na própria placa**: Conforto / Eco por horário, funciona sem Home Assistant
- Sensores à escolha por zona: **Shelly H&T** (Wi-Fi), **MQTT**, **Home Assistant** ou **DS18B20**
- **Bomba** com atrasos e **pedido de calor** à bomba de calor / caldeira
- **Diagrama em tempo real** na página web, assistente, editor de horários, equilíbrio das zonas
- Opcionais: janela aberta, arranque antecipado, excedente solar, tarifa bi-horária, anti-bloqueio, limite do pavimento, ligação ao ESPHome_Altherma, segunda placa para o [tanque de inércia](docs/tank.pt.md)
- Atualizações com um clique (ou automáticas à noite)

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
