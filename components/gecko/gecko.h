#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

#include <vector>

namespace esphome {
namespace gecko {

class GeckoComponent : public Component, public i2c::I2CDevice {
    public:
        void setup() override;
        void dump_config() override;
        /// HARDWARE_LATE setup priority
        float get_setup_priority() const override { return setup_priority::DATA; }
        void set_continuous_mode(bool continuous_mode) { continuous_mode_ = continuous_mode; }

        /// Helper method to request a measurement from a sensor.
        float request_measurement();
        // float request_measurement(GeckoMultiplexer multiplexer, GeckoGain gain, GeckoResolution resolution);

    protected:
        uint16_t prev_config_{0};
        bool continuous_mode_;
};

}  // namespace gecko
}  // namespace esphome
