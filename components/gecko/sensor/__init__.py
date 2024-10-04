import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor  #, voltage_sampler
from esphome.const import (
    # CONF_GAIN,
    # CONF_MULTIPLEXER,
    # CONF_RESOLUTION,
    # DEVICE_CLASS_VOLTAGE,
    CONF_ID,
    STATE_CLASS_MEASUREMENT,
    # UNIT_VOLT,
)
from .. import gecko_ns, GeckoComponent, CONF_GECKO_ID, CONF_REGISTER

# AUTO_LOAD = ["voltage_sampler"]
DEPENDENCIES = ["gecko"]
CONF_SENSOR_ID = "sensor_id"

GeckoSensor = gecko_ns.class_(
    "GeckoSensor", sensor.Sensor, cg.PollingComponent #, voltage_sampler.VoltageSampler
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        GeckoSensor,
        # unit_of_measurement=UNIT_VOLT,
        # accuracy_decimals=3,
        # device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(  # Temporary stuff to make unique sensors
        {
            cv.Required(CONF_REGISTER): cv.hex_uint16_t,
        }
    )
    .extend(
        {
            cv.GenerateID(CONF_GECKO_ID): cv.use_id(GeckoComponent),
        }
    )
    .extend(cv.polling_component_schema("60s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_GECKO_ID])

    cg.add(var.set_register(config[CONF_REGISTER]))

    # cg.add(var.set_multiplexer(config[CONF_MULTIPLEXER]))
    # cg.add(var.set_gain(config[CONF_GAIN]))
    # cg.add(var.set_resolution(config[CONF_RESOLUTION]))
