#include "gecko.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace gecko {

static const char *const TAG = "gecko";

static const uint8_t GECKO_REGISTER_CONVERSION = 0x00;
static const uint8_t GECKO_REGISTER_CONFIG = 0x01;

static const uint8_t GECKO_DATA_RATE_860_SPS = 0b111;  // 3300_SPS for ADS1015

void GeckoComponent::setup() {
    ESP_LOGCONFIG(TAG, "Setting up Gecko...");
    uint16_t value;
    if (!this->read_byte_16(GECKO_REGISTER_CONVERSION, &value)) {
        this->mark_failed();
        return;
    }

    ESP_LOGCONFIG(TAG, "Configuring Gecko...");

    // uint16_t config = 0;
    // // Clear single-shot bit
    // //        0b0xxxxxxxxxxxxxxx
    // config |= 0b0000000000000000;
    // // Setup multiplexer
    // //        0bx000xxxxxxxxxxxx
    // config |= GECKO_MULTIPLEXER_P0_N1 << 12;
    // if (!this->write_byte_16(GECKO_REGISTER_CONFIG, config)) {
    //     this->mark_failed();
    //     return;
    // }
    // this->prev_config_ = config;
}
void GeckoComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "Setting up Gecko...");
    LOG_I2C_DEVICE(this);
    if (this->is_failed()) {
        ESP_LOGE(TAG, "Communication with Gecko failed!");
    }
}
float GeckoComponent::request_measurement() {
    // uint16_t config = this->prev_config_;
    // // Multiplexer
    // //        0bxBBBxxxxxxxxxxxx
    // config &= 0b1000111111111111;
    // config |= (multiplexer & 0b111) << 12;

    // // Gain
    // //        0bxxxxBBBxxxxxxxxx
    // config &= 0b1111000111111111;
    // config |= (gain & 0b111) << 9;

    // if (!this->continuous_mode_ || this->prev_config_ != config) {
    //     if (!this->write_byte_16(GECKO_REGISTER_CONFIG, config)) {
    //         this->status_set_warning();
    //         return NAN;
    //     }
    //     this->prev_config_ = config;

    //     // in continuous mode, conversion will always be running, rely on the delay
    //     // to ensure conversion is taking place with the correct settings
    //     // can we use the rdy pin to trigger when a conversion is done?
    //     if (!this->continuous_mode_) {
    //         uint32_t start = millis();
    //         while (this->read_byte_16(GECKO_REGISTER_CONFIG, &config) && (config >> 15) == 0) {
    //         if (millis() - start > 100) {
    //             ESP_LOGW(TAG, "Reading Gecko timed out");
    //             this->status_set_warning();
    //             return NAN;
    //         }
    //         yield();
    //         }
    //     }
    // }

    uint16_t raw_conversion;
    if (!this->read_byte_16(GECKO_REGISTER_CONVERSION, &raw_conversion)) {
        this->status_set_warning();
        return NAN;
    }

    // if (resolution == ADS1015_12_BITS) {
    //     bool negative = (raw_conversion >> 15) == 1;

    //     // shift raw_conversion as it's only 12-bits, left justified
    //     raw_conversion = raw_conversion >> (16 - ADS1015_12_BITS);

    //     // check if number was negative in order to keep the sign
    //     if (negative) {
    //     // the number was negative
    //     // 1) set the negative bit back
    //     raw_conversion |= 0x8000;
    //     // 2) reset the former (shifted) negative bit
    //     raw_conversion &= 0xF7FF;
    //     }
    // }

    auto signed_conversion = static_cast<int16_t>(raw_conversion);

    float millivolts;
    float divider = 1.0; //(resolution == GECKO_16_BITS) ? 32768.0f : 2048.0f;
    millivolts = (signed_conversion * 6144) / divider;

    this->status_clear_warning();
    return millivolts / 1e3f;
}

}  // namespace gecko
}  // namespace esphome
