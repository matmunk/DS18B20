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

void setup() {
  Serial.begin(9600);
  
  // Set low and high alarms of all devices.
  while(ds.selectNext())
  {
    ds.setAlarms(LOW_ALARM, HIGH_ALARM);
  }
}

void loop()
{
  // Tell every device to start a temperature conversion.
  ds.doConversion();
  
  // Print alarm values and current temperature for every device with an active alarm condition.
  while(ds.selectNextAlarm())
  {
    // Print value of low alarm.
    Serial.print("Alarm Low: ");
    Serial.print(ds.getAlarmLow());
    Serial.println(" C");
    
    // Print value of high alarm.
    Serial.print("Alarm High: ");
    Serial.print(ds.getAlarmHigh());
    Serial.println(" C");
    
    // Print current temperature to verify that it is still either < LOW_ALARM or > HIGH_ALARM.
    Serial.print("Temperature: ");
    Serial.print(ds.getTempC());
    Serial.println(" C\n");
  }
  
  // Wait 10 seconds.
  delay(10000);
}
