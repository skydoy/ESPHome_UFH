import esphome.codegen as cg
from esphome.components import climate
import esphome.config_validation as cv

from . import CONF_UFH_ID, CONF_ZONE, ufh_ns, zone_schema

UFHZoneClimate = ufh_ns.class_("UFHZoneClimate", climate.Climate, cg.Component)

CONFIG_SCHEMA = (
    climate.climate_schema(UFHZoneClimate)
    .extend(zone_schema())
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await climate.new_climate(config)
    await cg.register_component(var, config)
    parent = await cg.get_variable(config[CONF_UFH_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_zone(config[CONF_ZONE]))
    cg.add(parent.add_climate(var))
