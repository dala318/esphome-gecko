#include "i2c_sniffer.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/util.h"
#include "esphome/core/version.h"

#include "esphome/components/network/util.h"
#include "esphome/components/socket/socket.h"

namespace esphome {
namespace i2c_sniffer {

static const char *TAG = "i2c_sniffer";

void I2CSnifferComponent::setup() {
    ESP_LOGCONFIG(TAG, "Setting up stream server...");

    // The make_unique() wrapper doesn't like arrays, so initialize the unique_ptr directly.
    this->buf_ = std::unique_ptr<uint8_t[]>{new uint8_t[this->buf_size_]};

    struct sockaddr_storage bind_addr;
#if ESPHOME_VERSION_CODE >= VERSION_CODE(2023, 4, 0)
    socklen_t bind_addrlen = socket::set_sockaddr_any(reinterpret_cast<struct sockaddr *>(&bind_addr), sizeof(bind_addr), this->port_);
#else
    socklen_t bind_addrlen = socket::set_sockaddr_any(reinterpret_cast<struct sockaddr *>(&bind_addr), sizeof(bind_addr), htons(this->port_));
#endif

    this->socket_ = socket::socket_ip(SOCK_STREAM, PF_INET);
    this->socket_->setblocking(false);
    this->socket_->bind(reinterpret_cast<struct sockaddr *>(&bind_addr), bind_addrlen);
    this->socket_->listen(8);

    // sniffer specific
    this->reset_i2c_variables();
    this->scl_pin_->attach_interrupt(I2CSnifferComponent::i2c_trigger_on_raising_scl, this, gpio::INTERRUPT_RISING_EDGE);
    this->sda_pin_->attach_interrupt(I2CSnifferComponent::i2c_trigger_on_change_sda, this, gpio::INTERRUPT_ANY_EDGE);

    this->publish_sensor();
}

void I2CSnifferComponent::loop()
{
    this->accept();
    this->read();
    this->flush();
    this->empty_sockets();
    // this->write();
    this->cleanup();
}

void I2CSnifferComponent::dump_config()
{
    ESP_LOGCONFIG(TAG, "Stream Server:");
    ESP_LOGCONFIG(TAG, "  Address: %s:%u", esphome::network::get_use_address().c_str(), this->port_);
#ifdef USE_BINARY_SENSOR
    LOG_BINARY_SENSOR("  ", "Connected:", this->connected_sensor_);
#endif
#ifdef USE_SENSOR
    LOG_SENSOR("  ", "Connection count:", this->connection_count_sensor_);
#endif
}

void I2CSnifferComponent::on_shutdown()
{
    for (const Client &client : this->clients_)
        client.socket->shutdown(SHUT_RDWR);
}

void I2CSnifferComponent::publish_sensor()
{
#ifdef USE_BINARY_SENSOR
    if (this->connected_sensor_)
        this->connected_sensor_->publish_state(this->clients_.size() > 0);
#endif
#ifdef USE_SENSOR
    if (this->connection_count_sensor_)
        this->connection_count_sensor_->publish_state(this->clients_.size());
#endif
}

void I2CSnifferComponent::accept()
{
    struct sockaddr_storage client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    std::unique_ptr<socket::Socket> socket = this->socket_->accept(reinterpret_cast<struct sockaddr *>(&client_addr), &client_addrlen);
    if (!socket)
        return;

    socket->setblocking(false);
    std::string identifier = socket->getpeername();
    this->clients_.emplace_back(std::move(socket), identifier, this->buf_head_);
    ESP_LOGD(TAG, "New client connected from %s", identifier.c_str());
    this->publish_sensor();
}

void I2CSnifferComponent::cleanup()
{
    auto discriminator = [](const Client &client) { return !client.disconnected; };
    auto last_client = std::partition(this->clients_.begin(), this->clients_.end(), discriminator);
    if (last_client != this->clients_.end()) {
        this->clients_.erase(last_client, this->clients_.end());
        this->publish_sensor();
    }
}

void I2CSnifferComponent::read()
{
    if(this->i2c_status_ == I2C_IDLE)
    {
        if(this->buffer_poi_w_ == this->buffer_poi_r_)  //There is nothing to say
            return;

        uint16_t pw = this->buffer_poi_w_;
        uint8_t read_data[9600];
        uint16_t read_data_bytes = 0;
        std::string read_data_str = "";
        ESP_LOGD(TAG, "\nSCL up: %d SDA up: %d SDA down: %d false start: %d\n", this->scl_up_cnt_, this->sda_up_cnt_, this->sda_down_cnt_, this->false_start_cnt_);
        while(this->buffer_poi_r_ < pw)
        {
            read_data[read_data_bytes] = this->data_buffer_[this->buffer_poi_r_];
            read_data_str += this->data_buffer_[this->buffer_poi_r_];
            this->buffer_poi_r_++;
            read_data_bytes++;
        }

        //if there is no I2C action in progress and there wasn't during the Serial.print then buffer was printed out completly and can be reset.
        if(this->i2c_status_ == I2C_IDLE && pw == this->buffer_poi_w_)
        {
            this->buffer_poi_w_ = 0;
            this->buffer_poi_r_ = 0;
        }	

        if (read_data_bytes > 0)
        {
            ESP_LOGD(TAG, "Read %d bytes data", read_data_bytes);
            ESP_LOGD(TAG, ">> %s", read_data_str);

            size_t len = 0;
            size_t read_from = 0;
            int available;
            // while ((available = this->stream_->available()) > 0)
            while ((available = read_data_bytes - read_from) > 0)
            {
                size_t free = this->buf_size_ - (this->buf_head_ - this->buf_tail_);
                if (free == 0) {
                    // Only overwrite if nothing has been added yet, otherwise give flush() a chance to empty the buffer first.
                    if (len > 0)
                        return;
                    ESP_LOGE(TAG, "Incoming bytes available, but outgoing buffer is full: stream will be corrupted!");
                    free = std::min<size_t>(available, this->buf_size_);
                    this->buf_tail_ += free;
                    for (Client &client : this->clients_) {
                        if (client.position < this->buf_tail_) {
                            ESP_LOGW(TAG, "Dropped %u pending bytes for client %s", this->buf_tail_ - client.position, client.identifier.c_str());
                            client.position = this->buf_tail_;
                        }
                    }
                }
                // Fill all available contiguous space in the ring buffer.
                len = std::min<size_t>(available, std::min<size_t>(this->buf_ahead(this->buf_head_), free));
                for(int i = 0; i < len; i++)
                {
                    this->buf_[this->buf_index(this->buf_head_) + i] = read_data[read_from++];
                }
                this->buf_head_ += len;
            }
        }        
    }
}

void I2CSnifferComponent::flush() {
    ssize_t written;
    this->buf_tail_ = this->buf_head_;
    for (Client &client : this->clients_) {
        if (client.disconnected || client.position == this->buf_head_)
            continue;

        // Split the write into two parts: from the current position to the end of the ring buffer, and from the start
        // of the ring buffer until the head. The second part might be zero if no wraparound is necessary.
        struct iovec iov[2];
        iov[0].iov_base = &this->buf_[this->buf_index(client.position)];
        iov[0].iov_len = std::min(this->buf_head_ - client.position, this->buf_ahead(client.position));
        iov[1].iov_base = &this->buf_[0];
        iov[1].iov_len = this->buf_head_ - (client.position + iov[0].iov_len);
        if ((written = client.socket->writev(iov, 2)) > 0) {
            client.position += written;
        } else if (written == 0 || errno == ECONNRESET) {
            ESP_LOGD(TAG, "Client %s disconnected", client.identifier.c_str());
            client.disconnected = true;
            continue;  // don't consider this client when calculating the tail position
        } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // Expected if the (TCP) transmit buffer is full, nothing to do.
        } else {
            ESP_LOGE(TAG, "Failed to write to client %s with error %d!", client.identifier.c_str(), errno);
        }

        this->buf_tail_ = std::min(this->buf_tail_, client.position);
    }
}

void I2CSnifferComponent::empty_sockets() {
    uint8_t buf[128];
    ssize_t read;
    for (Client &client : this->clients_) {
        if (client.disconnected)
            continue;

        // Just empty the incomming buffer in case data was sent to ESP
        while ((read = client.socket->read(&buf, sizeof(buf))) > 0)
        {
            // TODO: Remove, using this for now to try to fake data from i2c
            ESP_LOGD(TAG, "Faking data");
            this->data_buffer_[this->buffer_poi_w_++] = '0';
            this->data_buffer_[this->buffer_poi_w_++] = '1';
            this->data_buffer_[this->buffer_poi_w_++] = '2';
            this->data_buffer_[this->buffer_poi_w_++] = '3';
            this->data_buffer_[this->buffer_poi_w_++] = '4';
            this->data_buffer_[this->buffer_poi_w_++] = '5';
            this->data_buffer_[this->buffer_poi_w_++] = '6';
            this->data_buffer_[this->buffer_poi_w_++] = '7';
            this->data_buffer_[this->buffer_poi_w_++] = '8';
        }

        if (read == 0 || errno == ECONNRESET) {
            ESP_LOGD(TAG, "Client %s disconnected", client.identifier.c_str());
            client.disconnected = true;
        } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // Expected if the (TCP) receive buffer is empty, nothing to do.
        } else {
            ESP_LOGW(TAG, "Failed to read from client %s with error %d!", client.identifier.c_str(), errno);
        }
    }
}

void IRAM_ATTR I2CSnifferComponent::i2c_trigger_on_raising_scl(I2CSnifferComponent *sniffer)
{
    sniffer->scl_up_cnt_++;

    //is it a false trigger?
    if(sniffer->i2c_status_ == I2C_IDLE)
    {
        sniffer->false_start_cnt_++;
        //return;  //this is not clear why do we have so many false START
    }


    //get the value from SDA
    uint8_t i2c_bit_c =  sniffer->sda_pin_->digital_read();

    //decide wherewe are and what to do with incoming data
    uint8_t i2c_case = 0;  //normal case

    if(sniffer->bit_count_ == 8)  //ACK case
        i2c_case = 1;

    // R/W if the first address byte
    if(sniffer->bit_count_ == 7 && sniffer->byte_count_ == 0 )
        i2c_case = 2;

    sniffer->bit_count_++;

    switch (i2c_case)
    {
        case 0:  // Normal case
            sniffer->data_buffer_[sniffer->buffer_poi_w_++] = '0' + i2c_bit_c;  //48
            break;
        case 1:  // ACK
            if(i2c_bit_c)  // 1 NACK SDA HIGH
            {
                sniffer->data_buffer_[sniffer->buffer_poi_w_++] = '-';  //45
            }
            else  // 0 ACK SDA LOW
            {
                sniffer->data_buffer_[sniffer->buffer_poi_w_++] = '+';  //43
            }	
            sniffer->byte_count_++;
            sniffer->bit_count_=0;
            break;
        case 2:
            if(i2c_bit_c)
            {
                sniffer->data_buffer_[sniffer->buffer_poi_w_++] = 'R';  //82
            }
            else
            {
                sniffer->data_buffer_[sniffer->buffer_poi_w_++] = 'W';  //87
            }
        break;
    }
}

void IRAM_ATTR I2CSnifferComponent::i2c_trigger_on_change_sda(I2CSnifferComponent *sniffer)
{
    uint8_t i2c_bit_d = 0;
    uint8_t i2c_bit_d2 = 1;
    // Make sure that the SDA is in stable state
    do
    {
        i2c_bit_d = sniffer->sda_pin_->digital_read();
        i2c_bit_d2 = sniffer->sda_pin_->digital_read();
    } while (i2c_bit_d != i2c_bit_d2);

    if(i2c_bit_d)  //RISING if SDA is HIGH (1)
    {
        uint8_t i2c_clk = sniffer->scl_pin_->digital_read();
        if(sniffer->i2c_status_ =! I2C_IDLE && i2c_clk == 1)  //If SCL still HIGH then it is a STOP sign
        {			
            //sniffer->i2c_status_ = I2C_STOP;
            sniffer->i2c_status_ = I2C_IDLE;
            sniffer->bit_count_ = 0;
            sniffer->byte_count_ = 0;
            sniffer->buffer_poi_w_--;
            sniffer->data_buffer_[sniffer->buffer_poi_w_++] = 's';  //115
            sniffer->data_buffer_[sniffer->buffer_poi_w_++] = '\n'; //10
        }
        sniffer->sda_up_cnt_++;
    }
    else  //FALLING if SDA is LOW
    {
        
        uint8_t i2c_clk = sniffer->scl_pin_->digital_read();
        if(sniffer->i2c_status_ == I2C_IDLE && i2c_clk)  //If SCL still HIGH than this is a START
        {
        sniffer->i2c_status_ = I2C_TRX;
        //sniffer->last_start_millis_ = millis();  // Takes too long in an interrupt handler and caused timeout panic and CPU restart
        sniffer->bit_count_ = 0;
        sniffer->byte_count_ = 0;
        sniffer->data_buffer_[sniffer->buffer_poi_w_++] = 'S';  //83 STOP
        //sniffer->i2c_status_ = START;		
        }
        sniffer->sda_down_cnt_++;
    }
}

void I2CSnifferComponent::reset_i2c_variables()
{
    this->i2c_status_ = I2C_IDLE;
    this->buffer_poi_w_= 0;
    this->buffer_poi_r_= 0;
    this->bit_count_ = 0;
    this->false_start_cnt_ = 0;
}

I2CSnifferComponent::Client::Client(std::unique_ptr<esphome::socket::Socket> socket, std::string identifier, size_t position)
    : socket(std::move(socket)), identifier{identifier}, position{position} {}

}  // namespace i2c_sniffer
}  // namespace esphome
