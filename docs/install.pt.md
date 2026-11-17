# Instalar

1. Abre o **[instalador web](https://skydoy.github.io/ESPHome_UFH/)** no **Chrome** ou no **Edge** num computador.
2. Liga a Waveshare ESP32-S3-ETH por **USB-C** (cabo de dados, não só de carregar) e carrega em **Instalar**.
   Se a porta não aparecer, mantém o botão **BOOT** carregado ao ligar o cabo.
3. Tira o USB e liga o **cabo de rede** (PoE, ou uma porta normal e 5 V no USB-C).
4. O Home Assistant mostra a placa nova em **Definições → Dispositivos e serviços → Descobertos**. Carrega em **Configurar**.

Não há Wi-Fi para configurar: a placa usa Ethernet e recebe o endereço por DHCP.

!!! tip "IP fixo"
    Reserva um endereço DHCP para a placa no router, ou define um IP fixo depois de a adotares ([Configuração](configuration.md#fixed-ip-address)).
