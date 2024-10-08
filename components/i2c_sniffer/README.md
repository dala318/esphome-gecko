# I2C Sniffer
A component to sniff I2C data from connacted bus

A fusion of the following code-bases

* [ESP Home Stream Server](https://github.com/oxan/esphome-stream-server) by @oxan
* [I2C Sniffer](https://github.com/WhitehawkTailor/I2C-sniffer) by @WhitehawkTailor

# Usage

```yaml
external_components:
- source: github://dala318/esphome-gecko:
# - source:
#       type: local
#       path: my_dev_components

i2c_sniffer:
  sda: GPIO11
  scl: GPIO12
  port: 6638
  buffer_size: 256
```

# Sensors

```yaml
binary_sensor:
  - platform: i2c_sniffer
    connected:
      name: Connected

sensor:
  - platform: i2c_sniffer
    connection_count:
      name: Number of connections
```