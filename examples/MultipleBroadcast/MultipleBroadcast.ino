#include <DS18B20.h>

/* Configuration (start) */
const uint8_t CFG_N_DEVICES = 2; // number of sensors in the system
const uint8_t CFG_BUS_PIN   = 3;  // 1-Wire data pin
const uint8_t CFG_INTERVAL  = 1000; // one second

/* The CFG_INTERVAL value MUST be consistent with the sensor conversion time (see the documentation)
 Sensor conversion time table:
   --------------------------------
   Resolution           | Time
   --------------------------------
   12 bit (default)     | 750ms
   11 bit               | 750ms / 2
   10 bit               | 750ms / 4
   9  bit               | 750ms / 8
   --------------------------------
*/

/* A NOTE ON THE RESOLUTION SETTING:

 It seems fake sensors (as we probably have here)
 don't allow to change the resolution so
 you are able to use only the default one --- 12 bits.
*/

/* Printing format
 a) If defined printing format is the following  (4-column temperature table):

    temp  temp  temp  temp
    ...
    temp  temp  temp  temp

 useful when debugging.

 b) If undefined (e.g. commented) printing format is a list of 3 fields:

    temp
    resolution
    address

    temp
    resolution
    address

    ...
 */
#define CFG_TABLE_VIEW
/* Configuration (end) */

/* SensorData (start) */
struct SensorData
{
  uint8_t resolution;
  float   temperature;
  uint8_t address[8];

  void print();
  void reset();
};

void SensorData::print()
{
  Serial.print( "Address: " );
  for( uint8_t i = 0; i < 8; ++i )
  {
    Serial.print( address[i], HEX );
  }
  Serial.println();
  Serial.print( "Resolution: " );
  Serial.println( resolution );
  Serial.print( "Temperature: " );
  Serial.println( temperature );
}

void SensorData::reset()
{
  temperature = 0.0;
  resolution  = 0;
  for( uint8_t i = 0; i < 8; ++i )
  {
    address[i] = 0;
  }
}
/* SensorData (end) */

/* Global (start) */
DS18B20 sensor( CFG_BUS_PIN );
SensorData sensors[CFG_N_DEVICES];
/* Global (end) */

void setup()
{
  Serial.begin( 115200 );
}

unsigned long now = 0;

void loop()
{
  if( millis() - now > CFG_INTERVAL )
  {
      for( uint8_t i = 0; sensor.selectNext() and (i < CFG_N_DEVICES); ++i )
      {
        sensors[i].temperature = sensor.getTempCFromScratchPad();
        sensors[i].resolution  = sensor.getResolution();
        sensor.getAddress( sensors[i].address );
      }
      sensor.doConversion( false );
      now = millis();
  }

#ifdef CFG_TABLE_VIEW
    Serial.print("============================");
    for( uint8_t i = 0; i < CFG_N_DEVICES; ++i )
    {
      if( i % 4 == 0 ) Serial.println();
      Serial.print( sensors[i].temperature );
      Serial.print( "  " );
    }
    Serial.println();
    Serial.print("============================");
    Serial.println();
#else
    for( uint8_t i = 0; i < CFG_N_DEVICES; ++i )
    {
      sensors[i].print();
      Serial.println();
    }
#endif
}
