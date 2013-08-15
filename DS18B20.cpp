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

#include <DS18B20.h>

DS18B20::DS18B20(uint8_t pin) : oneWire(OneWire(pin))
{
	// Initialize to 9 bit, which is the lowest possible resolution.
	maxResolution = 9;

	// Start by assuming that all devices are running on external power.
	globalParasite = 0;

	devices = 0;

	// Reset search in order to initialize search variables.
	resetSearch();

	uint8_t address[8];

	// Determine highest resolution of any device and whether any device is running in parasitic power mode.
	while(getNextDevice(address))
	{
		uint8_t resolution = getResolution(address);

		if(resolution > maxResolution)
		{
			maxResolution = resolution;
		}

		if(isParasite(address))
		{
			globalParasite = 1;
		}

		// Count the number of devices.
		devices++;
	}
}

// Resets the search so that the next search will return the first device again.
void DS18B20::resetSearch()
{
	lastDiscrepancy = 0;
	lastDevice = 0;
}

// Tells every device to start a temperature conversion and delays until it is completed.
void DS18B20::doConversion()
{
	sendCommand(CONVERT_T, globalParasite);

	delayForConversion(maxResolution, globalParasite);
}

// Tells a device to start a temperature conversion and delays until it is completed.
void DS18B20::doConversion(uint8_t address[])
{
	uint8_t resolution = getResolution(address);
	uint8_t parasite = isParasite(address);

	sendCommand(CONVERT_T, address, parasite);

	delayForConversion(resolution, parasite);
}

// Returns the current temperature in degrees Celcius.
float DS18B20::getTempC(uint8_t address[])
{
	doConversion(address);

	readScratchpad(address);

	uint8_t lsb = scratchpad[TEMP_LSB];
	uint8_t msb = scratchpad[TEMP_MSB];

	uint8_t resolution = getResolution(address);

	// Trim low-order byte according to resolution.
	switch(resolution)
	{
		case 9:
			lsb &= 0xF8; // Remove bits 0, 1 and 2.
			break;
		case 10:
			lsb &= 0xFC; // Remove bits 0 and 1.
			break;
		case 11:
			lsb &= 0xFE; // Remove bit 0.
			break;
	}

	// High-order bit denotes sign. 1 = negative, 0 = positive.
	uint8_t sign = msb & 0x80;

	// Combine high-order byte and low-order byte.
	int16_t temp = (msb << 8) + lsb;

	if(sign)
	{
		// Convert from two's complement.
		temp = ((temp ^ 0xffff) + 1) * -1;
	}

	return temp / 16.0;
}

// Returns the resolution of a device.
uint8_t DS18B20::getResolution(uint8_t address[])
{
	// Get resolution from device.
	readScratchpad(address);

	// Extract resolution from scratchpad buffer and return it.
	switch(scratchpad[CONFIGURATION])
	{
		case RES_9_BIT:
			return 9;
		case RES_10_BIT:
			return 10;
		case RES_11_BIT:
			return 11;
		case RES_12_BIT:
			return 12;
	}
}

// Sets the resolution of every device.
void DS18B20::setResolution(uint8_t resolution)
{
	uint8_t address[8];

	// Iterate through all devices and set their resolution.
	while(getNextDevice(address))
	{
		setResolution(resolution, address);
	}
}

// Sets the resolution of a device.
void DS18B20::setResolution(uint8_t resolution, uint8_t address[])
{
	// Get alarm values from device and store them in scratchpad buffer (this avoids overwriting them).
	readScratchpad(address);

	// Make sure that resolution is in range [9;12].
	resolution = constrain(resolution, 9, 12);

	// Set new resolution in scratchpad buffer.
	switch(resolution)
	{
		case 9:
			scratchpad[CONFIGURATION] = RES_9_BIT;
			break;
		case 10:
			scratchpad[CONFIGURATION] = RES_10_BIT;
			break;
		case 11:
			scratchpad[CONFIGURATION] = RES_11_BIT;
			break;
		case 12:
			scratchpad[CONFIGURATION] = RES_12_BIT;
			break;
	}

	// Write scratchpad buffer to EEPROM.
	writeScratchpad(address);

	// Update max resolution if necessary.
	if(resolution > maxResolution)
	{
		maxResolution = resolution;
	}
}

// Returns the power mode of a device. 1 = parasite, 0 = external.
uint8_t DS18B20::isParasite(uint8_t address[])
{
	sendCommand(READ_POWER_SUPPLY, address);

	// Read bit. 0 = parasite, 1 = external.
	return !oneWire.read_bit();
}

uint8_t DS18B20::hasAlarm(uint8_t address[])
{
	// Save old resolution.
	uint8_t oldResolution = getResolution(address);

	// Set resolution to 9 bits since we wont use the fractional part in the comparison anyway.
	setResolution(9, address);

	float temp = getTempC(address);

	if(temp <= scratchpad[ALARM_LOW] || temp >= scratchpad[ALARM_HIGH])
	{
		return 1;
	}

	return 0;
}

// Sets both high and low alarms.
void DS18B20::setAlarms(uint8_t alarmLow, uint8_t alarmHigh, uint8_t address[])
{
	readScratchpad(address);

	scratchpad[ALARM_LOW] = alarmLow;
	scratchpad[ALARM_HIGH] = alarmHigh;

	writeScratchpad(address);
}

// Returns the value of the low alarm.
uint8_t DS18B20::getAlarmLow(uint8_t address[]) {
	readScratchpad(address);

	return scratchpad[ALARM_LOW];
}

// Sets the low alarm.
void DS18B20::setAlarmLow(uint8_t alarmLow, uint8_t address[])
{
	readScratchpad(address);

	scratchpad[ALARM_LOW] = alarmLow;

	writeScratchpad(address);
}

// Returns the value of the high alarm.
uint8_t DS18B20::getAlarmHigh(uint8_t address[])
{
	readScratchpad(address);

	return scratchpad[ALARM_HIGH];
}

// Sets the high alarm.
void DS18B20::setAlarmHigh(uint8_t alarmHigh, uint8_t address[])
{
	readScratchpad(address);

	scratchpad[ALARM_HIGH] = alarmHigh;

	writeScratchpad(address);
}

// Performs either a SEARCH_ROM or ALARM_SEARCH command.
// Returns 1 or 0 indicating whether the search was successful or not.
uint8_t DS18B20::search(uint8_t command, uint8_t address[])
{
	// Search is completed. Reset flag and return.
	if(lastDevice)
	{
		resetSearch();
		return 0;
	}

	// No presence pulse(s).
	if(!oneWire.reset())
	{
		resetSearch();
		return 0;
	}

	uint8_t lastZero = 0;
	uint8_t direction;

	// Send command.
	oneWire.write(command);

	// Iterate through bits 0 to 63.
	for(uint8_t bitPosition = 0; bitPosition < 64; bitPosition++)
	{
		// Current byte of the registration number and current bit number within that byte.
		uint8_t byteNumber = bitPosition / 8;
		uint8_t bitNumber = bitPosition % 8;

		// Current bit and its complement.
		uint8_t currentBit = oneWire.read_bit();
		uint8_t currentBitComp = oneWire.read_bit();

		// Happens if the device being discovered is disconnected, becomes faulty etc. during search.
		if(currentBit && currentBitComp)
		{
			lastDiscrepancy = 0;
			return 0;
		}

		// Discrepancy here. Both 0s and 1s at the current bit position.
		if(!currentBit && !currentBitComp)
		{
			if(bitPosition == lastDiscrepancy)
			{
				direction = 1;
			}
			else if(bitPosition > lastDiscrepancy)
			{
				direction = 0;
				lastZero = bitPosition;
			}
			else
			{
				direction = bitRead(registrationNumber[byteNumber], bitNumber);

				if(!direction)
				{
					lastZero = bitPosition;
				}
			}
		}
		else // Simple case. All devices have either exclusively 0s or 1s at the current bit position.
		{
			direction = currentBit;
		}

		// Save current direction.
		bitWrite(registrationNumber[byteNumber], bitNumber, direction);

		// Deselect devices whose registration numbers do not match current direction.
		oneWire.write_bit(direction);
	}

	lastDiscrepancy = lastZero;

	if(!lastDiscrepancy)
	{
		lastDevice = 1;
	}

	// Copy registration number to address array.
	memcpy(address, registrationNumber, 8);

	return 1;
}

// Reads the scratchpad of a device.
void DS18B20::readScratchpad(uint8_t address[])
{
	// Start read sequence.
	sendCommand(READ_SCRATCHPAD, address);

	// Read entire scratchpad of the device.
	for(uint8_t i = 0; i < SIZE_SCRATCHPAD; i++)
	{
		scratchpad[i] = oneWire.read();
	}
}

// Writes the scratchpad of a device into its EEPROM.
void DS18B20::writeScratchpad(uint8_t address[])
{
	// Get power mode of the device.
	uint8_t parasite = isParasite(address);

	// Start write sequence.
	sendCommand(WRITE_SCRATCHPAD, address);

	// Write scratchpad buffer to the device.
	oneWire.write(scratchpad[ALARM_HIGH]);
	oneWire.write(scratchpad[ALARM_LOW]);
	oneWire.write(scratchpad[CONFIGURATION]);

	// Write scratchpad to EEPROM.
	sendCommand(COPY_SCRATCHPAD, address, parasite);

	// Delay for 10 ms if the device is running in parasitic power mode according to datasheet.
	if(parasite)
	{
		delay(10);
	}
}

// Delays for the amount of time required to perform a temperature conversion at the specified resolution.
void DS18B20::delayForConversion(uint8_t resolution, uint8_t parasite)
{
	// Make sure that resolution is in range [9;12].
	resolution = constrain(resolution, 9, 12);

	if(parasite)
	{
		// Delay for the appropriate amount of time depending on the resolution.
		switch(resolution)
		{
			case 9:
				delay(CONV_TIME_9_BIT);
				break;
			case 10:
				delay(CONV_TIME_10_BIT);
				break;
			case 11:
				delay(CONV_TIME_11_BIT);
				break;
			case 12:
				delay(CONV_TIME_12_BIT);
				break;
		}
	}
	else
	{
		// Poll sensor until temperature conversion is complete.
		while(!oneWire.read_bit());
	}
}

// Sends a command to all devices, with or without parasitic power at the end.
void DS18B20::sendCommand(uint8_t command, uint8_t parasite)
{
	// Send reset pulse.
	if(!oneWire.reset())
	{
		// No presence pulse(s). Maybe do some error handling.
	}

	// Select all devices.
	oneWire.skip();

	// Send command.
	oneWire.write(command, parasite);
}

// Sends a command to a device, with or without parasitic power at the end.
void DS18B20::sendCommand(uint8_t command, uint8_t address[], uint8_t parasite)
{
	// Send reset pulse.
	if(!oneWire.reset())
	{
		// No presence pulse(s). Maybe do some error handling.
	}

	// Select device with corresponding address.
	oneWire.select(address);

	// Send command.
	oneWire.write(command, parasite);
}
