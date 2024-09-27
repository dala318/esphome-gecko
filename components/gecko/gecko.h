#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
// #include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace gecko {

// class GECKOComponent : public PollingComponent, public i2c::I2CDevice {
class GECKOComponent : public PollingComponent {
    public:
        void setup() override;
        void dump_config() override;
        void update() override;
        void loop() override;

        float get_setup_priority() const override;

        void set_interrupt_pin(GPIOPin *interrupt_pin) { interrupt_pin_ = interrupt_pin; }
        void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }

    protected:
        GPIOPin *interrupt_pin_{nullptr};
        sensor::Sensor *temperature_sensor_{nullptr};
};

}  // namespace gecko
}  // namespace esphome
