# Contribuir

Obrigado pela ajuda!

- O README fica curto; os pormenores vão para `docs/`. As docs de utilizador existem em inglês e
  em português: atualiza as duas (`page.md` / `page.pt.md`, `README.md` / `README.pt.md`).
- O que é específico da placa (pinos, chip ESP, Ethernet, expansor de I/O) fica em
  `core/boards/`; a lógica de aquecimento em `ufh/`; os serviços genéricos em `core/`.
  `devices/` só compõe os packages.
- Antes de um PR: `esphome config devices/ufh-p4-poe.yaml` e `devices/ufh-p4-poe.factory.yaml`
  passam, e o `CHANGELOG.md` tem uma linha em **Unreleased**.
- Diz o que testaste em hardware real (placa, placa de relés, atuadores), ou "não testado em
  hardware".

## Estrutura do repositório
| Pasta | O quê |
|---|---|
| `devices/` | `ufh-p4-poe.yaml` para adotar, `ufh-p4-poe.factory.yaml` = firmware do instalador web |
| `core/boards/` | Pinos, chip ESP, Ethernet, MCP23017, LED de estado |
| `ufh/` | Lógica de aquecimento: controlador (bomba, pedido de calor), zonas, termóstato opcional |
| `core/` | Serviços de rede, diagnósticos, página web, atualizações |
| `examples/` | Exemplo de configuração adotada com termóstatos no dispositivo |
| `static/` | Página do instalador web (publicada no GitHub Pages) |
| `docs/` | Guias (EN + PT); `docs/img/` logótipo |
| `tools/` | `make-logo.py` (volta a gerar o logótipo) |

## Lançar uma versão
1. Passa as notas de **Unreleased** para `## [x.y.z] - YYYY-MM-DD` e define
   `project_version: "x.y.z"` em `devices/ufh-p4-poe.yaml` e
   `devices/ufh-p4-poe-direct.yaml`.
2. Junta à `main`. Um envio para a `main` só valida e constrói artefactos; não cria
   uma tag, uma GitHub Release, nem publica firmware no GitHub Pages.
3. Quando estiveres pronto, cria e envia a tag `vx.y.z` nesse commit (tem de coincidir
   com o `project_version`). O CI constrói então o firmware, cria a GitHub Release,
   publica o instalador e os manifests OTA no GitHub Pages, e move o ramo `stable`.
   As atualizações só de documentação usam o `publish-docs.yml` e deixam o firmware
   que está no ar como está.

English version: [CONTRIBUTING.md](CONTRIBUTING.md).
