/*
The MIT License (MIT)

Copyright (c) 2013 Mathias Munk Hansen

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef DS18B20_H
#define DS18B20_H

#include <OneWire.h>

// Commands.
#define SEARCH_ROM        0xF0
#define READ_ROM          0x33 // Currently not used.
#define MATCH_ROM         0x55 // Currently not used.
#define SKIP_ROM          0xCC // Currently not used.
#define ALARM_SEARCH      0xEC
#define CONVERT_T         0x44
#define WRITE_SCRATCHPAD  0x4E
#define READ_SCRATCHPAD   0xBE
#define COPY_SCRATCHPAD   0x48
#define RECALL            0xB8 // Currently not used.
#define READ_POWER_SUPPLY 0xB4

// Family codes.
#define MODEL_DS18S20 0x10 // Currently not supported.
#define MODEL_DS1822  0x22 // Currently not supported.
#define MODEL_DS18B20 0x28

// Size of the scratchpad in bytes.
#define SIZE_SCRATCHPAD 9

// Scratchpad locations. Bytes 5 through 7 are reserved.
#define TEMP_LSB      0
#define TEMP_MSB      1
#define ALARM_HIGH    2
#define ALARM_LOW     3
#define CONFIGURATION 4
#define CRC           8

// Resolution values.
#define RES_9_BIT  0x1F
#define RES_10_BIT 0x3F
#define RES_11_BIT 0x5F
#define RES_12_BIT 0x7F

// Rounded up worst-case conversion times in milliseconds for different resolutions.
#define CONV_TIME_9_BIT  94
#define CONV_TIME_10_BIT 188
#define CONV_TIME_11_BIT 375
#define CONV_TIME_12_BIT 750

class DS18B20
{
	public:
		DS18B20(uint8_t pin);

		// Gets the address of the next device.
		uint8_t getNextDevice(uint8_t address[]);

		// Resets the search so that the next search will return the first device again.
		void resetSearch();

		// Tells every device to start a temperature conversion.
		void startConversion();

		// Returns the current temperature in degrees Celcius.
		float getTempC(uint8_t address[]);

		// Returns the current temperature in degrees Fahrenheit.
		float getTempF(uint8_t address[]);

		// Returns the resolution of a device.
		uint8_t getResolution(uint8_t address[]);

		// Sets the resolution of every device.
		void setResolution(uint8_t resolution);

		// Sets the resolution of a device.
		void setResolution(uint8_t resolution, uint8_t address[]);

		// Returns the total number of devices on the wire.
		uint8_t getNumberOfDevices();

		// Returns the family code of a device.
		uint8_t getFamilyCode(uint8_t address[]);

		// Returns the power mode of a device. 1 = parasite, 0 = external.
		uint8_t isParasite(uint8_t address[]);

		// Gets the address of the next active alarm.
		uint8_t getNextAlarm(uint8_t address[]);

		// Sets both high and low alarms.
		void setAlarms(uint8_t alarmLow, uint8_t alarmHigh, uint8_t address[]);

		// Returns the value of the low alarm.
		uint8_t getAlarmLow(uint8_t address[]);

		// Sets the low alarm.
		void setAlarmLow(uint8_t alarmLow, uint8_t address[]);

		// Returns the value of the high alarm.
		uint8_t getAlarmHigh(uint8_t address[]);

		// Sets the high alarm.
		void setAlarmHigh(uint8_t alarmHigh, uint8_t address[]);


		///////////////////////////////////////////////////////////////////////////
		// Alias functions for using alarm registers as general purpose storage. //
		//             Should not be used in conjunction with alarms.            //
		///////////////////////////////////////////////////////////////////////////

		// Same as setAlarms.
		void setRegisters(uint8_t firstValue, uint8_t secondValue, uint8_t address[]) { setAlarms(firstValue, secondValue, address); }

		// Same as getAlarmLow.
		uint8_t getFirstRegister(uint8_t address[]) { return getAlarmLow(address); }

		// Same as setAlarmLow.
		void setFirstRegister(uint8_t value, uint8_t address[]) { setAlarmLow(value, address); }

		// Same as getAlarmHigh.
		uint8_t getSecondRegister(uint8_t address[]) { return getAlarmHigh(address); }

		// Same as setAlarmHigh.
		void setSecondRegister(uint8_t value, uint8_t address[]) { setAlarmHigh(value, address); }

	private:
		// OneWire object needed to communicate with 1-Wire devices.
		OneWire oneWire;

		// Scratchpad buffer.
		uint8_t scratchpad[SIZE_SCRATCHPAD];

		// Highest resolution of any device on the wire.
		uint8_t maxResolution;

		// 1 = At least one device on the wire is running in parasitic power mode.
		// 0 = All devices on the wire are running on external power. 
		uint8_t globalParasite;

		// The total number of devices on the wire.
		uint8_t devices;

		// Previous and current registration number. Used in search.
		uint8_t registrationNumber[8];

		// The bit position in the registration number where the last discrepancy was.
		uint8_t lastDiscrepancy;

		// Indicates whether or not a search is completed (i.e. last device has been found).
		uint8_t lastDevice;

		// Performs either a SEARCH_ROM or ALARM_SEARCH command.
		// Returns 1 or 0 indicating whether the search was successful or not.
		uint8_t search(uint8_t command, uint8_t address[]);

		// Reads the scratchpad of a device.
		void readScratchpad(uint8_t address[]);

		// Writes the scratchpad of a device into its EEPROM.
		void writeScratchpad(uint8_t address[]);

		// Delays for the amount of time required to perform a temperature conversion at the specified resolution.
		void delayForConversion(uint8_t resolution, uint8_t parasite);

		// Sends a command to all devices, with or without parasitic power at the end.
		void sendCommand(uint8_t command, uint8_t parasite = 0);

		// Sends a command to a device, with or without parasitic power at the end.
		void sendCommand(uint8_t command, uint8_t address[], uint8_t parasite = 0);
};

#endif
