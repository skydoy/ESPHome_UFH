import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_TYPE,
    DEVICE_CLASS_DURATION,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_TIMESTAMP,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_CELSIUS,
    UNIT_HOUR,
    UNIT_PERCENT,
)

from . import CONF_UFH_ID, CONF_ZONE, ufh_ns, zone_schema

UFHZoneSensor = ufh_ns.class_("UFHZoneSensor", sensor.Sensor)

TYPES = {
    "runtime_today": (0, sensor.sensor_schema(UFHZoneSensor, unit_of_measurement=UNIT_HOUR, accuracy_decimals=2,
                                              device_class=DEVICE_CLASS_DURATION, state_class=STATE_CLASS_MEASUREMENT,
                                              icon="mdi:timer-outline")),
    "runtime_total": (1, sensor.sensor_schema(UFHZoneSensor, unit_of_measurement=UNIT_HOUR, accuracy_decimals=1,
                                              device_class=DEVICE_CLASS_DURATION,
                                              state_class=STATE_CLASS_TOTAL_INCREASING, icon="mdi:timer-sand")),
    "duty_24h": (2, sensor.sensor_schema(UFHZoneSensor, unit_of_measurement=UNIT_PERCENT, accuracy_decimals=0,
                                         state_class=STATE_CLASS_MEASUREMENT, icon="mdi:percent-outline")),
    "temperature": (3, sensor.sensor_schema(UFHZoneSensor, unit_of_measurement=UNIT_CELSIUS, accuracy_decimals=1,
                                            device_class=DEVICE_CLASS_TEMPERATURE,
                                            state_class=STATE_CLASS_MEASUREMENT)),
    # when the last room reading arrived (e.g. from the Shelly H&T); Home Assistant shows "5 minutes ago"
    "last_reading": (4, sensor.sensor_schema(UFHZoneSensor, accuracy_decimals=0, device_class=DEVICE_CLASS_TIMESTAMP,
                                             icon="mdi:clock-check-outline")),
    # learned rates: how fast the room warms with the zone open / cools with it closed
    "heating_rate": (5, sensor.sensor_schema(UFHZoneSensor, unit_of_measurement="°C/h", accuracy_decimals=2,
                                             state_class=STATE_CLASS_MEASUREMENT, icon="mdi:thermometer-chevron-up")),
    "cooling_rate": (6, sensor.sensor_schema(UFHZoneSensor, unit_of_measurement="°C/h", accuracy_decimals=2,
                                             state_class=STATE_CLASS_MEASUREMENT, icon="mdi:thermometer-chevron-down")),
}

CONFIG_SCHEMA = cv.typed_schema(
    {k: v[1].extend(zone_schema()) for k, v in TYPES.items()}, key=CONF_TYPE
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    parent = await cg.get_variable(config[CONF_UFH_ID])
    cg.add(var.set_zone(config[CONF_ZONE]))
    cg.add(var.set_kind(TYPES[config[CONF_TYPE]][0]))
    cg.add(parent.add_zone_sensor(var))
