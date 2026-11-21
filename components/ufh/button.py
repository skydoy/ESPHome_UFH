import esphome.codegen as cg
from esphome.components import button

from . import CONF_UFH_ID, CONF_ZONE, ufh_ns, zone_schema

UFHZoneButton = ufh_ns.class_("UFHZoneButton", button.Button)

# "Boost": +X °C for Y minutes on this zone (press again to stop).
CONFIG_SCHEMA = button.button_schema(UFHZoneButton, icon="mdi:rocket-launch-outline").extend(zone_schema())


async def to_code(config):
    var = await button.new_button(config)
    parent = await cg.get_variable(config[CONF_UFH_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_zone(config[CONF_ZONE]))
    cg.add(parent.add_zone_button(var))
