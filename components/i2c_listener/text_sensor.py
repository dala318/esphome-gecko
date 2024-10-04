import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from . import I2CListenerComponent, CONF_I2C_LISTENER

CONF_DUMP_DATA = "dump_data"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_I2C_LISTENER): cv.use_id(I2CListenerComponent),
        cv.Required(CONF_DUMP_DATA): text_sensor.text_sensor_schema(
            # state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    server = await cg.get_variable(config[CONF_I2C_LISTENER])

    sens = await text_sensor.new_text_sensor(config[CONF_DUMP_DATA])
    cg.add(server.set_dump_data_sensor(sens))