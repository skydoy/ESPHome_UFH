# Install

1. Open the **[web installer](https://skydoy.github.io/ESPHome_UFH/)** in **Chrome** or **Edge** on a computer.
2. Connect the Waveshare ESP32-S3-ETH by **USB-C** (a data cable, not a charge-only one) and press **Install**.
   If the port does not show up, hold **BOOT** while plugging in the cable.
3. Unplug USB and connect the **network cable** (PoE, or a normal port plus 5 V on USB-C).
4. Home Assistant shows the new device under **Settings → Devices & services → Discovered**. Press **Configure**.

There is no Wi-Fi to set up: the board uses Ethernet and gets its address by DHCP.

!!! tip "Fixed IP"
    Give the board a DHCP reservation in your router, or set a fixed address after adopting it ([Configuration](configuration.md#fixed-ip-address)).
