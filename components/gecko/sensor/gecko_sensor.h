#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#include "esphome/components/sensor/sensor.h"
// #include "esphome/components/voltage_sampler/voltage_sampler.h"

#include "../gecko.h"

namespace esphome {
namespace gecko {

/// Internal holder class that is in instance of Sensor so that the hub can create individual sensors.
class GeckoSensor : public sensor::Sensor,
                    public PollingComponent,
                    // public voltage_sampler::VoltageSampler,
                    public Parented<GeckoComponent> {
  public:
      void update() override;
      // void set_multiplexer(GeckoMultiplexer multiplexer) { this->multiplexer_ = multiplexer; }
      // void set_gain(GeckoGain gain) { this->gain_ = gain; }
      // void set_resolution(GeckoResolution resolution) { this->resolution_ = resolution; }
      float sample() override;

      void dump_config() override;

  protected:
      // GeckoMultiplexer multiplexer_;
      // GeckoGain gain_;
      // GeckoResolution resolution_;
};

}  // namespace gecko
}  // namespace esphome
