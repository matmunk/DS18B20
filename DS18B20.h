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

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <OneWire.h>

// Commands.
#define SEARCH_ROM        0xF0
#define READ_ROM          0x33
#define MATCH_ROM         0x55
#define SKIP_ROM          0xCC
#define ALARM_SEARCH      0xEC
#define CONVERT_T         0x44
#define WRITE_SCRATCHPAD  0x4E
#define READ_SCRATCHPAD   0xBE
#define COPY_SCRATCHPAD   0x48
#define RECALL            0xB8
#define READ_POWER_SUPPLY 0xB4

// Family codes.
#define MODEL_DS18S20 0x10
#define MODEL_DS1820  0x22
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

// Rounded up worst-case conversion times in milliseconds at different resolutions.
#define CONV_TIME_9_BIT  94
#define CONV_TIME_10_BIT 188
#define CONV_TIME_11_BIT 375
#define CONV_TIME_12_BIT 750

class DS18B20
{
	public:
		DS18B20(uint8_t pin);

		// Selects the device with the specified address if it is present.
		uint8_t select(uint8_t address[]);

		// Selects the next device.
		uint8_t selectNext();

		// Selects the next device with an active alarm condition.
		uint8_t selectNextAlarm();

		// Resets the search so that the next search will return the first device again.
		void resetSearch();

		// Returns the current temperature in degrees Celcius.
		float getTempC();

		// Returns the current temperature in degrees Fahrenheit.
		float getTempF();

		// Returns the resolution of the selected device.
		uint8_t getResolution();

		// Sets the resolution of the selected device.
		void setResolution(uint8_t resolution);

		// Returns the power mode of the selected device.
		uint8_t getPowerMode();

		// Returns the family code of the selected device.
		uint8_t getFamilyCode();

		// Copies the address of the selected device into the supplied array.
		void getAddress(uint8_t address[]);

		// Tells every device on the bus to start a temperature conversion and delays until it is completed.
		void doConversion();

		// Returns the number of devices present on the bus.
		uint8_t getNumberOfDevices();

		// Checks if the selected device has an active alarm condition.
		uint8_t hasAlarm();

		// Sets both alarms of the selected device.
		void setAlarms(int8_t alarmLow, int8_t alarmHigh);

		// Returns the low alarm value of the selected device.
		int8_t getAlarmLow();

		// Sets the low alarm value of the selected device.
		void setAlarmLow(int8_t alarmLow);

		// Returns the high alarm value of the selected device.
		int8_t getAlarmHigh();

		// Sets the high alarm value of the selected device.
		void setAlarmHigh(int8_t alarmHigh);

		// Sets both registers of the selected device.
		void setRegisters(int8_t lowRegister, int8_t highRegister);

		// Returns the low register value of the selected device.
		int8_t getLowRegister();

		// Sets the low register value of the selected device.
		void setLowRegister(int8_t lowRegister);

		// Returns the high register value of the selected device.
		int8_t getHighRegister();

		// Sets the high register value of the selected device.
		void setHighRegister(int8_t highRegister);

	private:
		// OneWire object needed to communicate with 1-Wire devices.
		OneWire oneWire;

		// The highest resolution of any device on the bus.
		uint8_t globalResolution;

		// Whether every single device on the bus is being powered externally.
		uint8_t globalPowerMode;

		// The number of devices on the bus.
		uint8_t numberOfDevices;

		// 64 bit address of the selected device.
		uint8_t selectedAddress[8];

		// Scratchpad of the selected device.
		uint8_t selectedScratchpad[SIZE_SCRATCHPAD];

		// Resolution of the selected device.
		uint8_t selectedResolution;

		// Power mode of the selected device.
		uint8_t selectedPowerMode;

		// Most recent address discovered by searching.
		uint8_t searchAddress[8];

		// The bit position where the last discrepancy was.
		uint8_t lastDiscrepancy;

		// Indicates whether or not a search is completed (i.e. last device has been found).
		uint8_t lastDevice;

		// Reads the scratchpad of the selected device.
		uint8_t readScratchpad();

		// Writes the scratchpad of the selected device into its EEPROM.
		void writeScratchpad();

		// Sends a rom command to the selected device.
		uint8_t sendCommand(uint8_t romCommand);

		// Sends a rom command followed by a function command to the selected device.
		uint8_t sendCommand(uint8_t romCommand, uint8_t functionCommand, uint8_t power = 0);

		// Performs a 1-Wire search, either normal or conditional.
		uint8_t oneWireSearch(uint8_t romCommand);

		// Checks if the specified device is present on the bus.
		uint8_t isConnected(uint8_t address[]);

		// Delays for the amount of time required to perform a temperature conversion.
		void delayForConversion(uint8_t resolution, uint8_t powerMode);
};

#endif
