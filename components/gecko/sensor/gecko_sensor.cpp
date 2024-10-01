#include "gecko_sensor.h"

#include "esphome/core/log.h"

namespace esphome {
namespace gecko {

static const char *const TAG = "gecko.sensor";

float GeckoSensor::sample() {
    return this->parent_->request_measurement();
    // return this->parent_->request_measurement(this->multiplexer_, this->gain_, this->resolution_);
}

void GeckoSensor::update() {
    float v = this->sample();
    if (!std::isnan(v)) {
        ESP_LOGD(TAG, "'%s': Got Voltage=%fV", this->get_name().c_str(), v);
        this->publish_state(v);
    }
}

void GeckoSensor::dump_config() {
    LOG_SENSOR("  ", "Gecko Sensor", this);
    // ESP_LOGCONFIG(TAG, "    Multiplexer: %u", this->multiplexer_);
    // ESP_LOGCONFIG(TAG, "    Gain: %u", this->gain_);
    // ESP_LOGCONFIG(TAG, "    Resolution: %u", this->resolution_);
}

}  // namespace gecko
}  // namespace esphome
