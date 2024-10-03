# I2C Listener
A component to sniff I2C data from connacted bus

A fusion of the following code-bases

* [ESP Home Stream Server](https://github.com/oxan/esphome-stream-server) by @oxan
* [I2C Listener](https://github.com/WhitehawkTailor/I2C-listener) by @WhitehawkTailor

# Usage

```yaml
external_components:
- source: github://dala318/esphome-gecko:
# - source:
#       type: local
#       path: my_dev_components

i2c_listener:
  sda: GPIO11
  scl: GPIO12
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