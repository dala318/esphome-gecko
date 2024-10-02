import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from . import I2CSnifferComponent, CONF_I2C_SNIFFER

CONF_CONNECTION_COUNT = "connection_count"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_I2C_SNIFFER): cv.use_id(I2CSnifferComponent),
        cv.Required(CONF_CONNECTION_COUNT): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    server = await cg.get_variable(config[CONF_I2C_SNIFFER])

    sens = await sensor.new_sensor(config[CONF_CONNECTION_COUNT])
    cg.add(server.set_connection_count_sensor(sens))