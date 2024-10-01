#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/socket/socket.h"

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif

#include <Arduino.h>

#include <memory>
#include <string>
#include <vector>

#define I2C_IDLE 0
#define I2C_TRX 2

namespace esphome {
namespace i2c_sniffer {

class I2CSnifferComponent : public esphome::Component {
    public:
        I2CSnifferComponent() = default;
        void set_sda_pin(esphome::InternalGPIOPin *sda_pin) { sda_pin_ = sda_pin; }
        void set_scl_pin(esphome::InternalGPIOPin *scl_pin) { scl_pin_ = scl_pin; }
        void set_buffer_size(size_t size) { this->buf_size_ = size; }
        void set_port(uint16_t port) { this->port_ = port; }

#ifdef USE_BINARY_SENSOR
        void set_connected_sensor(esphome::binary_sensor::BinarySensor *connected) { this->connected_sensor_ = connected; }
#endif
#ifdef USE_SENSOR
        void set_connection_count_sensor(esphome::sensor::Sensor *connection_count) { this->connection_count_sensor_ = connection_count; }
#endif

        void setup() override;
        void loop() override;
        void dump_config() override;
        void on_shutdown() override;

        float get_setup_priority() const override { return esphome::setup_priority::AFTER_WIFI; }

    protected:
        void publish_sensor();

        void accept();
        void cleanup();
        void read();
        void flush();
        void empty_sockets();

        // I2C Specific
        esphome::InternalGPIOPin *sda_pin_{nullptr};
        esphome::InternalGPIOPin *scl_pin_{nullptr};

        // Serial stream specific
        size_t buf_index(size_t pos) { return pos & (this->buf_size_ - 1); }
        /// Return the number of consecutive elements that are ahead of @p pos in memory.
        size_t buf_ahead(size_t pos) { return (pos | (this->buf_size_ - 1)) - pos + 1; }

        struct Client {
            Client(std::unique_ptr<esphome::socket::Socket> socket, std::string identifier, size_t position);

            std::unique_ptr<esphome::socket::Socket> socket{nullptr};
            std::string identifier{};
            bool disconnected{false};
            size_t position{0};
        };

        uint16_t port_;
        size_t buf_size_;

        // Sniffer specific
        uint8_t i2c_status_ = I2C_IDLE;         // Status of the I2C BUS
        // uint32_t last_start_millis_ = 0;        // Store the last time
        uint8_t data_buffer_[9600];             // Array for storing data of the I2C communication
        uint16_t buffer_poi_w_ = 0;             // Points to the first empty position in the dataBufer to write
        uint16_t buffer_poi_r_ = 0;             // Points to the position where to start read from
        uint8_t bit_count_ = 0;                 // Counter of bit appeared on the BUS
        uint16_t byte_count_ = 0;               // Counter of bytes were writen in one communication.
        // uint8_t i2c_bit_d_ = 0;                 // Container of the actual SDA bit
        // uint8_t i2c_bit_d2_ = 0;                // Container of the actual SDA bit
        // uint8_t i2c_bit_c_ = 0;                 // Container of the actual SDA bit
        // uint8_t i2c_clk_ = 0;                   // Container of the actual SCL bit
        // uint8_t i2c_ack_ = 0;                   // Container of the last ACK value
        // uint8_t i2c_case_ = 0;                  // Container of the last ACK value
        // uint8_t resp_count_ = 0;                // Auxiliary variable to help detect next byte instead of STOP these variables just for statistic reasons
        uint16_t false_start_cnt_ = 0;          // Auxiliary variable to count false start events
        uint16_t scl_up_cnt_ = 0;               // Auxiliary variable to count rising SCL
        uint16_t sda_up_cnt_ = 0;               // Auxiliary variable to count rising SDA
        uint16_t sda_down_cnt_ = 0;             // Auxiliary variable to count falling SDA

        static void IRAM_ATTR i2c_trigger_on_raising_scl(I2CSnifferComponent *sniffer);
        static void IRAM_ATTR i2c_trigger_on_change_sda(I2CSnifferComponent *sniffer);
        void reset_i2c_variables();


#ifdef USE_BINARY_SENSOR
        esphome::binary_sensor::BinarySensor *connected_sensor_;
#endif
#ifdef USE_SENSOR
        esphome::sensor::Sensor *connection_count_sensor_;
#endif

        std::unique_ptr<uint8_t []> buf_{};
        size_t buf_head_{0};
        size_t buf_tail_{0};

        std::unique_ptr<esphome::socket::Socket> socket_{};
        std::vector<Client> clients_{};
};

}  // namespace i2c_sniffer
}  // namespace esphome
