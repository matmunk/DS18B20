#include <OneWire.h>
#include <DS18B20.h>

// 1-Wire devices connected to digital pin 2 on the Arduino.
DS18B20 ds(2);

// Array for holding the 64 bit address of the current device.
uint8_t address[8];

void setup()
{
  Serial.begin(9600);
  
  // Set the resolution to 12 bit on all devices.
  ds.setResolution(12);
}

void loop()
{
  // Iterate through all devices.
  while(ds.getNextDevice(address))
  {
    // Print the temperature of the current device (in degrees Celcius).
    Serial.println(ds.getTempC(address));
  }
  
  // Wait 10 seconds.
  delay(10000);
}
