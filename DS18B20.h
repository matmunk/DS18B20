#ifndef DS18B20_H
#define DS18B20_H

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
#define MODEL_DS18S20 0x10 // Currently not supported.
#define MODEL_DS1822  0x22 // Currently not supported.
#define MODEL_DS18B20 0x28

// Size of the scratchpad in bytes.
#define SIZE_SCRATCHPAD 9

// Scratchpad locations.
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

		// Returns the family code of a device.
		uint8_t getFamilyCode(uint8_t address[]);

		// Returns the power mode of a device.
		// TRUE = parasite, FALSE = external.
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

	private:
		// OneWire object needed to communicate with 1-Wire devices.
		OneWire oneWire;

		// Global search state.
		uint8_t romBit[64];
		uint8_t lastDiscrepancy;
		uint8_t searchDone;

		// Performs either a SEARCH_ROM or ALARM_SEARCH command.
		// Returns TRUE/FALSE indicating whether the search was successful.
		uint8_t search(uint8_t command, uint8_t address[]);

		// Reads the scratchpad of a device.
		void readScratchpad(uint8_t scratchpad[], uint8_t address[]);

		// Writes an array of data into the EEPROM of a device.
		void writeScratchpad(uint8_t scratchpad[], uint8_t address[]);

		// Delays for the amount of time required to perform a temperature conversion at the specified resolution.
		void delayForConversion(uint8_t resolution);

		// Sends a command to all devices, with or without parasitic power at the end.
		void sendCommand(uint8_t command, uint8_t parasite = 0);

		// Sends a command to a device, with or without parasitic power at the end.
		void sendCommand(uint8_t command, uint8_t address[], uint8_t parasite = 0);
};

#endif
