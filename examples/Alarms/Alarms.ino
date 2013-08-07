#include <OneWire.h>
#include <DS18B20.h>

// Low alarm in degrees Celcius.
#define LOW_ALARM 20

// High alarm in degrees Celcius.
#define HIGH_ALARM 25

// 1-Wire devices connected to digital pin 2 on the Arduino.
DS18B20 ds(2);

// Array for holding the 64 bit address of the current device.
uint8_t address[8];

void setup() {
  Serial.begin(9600);
  
  // Set low and high alarms of all devices.
  while(ds.getNextDevice(address))
  {
    ds.setAlarms(LOW_ALARM, HIGH_ALARM, address);
  }
}

void loop()
{
  // Tell every device to start a temperature conversion.
  ds.startConversion();
  
  // Print alarm values and current temperature for every device with an active alarm.
  while(ds.getNextAlarm(address))
  {
    // Print value of low alarm.
    Serial.print("Alarm Low: ");
    Serial.print(ds.getAlarmLow(address));
    Serial.println(" C");
    
    // Print value of high alarm.
    Serial.print("Alarm High: ");
    Serial.print(ds.getAlarmHigh(address));
    Serial.println(" C");
    
    // Print current temperature to verify that it is still either < LOW_ALARM or > HIGH_ALARM.
    Serial.print("Temperature: ");
    Serial.print(ds.getTempC(address));
    Serial.println(" C\n");
  }
  
  // Wait 10 seconds.
  delay(10000);
}
