import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PORT, CONF_BUFFER_SIZE, CONF_SDA, CONF_SCL
from esphome import pins

AUTO_LOAD = ["socket"]

DEPENDENCIES = ["network"]

MULTI_CONF = True

i2c_sniffer_ns = cg.esphome_ns.namespace("i2c_sniffer")
I2CSnifferComponent = i2c_sniffer_ns.class_("I2CSnifferComponent", cg.Component)


def validate_buffer_size(buffer_size):
    if buffer_size & (buffer_size - 1) != 0:
        raise cv.Invalid("Buffer size must be a power of two.")
    return buffer_size


CONFIG_SCHEMA = cv.All(
    cv.require_esphome_version(2022, 3, 0),
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(I2CSnifferComponent),
            cv.Required(CONF_SDA): pins.gpio_input_pin_schema,
            cv.Required(CONF_SCL): pins.gpio_input_pin_schema,
            cv.Optional(CONF_PORT, default=6638): cv.port,
            cv.Optional(CONF_BUFFER_SIZE, default=128): cv.All(
                cv.positive_int, validate_buffer_size
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    sda = await cg.gpio_pin_expression(config[CONF_SDA])
    cg.add(var.set_sda_pin(sda))
    scl = await cg.gpio_pin_expression(config[CONF_SCL])
    cg.add(var.set_scl_pin(scl))
    cg.add(var.set_port(config[CONF_PORT]))
    cg.add(var.set_buffer_size(config[CONF_BUFFER_SIZE]))

    await cg.register_component(var, config)
