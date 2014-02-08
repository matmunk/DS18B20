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

// Low/high alarm in degrees Celcius.
#define LOW_ALARM 20
#define HIGH_ALARM 25

// 1-Wire devices connected to digital pin 2 on the Arduino.
DS18B20 ds(2);

// Address of the device.
uint8_t address[] = {40, 250, 31, 218, 4, 0, 0, 52};

// Indicates if the device was successfully selected.
uint8_t selected;

void setup()
{
  Serial.begin(9600);
  
  // Select device.
  selected = ds.select(address);
  
  if(selected)
  {
    // Set alarms.
    ds.setAlarms(LOW_ALARM, HIGH_ALARM);
  }
  else
  {
    Serial.println("Device not found!");
  }
}

void loop()
{
  // Check if the device has an active alarm condition.
  if(selected)
  {
    if(ds.hasAlarm())
    {
      Serial.print("Warning! Temperature is ");
      Serial.print(ds.getTempC());
      Serial.println(" C");
    }
  }
  else
  {
    Serial.println("Device not found!");
  }
  
  // Wait 10 seconds.
  delay(10000);
}
