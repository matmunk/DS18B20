# DS18B20 #

Arduino library for the Maxim Integrated DS18B20 1-Wire temperature sensor.

## Usage ##

This library uses the OneWire library, so you will need to have this installed. Get it [here](http://www.pjrc.com/teensy/td_libs_OneWire.html).

In the **OneWire.h** file set `ONEWIRE_SEARCH` to 0 since the search functionality is also implemented in this library (don't do this if you need the search functionality for other 1-Wire devices). You can also disable CRC if you don't need it. This may save some space on your Arduino.

In the **DS18B20.h** file set `ALARMS_ON` to 0 if you don't need the alarm functionality.

For general usage see the included examples.
