# Install

1. Open the **[web installer](https://skydoy.github.io/ESPHome_UFH/)** in **Chrome** or **Edge** on a computer.
2. Choose the firmware: **UFH zonas** or **UFH tanque**. For the zones board, then choose **relays on the ESP pins** (up to 13, e.g. a Denkovi relay board) or **MCP23017** (up to 16).
3. Connect the Unit PoE-P4 through its **USB-C download port** (the one not marked “Host”) and press **Install**. If the port does not show up, hold the side button until the green LED inside lights up, then release it.
4. Connect the **network cable** (PoE, or a normal port plus 5 V on USB-C).
5. Open `http://<device-ip>/` (the IP is in your router, or in Home Assistant under *Discovered*) and run the **setup wizard**. Zones stay off until you finish it.
6. Optional: add the device to Home Assistant (**Settings → Devices & services → Discovered → Configure**). Name the zones in the wizard first: renaming later creates new entities in Home Assistant.
