# I2C Listener
A component to listen to I2C data from connected bus

A fusion of the following code-bases and discussions

* [ESP Home Stream Server](https://github.com/oxan/esphome-stream-server) by @oxan
* [I2C Listener](https://forum.arduino.cc/t/esp32-i2c-sniffer-issue/1135004)
* [I2C Callback](https://community.home-assistant.io/t/issues-configuring-i2c-as-slave-with-esphome-and-how-i-solved-them/774238)

# Usage

```yaml
external_components:
- source: github://dala318/esphome-gecko:
# - source:
#       type: local
#       path: my_dev_components

i2c:
  id: i2c_bus
  sda: GPIO14
  scl: GPIO15

i2c_listener:
  i2c_id: i2c_bus
  address: 0x00
  port: 6638
  buffer_size: 256
```

# Sensors

```yaml
binary_sensor:
  - platform: i2c_listener
    connected:
      name: Connected

sensor:
  - platform: i2c_listener
    connection_count:
      name: Number of connections

text_sensor:    # Not working, writing result to it causes stack overflow?
  - platform: i2c_listener
    dump_data:
      name: Last data from I2C
```