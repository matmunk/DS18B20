#include <DS18B20.h>

DS18B20 ds(2);

void setup() {
  Serial.begin(9600);
  Serial.print("Devices: ");
  Serial.println(ds.getNumberOfDevices());
  Serial.println();
}

void loop() {
  // Prints the addresses formatted as "<type>-<network order address>-<crc xx>" as
  // is commonly used in various unix drivers and documentation/engravings on the
  // actual device.
  //
  while (ds.selectNext()) 
    Serial.println(ds.getAddress(true));

  delay(10000);
}
