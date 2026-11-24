"""ESPHome UFH tank board: buffer-tank pump, emergency heater and DS18B20.

Runtime-configured: the device web page stores providers, temperatures and PWM
settings in NVS. The board package only lists the two relays, the PWM output
and the 1-Wire bus.
"""

import esphome.codegen as cg
from esphome.components import output, time as time_, web_server_base
from esphome.components.one_wire import OneWireBus
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_TIME_ID

CODEOWNERS = ["@skydoy"]
DEPENDENCIES = ["network", "web_server_base"]
AUTO_LOAD = ["json", "sensor", "binary_sensor", "switch", "number"]

CONF_UFH_TANK_ID = "ufh_tank_id"
CONF_PUMP = "pump"
CONF_HEATER = "heater"
CONF_PWM = "pwm"
CONF_RELAYS_ACTIVE_LOW = "relays_active_low"
CONF_BOARD_NAME = "board_name"
CONF_PWM_PIN = "pwm_pin"
CONF_ONE_WIRE_ID = "one_wire_id"
CONF_WEB_SERVER_BASE_ID = "web_server_base_id"

ufh_tank_ns = cg.esphome_ns.namespace("ufh_tank")
TankComponent = ufh_tank_ns.class_("TankComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(TankComponent),
        cv.GenerateID(CONF_WEB_SERVER_BASE_ID): cv.use_id(web_server_base.WebServerBase),
        cv.Required(CONF_PUMP): cv.use_id(output.BinaryOutput),
        cv.Required(CONF_HEATER): cv.use_id(output.BinaryOutput),
        cv.Required(CONF_PWM): cv.use_id(output.FloatOutput),
        cv.Required(CONF_TIME_ID): cv.use_id(time_.RealTimeClock),
        cv.Optional(CONF_RELAYS_ACTIVE_LOW, default=False): cv.boolean,
        cv.Optional(CONF_BOARD_NAME, default=""): cv.string,
        cv.Optional(CONF_PWM_PIN, default=10): cv.int_range(min=0, max=54),
        cv.Optional(CONF_ONE_WIRE_ID): cv.use_id(OneWireBus),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    pump = await cg.get_variable(config[CONF_PUMP])
    cg.add(var.set_pump(pump))
    heater = await cg.get_variable(config[CONF_HEATER])
    cg.add(var.set_heater(heater))
    pwm = await cg.get_variable(config[CONF_PWM])
    cg.add(var.set_pwm(pwm))
    t = await cg.get_variable(config[CONF_TIME_ID])
    cg.add(var.set_time(t))
    base = await cg.get_variable(config[CONF_WEB_SERVER_BASE_ID])
    cg.add(var.set_web_base(base))
    cg.add(var.set_default_active_low(config[CONF_RELAYS_ACTIVE_LOW]))
    cg.add(var.set_board_name(config[CONF_BOARD_NAME]))
    cg.add(var.set_pwm_pin(config[CONF_PWM_PIN]))
    if CONF_ONE_WIRE_ID in config:
        cg.add_define("USE_UFH_ONE_WIRE")
        bus = await cg.get_variable(config[CONF_ONE_WIRE_ID])
        cg.add(var.set_one_wire_bus(bus))
