"""ESPHome UFH: underfloor heating controller (zones, pump, heat demand, thermostats).

Runtime-configured: the device web page runs a setup wizard and stores everything in NVS,
so one firmware serves every installation. The board package only lists the relay outputs.
"""

import esphome.codegen as cg
from esphome.components import esp32_ble_tracker, output, time as time_, web_server_base
from esphome.components.one_wire import OneWireBus
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_TIME_ID

CODEOWNERS = ["@skydoy"]
DEPENDENCIES = ["network", "web_server_base"]
AUTO_LOAD = ["json", "climate", "sensor", "binary_sensor"]

CONF_UFH_ID = "ufh_id"
CONF_RELAYS = "relays"
CONF_RELAYS_ACTIVE_LOW = "relays_active_low"
CONF_BOARD_NAME = "board_name"
CONF_ONE_WIRE_ID = "one_wire_id"
CONF_ESP32_BLE_ID = "esp32_ble_id"
CONF_WEB_SERVER_BASE_ID = "web_server_base_id"
CONF_ZONE = "zone"

ufh_ns = cg.esphome_ns.namespace("ufh")
UFHComponent = ufh_ns.class_("UFHComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(UFHComponent),
        cv.GenerateID(CONF_WEB_SERVER_BASE_ID): cv.use_id(web_server_base.WebServerBase),
        cv.Required(CONF_RELAYS): cv.All(
            cv.ensure_list(cv.use_id(output.BinaryOutput)), cv.Length(min=2, max=16)
        ),
        cv.Required(CONF_TIME_ID): cv.use_id(time_.RealTimeClock),
        cv.Optional(CONF_RELAYS_ACTIVE_LOW, default=False): cv.boolean,
        cv.Optional(CONF_BOARD_NAME, default=""): cv.string,
        cv.Optional(CONF_ONE_WIRE_ID): cv.use_id(OneWireBus),
        cv.Optional(CONF_ESP32_BLE_ID): cv.use_id(esp32_ble_tracker.ESP32BLETracker),
    }
).extend(cv.COMPONENT_SCHEMA)


def zone_schema(max_zone=16):
    return cv.Schema(
        {
            cv.GenerateID(CONF_UFH_ID): cv.use_id(UFHComponent),
            cv.Required(CONF_ZONE): cv.int_range(min=1, max=max_zone),
        }
    )


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    for rid in config[CONF_RELAYS]:
        r = await cg.get_variable(rid)
        cg.add(var.add_relay(r))
    t = await cg.get_variable(config[CONF_TIME_ID])
    cg.add(var.set_time(t))
    base = await cg.get_variable(config[CONF_WEB_SERVER_BASE_ID])
    cg.add(var.set_web_base(base))
    cg.add(var.set_default_active_low(config[CONF_RELAYS_ACTIVE_LOW]))
    cg.add(var.set_board_name(config[CONF_BOARD_NAME]))
    if CONF_ONE_WIRE_ID in config:
        cg.add_define("USE_UFH_ONE_WIRE")
        bus = await cg.get_variable(config[CONF_ONE_WIRE_ID])
        cg.add(var.set_one_wire_bus(bus))
    if CONF_ESP32_BLE_ID in config:
        cg.add_define("USE_UFH_BLE")
        await esp32_ble_tracker.register_ble_device(var, config)
