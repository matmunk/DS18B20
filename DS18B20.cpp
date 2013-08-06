#include <DS18B20.h>

DS18B20::DS18B20(uint8_t _pin) : oneWire(OneWire(_pin))
{
	pin = _pin;
	resetSearch();
}

// Gets the address of the next device.
uint8_t DS18B20::getNextDevice(uint8_t address[])
{
	return search(SEARCH_ROM, address);
}

// Resets the search so that the next search will return the first device again.
void DS18B20::resetSearch()
{
	lastDiscrepancy = 0;
	searchDone = 0;
}

// Returns the current temperature in degrees Celcius.
float DS18B20::getTempC(uint8_t address[])
{
	uint8_t resolution = getResolution(address);
	uint8_t parasite = isParasite(address);

	sendCommand(CONVERT_T, address, parasite);

	if(parasite)
	{
		// Wait for a specific amount of time depending on the resolution.
		delayForConversion(resolution);
	}
	else
	{
		// Wait while the temperature conversion is in progress.
		while(!oneWire.read_bit());
	}

	uint8_t scratchpad[SIZE_SCRATCHPAD];
	readScratchpad(scratchpad, address);

	uint8_t lsb = scratchpad[TEMP_LSB];
	uint8_t msb = scratchpad[TEMP_MSB];

	switch(resolution)
	{
		case 9:
			lsb &= 0xF8;
			break;
		case 10:
			lsb &= 0xFC;
			break;
		case 11:
			lsb &= 0xFE;
			break;
	}

	uint8_t sign = msb & 0x80;
	int16_t temp = (msb << 8) + lsb;

	if(sign)
	{
		temp = ((temp ^ 0xffff) + 1) * -1;
	}

	return temp / 16.0;
}

// Returns the current temperature in degrees Fahrenheit.
float DS18B20::getTempF(uint8_t address[])
{
	return getTempC(address) * 1.8 + 32;
}

// Returns the resolution of a device.
uint8_t DS18B20::getResolution(uint8_t address[])
{
	// Read scratchpad.
	// Contents of EEPROM are copied to scratchpad at power-up, so no need to read from EEPROM.
	uint8_t scratchpad[SIZE_SCRATCHPAD];
	readScratchpad(scratchpad, address);

	// Extract resolution from configuration register and return corresponding integer.
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

	while(getNextDevice(address))
	{
		setResolution(resolution, address);
	}
}

// Sets the resolution of a device.
void DS18B20::setResolution(uint8_t resolution, uint8_t address[])
{
	// Read scratchpad in order to retrieve and preserve alarm values.
	// Contents of EEPROM are copied to scratchpad at power-up, so no need to read from EEPROM.
	uint8_t scratchpad[SIZE_SCRATCHPAD];
	readScratchpad(scratchpad, address);

	// Make sure that resolution is in range [9;12].
	resolution = constrain(resolution, 9, 12);

	// Set the configuration register to the corresponding value.
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

	// Write new resolution to scratchpad and EEPROM.
	writeScratchpad(scratchpad, address);
}

// Returns the family code of a device.
uint8_t DS18B20::getFamilyCode(uint8_t address[])
{
	return address[0];
}

// Returns the power mode of a device.
// TRUE = parasite, FALSE = external.
uint8_t DS18B20::isParasite(uint8_t address[])
{
	sendCommand(READ_POWER_SUPPLY, address);

	return !oneWire.read_bit();
}

// Gets the address of the next active alarm.
uint8_t DS18B20::getNextAlarm(uint8_t address[])
{
	return search(ALARM_SEARCH, address);
}

// Sets both high and low alarms.
void DS18B20::setAlarms(uint8_t alarmHigh, uint8_t alarmLow, uint8_t address[])
{
	uint8_t scratchpad[SIZE_SCRATCHPAD];
	readScratchpad(scratchpad, address);

	scratchpad[ALARM_HIGH] = alarmHigh;
	scratchpad[ALARM_LOW] = alarmLow;

	writeScratchpad(scratchpad, address);
}

// Returns the value of the high alarm.
uint8_t DS18B20::getAlarmHigh(uint8_t address[])
{
	uint8_t scratchpad[SIZE_SCRATCHPAD];
	readScratchpad(scratchpad, address);

	return scratchpad[ALARM_HIGH];
}

// Sets the high alarm.
void DS18B20::setAlarmHigh(uint8_t alarmHigh, uint8_t address[])
{
	uint8_t scratchpad[SIZE_SCRATCHPAD];
	readScratchpad(scratchpad, address);

	scratchpad[ALARM_HIGH] = alarmHigh;

	writeScratchpad(scratchpad, address);
}

// Returns the value of the low alarm.
uint8_t DS18B20::getAlarmLow(uint8_t address[]) {
	uint8_t scratchpad[SIZE_SCRATCHPAD];
	readScratchpad(scratchpad, address);

	return scratchpad[ALARM_LOW];
}

// Sets the low alarm.
void DS18B20::setAlarmLow(uint8_t alarmLow, uint8_t address[])
{
	uint8_t scratchpad[SIZE_SCRATCHPAD];
	readScratchpad(scratchpad, address);

	scratchpad[ALARM_LOW] = alarmLow;

	writeScratchpad(scratchpad, address);
}

// Performs either a SEARCH_ROM or ALARM_SEARCH command.
// Returns TRUE/FALSE indicating whether the search was successful.
uint8_t DS18B20::search(uint8_t command, uint8_t address[])
{
	if(searchDone)
	{
		searchDone = 0;
		return 0;
	}

	if(!oneWire.reset())
	{
		lastDiscrepancy = 0;
		return 0;
	}

	uint8_t romBitIndex = 0;
	uint8_t discrepancy = 0;
	uint8_t currentBit;
	uint8_t currentBitComp;

	oneWire.write(command);

	for(romBitIndex; romBitIndex < 64; romBitIndex++)
	{
		// Current bit and its complement.
		currentBit = oneWire.read_bit();
		currentBitComp = oneWire.read_bit();

		// No devices in search.
		if(currentBit && currentBitComp)
		{
			lastDiscrepancy = 0;
			return 0;
		}

		// Both 0s and 1s in current bit position.
		if(!currentBit && !currentBitComp)
		{
			if(romBitIndex == lastDiscrepancy)
			{
				romBit[romBitIndex] = 1;
			}
			else if(romBitIndex > lastDiscrepancy)
			{
				romBit[romBitIndex] = 0;
				discrepancy = romBitIndex;
			}
			else if(romBit[romBitIndex] == 0)
			{
				discrepancy = romBitIndex;
			}
		}
		else
		{
			romBit[romBitIndex] = currentBit;
		}

		oneWire.write_bit(romBit[romBitIndex]);

		bitWrite(address[romBitIndex / 8], romBitIndex % 8, romBit[romBitIndex]);
	}

	lastDiscrepancy = discrepancy;

	if(!lastDiscrepancy)
	{
		searchDone = 1;
	}

	return 1;
}

// Reads the scratchpad of a device.
void DS18B20::readScratchpad(uint8_t scratchpad[], uint8_t address[])
{
	sendCommand(READ_SCRATCHPAD, address);

	for(uint8_t i = 0; i < SIZE_SCRATCHPAD; i++)
	{
		scratchpad[i] = oneWire.read();
	}
}

// Writes an array of data into the EEPROM of a device.
void DS18B20::writeScratchpad(uint8_t scratchpad[], uint8_t address[])
{
	uint8_t parasite = isParasite(address);

	sendCommand(WRITE_SCRATCHPAD, address);

	oneWire.write(scratchpad[ALARM_HIGH]);
	oneWire.write(scratchpad[ALARM_LOW]);
	oneWire.write(scratchpad[CONFIGURATION]);

	sendCommand(COPY_SCRATCHPAD, address, parasite);

	if(parasite)
	{
		delay(10);
	}
}

// Delays for the amount of time required to perform a temperature conversion at the specified resolution.
void DS18B20::delayForConversion(uint8_t resolution)
{
	// Make sure that resolution is in range [9;12].
	resolution = constrain(resolution, 9, 12);

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

// Sends a command to a device, with or without parasitic power at the end.
void DS18B20::sendCommand(uint8_t command, uint8_t address[], uint8_t parasite)
{
	if(!oneWire.reset())
	{
		// Do some error handling.
	}

	oneWire.select(address);

	oneWire.write(command, parasite);
}
