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
	// Reset search in order to initialize search variables.
	resetSearch();

	// Tell every device on the bus to transmit their power mode.
	// 0 = at least one device is running in parasitic power mode.
	sendCommand(SKIP_ROM, READ_POWER_SUPPLY);
	globalPowerMode = oneWire.read_bit();

	// Determine the highest resolution of any device on the bus.
	while(selectNext())
	{
		uint8_t resolution = getResolution();

		if(resolution > globalResolution)
		{
			globalResolution = resolution;
		}

		numberOfDevices++;
	}
}

// Selects the device with the specified address if it is present.
uint8_t DS18B20::select(uint8_t address[])
{
	if(isConnected(address))
	{
		memcpy(selectedAddress, address, 8);

		if(readScratchpad())
		{
			selectedResolution = getResolution();

			sendCommand(MATCH_ROM, READ_POWER_SUPPLY);
			selectedPowerMode = oneWire.read_bit();

			return 1;
		}
	}

	return 0;
}

// Selects the next device.
uint8_t DS18B20::selectNext()
{
	if(oneWireSearch(SEARCH_ROM))
	{
		return select(searchAddress);
	}

	return 0;
}

// Selects the next device with an active alarm condition.
uint8_t DS18B20::selectNextAlarm()
{
	if(oneWireSearch(ALARM_SEARCH))
	{
		return select(searchAddress);
	}

	return 0;
}

// Resets the search so that the next search will return the first device again.
void DS18B20::resetSearch()
{
	lastDiscrepancy = 0;
	lastDevice = 0;
}

// Returns the current temperature in degrees Celcius.
float DS18B20::getTempC()
{
	sendCommand(MATCH_ROM, CONVERT_T, !selectedPowerMode);
	delayForConversion(selectedResolution, selectedPowerMode);

	readScratchpad();
	uint8_t lsb = selectedScratchpad[TEMP_LSB];
	uint8_t msb = selectedScratchpad[TEMP_MSB];

	// Trim low-order byte according to resolution.
	switch(selectedResolution)
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

// Returns the current temperature in degrees Fahrenheit.
float DS18B20::getTempF()
{
	return getTempC() * 1.8 + 32;
}

// Returns the resolution of the selected device.
uint8_t DS18B20::getResolution()
{
	switch(selectedScratchpad[CONFIGURATION])
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

// Sets the resolution of the selected device.
void DS18B20::setResolution(uint8_t resolution)
{
	// The DS18B20 only supports resolutions between 9 and 12 bits.
	resolution = constrain(resolution, 9, 12);

	switch(resolution)
	{
		case 9:
			selectedScratchpad[CONFIGURATION] = RES_9_BIT;
			break;
		case 10:
			selectedScratchpad[CONFIGURATION] = RES_10_BIT;
			break;
		case 11:
			selectedScratchpad[CONFIGURATION] = RES_11_BIT;
			break;
		case 12:
			selectedScratchpad[CONFIGURATION] = RES_12_BIT;
			break;
	}

	if(resolution > globalResolution)
	{
		globalResolution = resolution;
	}

	writeScratchpad();
}

// Returns the power mode of the selected device.
uint8_t DS18B20::getPowerMode()
{
	return selectedPowerMode;
}

// Returns the family code of the selected device.
uint8_t DS18B20::getFamilyCode()
{
	return selectedAddress[0];
}

// Copies the address of the selected device into the supplied array.
void DS18B20::getAddress(uint8_t address[])
{
	memcpy(address, selectedAddress, 8);
}

// Tells every device on the bus to start a temperature conversion and delays until it is completed.
void DS18B20::doConversion()
{
	sendCommand(SKIP_ROM, CONVERT_T, !globalPowerMode);
	delayForConversion(globalResolution, globalPowerMode);
}

// Returns the number of devices present on the bus.
uint8_t DS18B20::getNumberOfDevices()
{
	return numberOfDevices;
}

// Checks if the selected device has an active alarm condition.
// The comparison is performed at 9 bits, since the alarm registers are only 8 bit anyway.
uint8_t DS18B20::hasAlarm()
{
	uint8_t oldResolution = selectedResolution;

	setResolution(9);

	float temp = getTempC();

	setResolution(oldResolution);

	return ((temp <= selectedScratchpad[ALARM_LOW]) || (temp >= selectedScratchpad[ALARM_HIGH]));
}

// Sets both alarms of the selected device.
void DS18B20::setAlarms(int8_t alarmLow, int8_t alarmHigh)
{
	setAlarmLow(alarmLow);
	setAlarmHigh(alarmHigh);

	writeScratchpad();
}

// Returns the low alarm value of the selected device.
int8_t DS18B20::getAlarmLow()
{
	return selectedScratchpad[ALARM_LOW];
}

// Sets the low alarm value of the selected device.
void DS18B20::setAlarmLow(int8_t alarmLow)
{
	// Constrain temperature to fit the temperature range of the DS18B20.
	alarmLow = constrain(alarmLow, -55, 125);

	selectedScratchpad[ALARM_LOW] = alarmLow;

	writeScratchpad();
}

// Returns the high alarm value of the selected device.
int8_t DS18B20::getAlarmHigh()
{
	return selectedScratchpad[ALARM_HIGH];
}

// Sets the high alarm value of the selected device.
void DS18B20::setAlarmHigh(int8_t alarmHigh)
{
	// Constrain temperature to fit the temperature range of the DS18B20.
	alarmHigh = constrain(alarmHigh, -55, 125);

	selectedScratchpad[ALARM_HIGH] = alarmHigh;

	writeScratchpad();
}

// Sets both registers of the selected device.
void DS18B20::setRegisters(int8_t lowRegister, int8_t highRegister)
{
	setAlarms(lowRegister, highRegister);
}

// Returns the low register value of the selected device.
int8_t DS18B20::getLowRegister()
{
	return getAlarmLow();
}

// Sets the low register value of the selected device.
void DS18B20::setLowRegister(int8_t lowRegister)
{
	setAlarmLow(lowRegister);
}

// Returns the high register value of the selected device.
int8_t DS18B20::getHighRegister()
{
	return getAlarmHigh();
}

// Sets the high register value of the selected device.
void DS18B20::setHighRegister(int8_t highRegister)
{
	setAlarmHigh(highRegister);
}

// Reads the scratchpad of the selected device.
uint8_t DS18B20::readScratchpad()
{
	sendCommand(MATCH_ROM, READ_SCRATCHPAD);

	for(uint8_t i = 0; i < SIZE_SCRATCHPAD; i++)
	{
		selectedScratchpad[i] = oneWire.read();
	}

	return OneWire::crc8(selectedScratchpad, 8) == selectedScratchpad[CRC];
}

// Writes the scratchpad of the selected device into its EEPROM.
void DS18B20::writeScratchpad()
{
	sendCommand(MATCH_ROM, WRITE_SCRATCHPAD);

	oneWire.write(selectedScratchpad[ALARM_HIGH]);
	oneWire.write(selectedScratchpad[ALARM_LOW]);
	oneWire.write(selectedScratchpad[CONFIGURATION]);

	sendCommand(MATCH_ROM, COPY_SCRATCHPAD, !selectedPowerMode);

	// Delay for 10 ms if the device is running in parasitic power mode according to datasheet.
	if(!selectedPowerMode)
	{
		delay(10);
	}
}

// Sends a rom command to the selected device.
uint8_t DS18B20::sendCommand(uint8_t romCommand)
{
	// Send reset pulse.
	if(!oneWire.reset())
	{
		// No presence pulse(s).
		return 0;
	}

	switch(romCommand)
	{
		case SEARCH_ROM:
		case SKIP_ROM:
		case ALARM_SEARCH:
			oneWire.write(romCommand);
			break;
		case MATCH_ROM:
			oneWire.select(selectedAddress);
			break;
		default:
			// Unsupported or unrecognized rom command.
			return 0;
	}

	return 1;
}

// Sends a rom command followed by a function command to the selected device.
uint8_t DS18B20::sendCommand(uint8_t romCommand, uint8_t functionCommand, uint8_t power)
{
	// Rom command failed for some reason.
	if(!sendCommand(romCommand))
	{
		return 0;
	}

	switch(functionCommand)
	{
		case CONVERT_T:
		case COPY_SCRATCHPAD:
			oneWire.write(functionCommand, power);
			break;
		case WRITE_SCRATCHPAD:
		case READ_SCRATCHPAD:
		case READ_POWER_SUPPLY:
			oneWire.write(functionCommand);
			break;
		default:
			// Unsupported or unrecognized function command.
			return 0;
	}

	return 1;
}

// Performs a 1-Wire search, either normal or conditional.
uint8_t DS18B20::oneWireSearch(uint8_t romCommand)
{
	// No presence pulse(s) or search completed. Reset search variables and end search early.
	if(lastDevice || !sendCommand(romCommand))
	{
		resetSearch();
		return 0;
	}

	uint8_t lastZero = 0;
	uint8_t direction, byteNumber, bitNumber, currentBit, currentBitComp;

	for(uint8_t bitPosition = 0; bitPosition < 64; bitPosition++)
	{
		currentBit = oneWire.read_bit();
		currentBitComp = oneWire.read_bit();

		// Happens if the device being discovered is disconnected, becomes faulty etc. during search.
		if(currentBit && currentBitComp)
		{
			lastDiscrepancy = 0;
			return 0;
		}

		// Current byte of the registration number and current bit number within that byte.
		byteNumber = bitPosition / 8;
		bitNumber = bitPosition % 8;

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
				direction = bitRead(searchAddress[byteNumber], bitNumber);

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
		bitWrite(searchAddress[byteNumber], bitNumber, direction);

		// Select only devices whose addresses match current direction.
		oneWire.write_bit(direction);
	}

	lastDiscrepancy = lastZero;

	// Search completed.
	if(!lastDiscrepancy)
	{
		lastDevice = 1;
	}

	return 1;
}

// Checks if the specified device is present on the bus.
uint8_t DS18B20::isConnected(uint8_t address[])
{
	if(!sendCommand(SEARCH_ROM))
	{
		return 0;
	}

	uint8_t currentBit, currentBitComp, byteNumber, bitNumber;

	for(uint8_t bitPosition = 0; bitPosition < 64; bitPosition++)
	{
		currentBit = oneWire.read_bit();
		currentBitComp = oneWire.read_bit();

		if(currentBit && currentBitComp)
		{
			return 0;
		}

		// Current byte of the registration number and current bit number within that byte.
		byteNumber = bitPosition / 8;
		bitNumber = bitPosition % 8;

		oneWire.write_bit(bitRead(address[byteNumber], bitNumber));
	}

	return 1;
}

// Delays for the amount of time required to perform a temperature conversion.
void DS18B20::delayForConversion(uint8_t resolution, uint8_t powerMode)
{
	if(powerMode)
	{
		// Poll sensor until temperature conversion is complete.
		while(!oneWire.read_bit());
	}
	else
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
}
