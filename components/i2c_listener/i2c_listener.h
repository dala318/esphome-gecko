#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/socket/socket.h"

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

#include <Arduino.h>
#include <Wire.h>

#include <memory>
#include <string>
#include <vector>

#define I2C_IDLE 0
#define I2C_TRX 2

namespace esphome {
namespace i2c_listener {

class I2CListenerComponent : public esphome::Component, public i2c::I2CDevice {
    public:
        I2CListenerComponent() = default;
        // void set_sda_pin(esphome::InternalGPIOPin *sda_pin) { sda_pin_ = sda_pin; }
        // void set_scl_pin(esphome::InternalGPIOPin *scl_pin) { scl_pin_ = scl_pin; }
        void set_buffer_size(size_t size) { this->buf_size_ = size; }
        void set_port(uint16_t port) { this->port_ = port; }

#ifdef USE_BINARY_SENSOR
        void set_connected_sensor(esphome::binary_sensor::BinarySensor *connected) { this->connected_sensor_ = connected; }
#endif
#ifdef USE_SENSOR
        void set_connection_count_sensor(esphome::sensor::Sensor *connection_count) { this->connection_count_sensor_ = connection_count; }
#endif
#ifdef USE_TEXT_SENSOR
        void set_dump_data_sensor(esphome::text_sensor::TextSensor *dump_data) { this->dump_data_sensor_ = dump_data; }
#endif

        void setup() override;
        void loop() override;
        void dump_config() override;
        void on_shutdown() override;

        float get_setup_priority() const override { return esphome::setup_priority::AFTER_WIFI; }

    protected:
        void publish_sensor();
        static void receive_callback(int byte_count);
        void receive_handler(int byte_count);

        void accept();
        void cleanup();
        void read();
        void flush();
        void local_output();
        void empty_sockets();

        // Static pointer to the class instance
        static I2CListenerComponent *instance_;

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

#ifdef USE_BINARY_SENSOR
        esphome::binary_sensor::BinarySensor *connected_sensor_;
#endif
#ifdef USE_SENSOR
        esphome::sensor::Sensor *connection_count_sensor_;
#endif
#ifdef USE_TEXT_SENSOR
        esphome::text_sensor::TextSensor *dump_data_sensor_;
#endif

        std::unique_ptr<uint8_t []> buf_{};
        size_t buf_head_{0};
        size_t buf_tail_{0};

        std::unique_ptr<esphome::socket::Socket> socket_{};
        std::vector<Client> clients_{};


        // Listener specific
        byte data_buffer_[9600];                // Array for storing data of the I2C communication
        std::string read_str_ = "";             // String for storing data of the I2C communication
        uint16_t read_data_bytes_ = 0;          // Number of bytes read from the I2C communication
        uint16_t buffer_poi_w_ = 0;             // Points to the first empty position in the dataBufer to write
        uint16_t buffer_poi_r_ = 0;             // Points to the position where to start read from
};

#ifndef I2C_LISTENER_H
#define I2C_LISTENER_H
I2CListenerComponent *I2CListenerComponent::instance_ = nullptr;
#endif

}  // namespace i2c_listener
}  // namespace esphome
