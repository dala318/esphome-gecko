# esphome-gecko
ESPHome Geacko spa controller integration

> **NOTE**: Just a placeholer, no real implementation done!

Relative to [GeckoLib](https://github.com/gazoodle/geckolib) but instead of interfacing with an in.Touch2 WiFi addon device connecting directly to the spa main-board controller via an ESP8266 or ESP32 and likely some voltage level isolation.

For now it's an all-in-one package, but if it makes sense and interest is to widen the usage I'm upen to split it to to a domain independent interface library and an ESPHome specific usage.

## I2C sniffers (experimental)

* [GPIO pin decoder](components/i2c_sniffer/README.md), low-level pin level shange. It compiles but have not been able to test it for real yet.
* [I2C bus reader](components/i2c_listener/README.md), Wire based (passive???) listner. It does not really compile yet, may have to abandon the callback since it requires a static method and data to be fed back to the class instance.

## Code used as template and inspiration

* Used source code for [ADS1115 Sensor](https://esphome.io/components/sensor/ads1115.html) as template to create an i2c component with related sensors.
