# Atualizações

**Firmware do instalador web:** a entidade **Atualização do firmware** instala versões novas pela rede. Com a **Atualização automática** ligada (por omissão) a placa procura todas as noites às 03:xx e instala sozinha uma versão mais nova. As zonas mantêm o estado depois do reinício.

**Adotada no Device Builder:** *Atualização disponível* e *Última versão* avisam quando sai uma versão nova; carrega em **Install** no Device Builder (o teu YAML segue o ramo `stable`).

Aviso opcional no telemóvel: importa o blueprint `blueprints/automation/esphome_ufh_update_available.yaml`.
