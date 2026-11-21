import esphome.codegen as cg
from esphome.components import switch

from . import CONF_UFH_ID, CONF_ZONE, ufh_ns, zone_schema

UFHZoneSwitch = ufh_ns.class_("UFHZoneSwitch", switch.Switch)

# "Follows Main": the zone takes its target from the main thermostat ("Geral").
CONFIG_SCHEMA = switch.switch_schema(UFHZoneSwitch, icon="mdi:link-variant").extend(zone_schema())


async def to_code(config):
    var = await switch.new_switch(config)
    parent = await cg.get_variable(config[CONF_UFH_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_zone(config[CONF_ZONE]))
    cg.add(parent.add_zone_switch(var))
