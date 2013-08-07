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
    // Print current resolution of the device.
    Serial.print("Resolution: ");
    Serial.println(ds.getResolution(address));
    
    // Print current power mode of the device.
    Serial.print("Power Mode: ");
    if(ds.isParasite(address))
    {
      Serial.println("Parasite");
    }
    else
    {
      Serial.println("External");
    }
    
    // Print current temperature in degrees Celcius and Fahrenheit.
    Serial.print("Temperature: ");
    Serial.print(ds.getTempC(address));
    Serial.print(" C / ");
    Serial.print(ds.getTempF(address));
    Serial.println(" F\n");
  }
  
  // Wait 10 seconds.
  delay(10000);
}
