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

#include <OneWire.h>
#include <DS18B20.h>

// 1-Wire devices connected to digital pin 2 on the Arduino.
DS18B20 ds(2);

void setup()
{
  Serial.begin(9600);
  
  // Print number of devices on the bus.
  Serial.print("Devices: ");
  Serial.println(ds.getNumberOfDevices());
  Serial.println();
}

void loop()
{
  // Iterate through all devices.
  while(ds.selectNext())
  {
    // Print family name.
    switch(ds.getFamilyCode())
    {
      case MODEL_DS18S20:
        Serial.println("Model: DS18S20");
        break;
      case MODEL_DS1820:
        Serial.println("Model: DS1820");
        break;
      case MODEL_DS18B20:
        Serial.println("Model: DS18B20");
        break;
      default:
        Serial.println("Unrecognized Device");
        break;
    }
    
    // Print address.
    uint8_t address[8];
    
    ds.getAddress(address);
    
    Serial.print("Address:");
    
    for(uint8_t i = 0; i < 8; i++)
    {
      Serial.print(" ");
      Serial.print(address[i]);
    }
    
    Serial.println();
    
    // Print resolution.
    Serial.print("Resolution: ");
    Serial.println(ds.getResolution());
    
    // Print power mode.
    Serial.print("Power Mode: ");
    
    if(ds.getPowerMode())
    {
      Serial.println("External");
    }
    else
    {
      Serial.println("Parasite");
    }
    
    // Print temperature in degrees Celcius and degrees Fahrenheit.
    Serial.print("Temperature: ");
    Serial.print(ds.getTempC());
    Serial.print(" C / ");
    Serial.print(ds.getTempF());
    Serial.println(" F");
    
    // Print an empty line.
    Serial.println();
  }
  
  // Wait 10 seconds.
  delay(10000);
}
