# esphome-gecko
ESPHome Geacko spa controller integration

> **NOTE**: Just a placeholer, no real implementation done!

Relative to [GeckoLib](https://github.com/gazoodle/geckolib) but instead of interfacing with a in.Touch2 WiFi addon device connecting directly to the spa main-board controller via an ESP8266 or ESP32 and likely some voltage level isolation.

For now it's an all-in-one package, but if it makes sense and interest is to widen the usage I'm upen to split it to to a domain independent interface library and an ESPHome specific usage.

# Declaration of copy & modify code

* Source code for [ADS1115 Sensor](https://esphome.io/components/sensor/ads1115.html)

# Experimental I2C sniffer

[Read more here](components/i2c_sniffer/README.md) for more about it. It compiles but have not been able to test it yet.