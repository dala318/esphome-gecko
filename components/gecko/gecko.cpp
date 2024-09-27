#include "gecko.h"

#include "esphome/core/log.h"

#include <cmath>

// // TODO: How to get this from i2c bus instead of hard-coded?
// #define I2C_SDA       11
// #define I2C_SCL       12

namespace esphome {
namespace gecko {

static const char *TAG = "gecko";

void GECKOComponent::setup() {
    // uint8_t sda_pin = this->bus_->sda_pin_;
    // uint8_t scl_pin = this->bus_->scl_pin_;
    // ESP_LOGI(TAG, "SDA: %u   SCL: %u", sda_pin, scl_pin);
 
    // Wire.begin(I2C_SDA, I2C_SCL);
    // if (!gecko.begin(Wire, GECKO_L_SLAVE_ADDRESS)) {
    if (!gecko.begin(Wire, GECKO_L_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        ESP_LOGE(TAG, "Failed to find GECKO - check your wiring!");
    }
    ESP_LOGI(TAG, "Device ID: %x",gecko.getChipID());
    

    if (this->interrupt_pin_ != nullptr)  // Don't seem to work very well? Use interrupt 2
    {
        this->interrupt_pin_->setup();
        // gecko.enableINT(SensorGECKO::IntPin1);
        // gecko.enableDataReadyINT();
        // ESP_LOGI(TAG, "Interrupt 1 enabled");
    }
}

void GECKOComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "GECKO:");
    // LOG_I2C_DEVICE(this);
    // if (this->is_failed()) {
    //     ESP_LOGE(TAG, "Communication with GECKO failed!");
    // }
    LOG_UPDATE_INTERVAL(this);
    LOG_PIN("  Interrupt pin: ", this->interrupt_pin_);
    LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
}

void GECKOComponent::loop() {
    PollingComponent::loop();

    if (this->interrupt_pin_ != nullptr)
    {
        bool interrupt = this->interrupt_pin_->digital_read();
        if (interrupt)
            this->update();
    }
}

void GECKOComponent::update() {
    // if (gecko.getDataReady()) {
    //     float temperature = gecko.getTemperature_C();
    //     ESP_LOGD(TAG, ">      %lu   %.2f ℃", gecko.getTimestamp(), temperature);
    //     if (this->temperature_sensor_ != nullptr)
    //         temperature_sensor_->publish_state(temperature);

    // }
    // else{
    //     ESP_LOGE(TAG, "Data not ready");
    // }
}

float GECKOComponent::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace gecko
}  // namespace esphome
