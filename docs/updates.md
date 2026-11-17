# Updates

**Web-installer firmware:** the **Firmware Update** entity installs new releases over the network. With **Auto Update** on (default) the device checks every night at 03:xx and installs a newer release by itself. Zones keep their state across the restart.

**Adopted in the Device Builder:** *Update Available* and *Latest Version* tell you about a new release; press **Install** in the Device Builder (your YAML follows the `stable` branch).

Optional notification: import the blueprint `blueprints/automation/esphome_ufh_update_available.yaml`.
