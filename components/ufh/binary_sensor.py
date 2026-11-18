import esphome.codegen as cg
from esphome.components import binary_sensor
from esphome.const import DEVICE_CLASS_HEAT

from . import CONF_UFH_ID, CONF_ZONE, ufh_ns, zone_schema

UFHZoneBinarySensor = ufh_ns.class_("UFHZoneBinarySensor", binary_sensor.BinarySensor)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(
    UFHZoneBinarySensor, device_class=DEVICE_CLASS_HEAT, icon="mdi:heating-coil"
).extend(zone_schema())


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    parent = await cg.get_variable(config[CONF_UFH_ID])
    cg.add(var.set_zone(config[CONF_ZONE]))
    cg.add(parent.add_zone_binary_sensor(var))
