#include <vector>
#include <DS18B20.h>

#define ONE_WIRE_PIN (2)
#define ONE_WIRE_PIN (5)

DS18B20 ds(ONE_WIRE_PIN);

/* Demonstrate sequential measure/convert versus instructing all
 * sensors on a string to measure/convert in one go; and then 
 * read them out sequentially.
 * 
 * The advantage of the latter is that it avoids taking (at 12 bits) around
 * 700 mSeconds for a whole measure/convert cycle (see [1], page 3
 * and Figure 2) for each readout. Instead; all DS18B20's all do
 * their conversion in parallel.
 *
 * Typical readout of multiple (e.g. over 3) sensors
 * is around N x 0.8 seconds. This is largely spend
 * in waiting for the AD/conversion cycle in the sensor.
 * 
 * It is possible to start the measurement and AD/conversion
 * cycle; but not read it out right away. Then wait for these
 * to complete - and do a fast reading past. The time then
 * becomes 0.7 + N * 0.04 or there about.
 *
 * Note: this requires the sensors to be powered.
 *
 * Typical output:
 *  
 * 00:08:18.905 -> Devices: 12
 * 00:08:27.979 -> Sequential: 19.56 18.94 19.62 20.00 19.56 19.69 19.44 19.50 19.44 19.50 19.37 19.81 [˚C]
 * 00:08:28.092 -> Parallel:   19.56 19.00 19.62 20.00 19.56 19.69 19.44 19.50 19.50 19.50 19.37 19.87 [˚C]
 * 00:08:28.167 -> Time sequential run: 8.38[sec] or 12 x 0.70[sec]/#
 * 00:08:28.239 -> Time parallel run:   0.71[sec] or 12 x 0.06[sec]/# (average, first one may 
 *                                                                     be slow, the rest are fast)
 *
 * 1: https://www.analog.com/media/en/technical-documentation/data-sheets/DS18B20.pdf
 */

void setup() {
  Serial.begin(9600);
  Serial.println("\n\n\n" __FILE__ " " __DATE__ " " __TIME__);
}

void loop() {
  unsigned long st, dt_seq, dt_par;
  size_t n = ds.getNumberOfDevices();

  Serial.print("Devices: ");
  Serial.println(n);

  // We store the results; as opposed to printing
  // them right away; to not have the serial
  // fifo have an influence on our timing measurements.
  //
  std::vector<float> seq, par;

  // Read each sensor out one by one; i.e.
  // start the measurement and A/D conversion,
  // wait for it to complete and then read 
  // it out & convert.
  //
  st = millis();
  while (ds.selectNext())
    seq.push_back(ds.getTempC());
  dt_seq = millis() - st;

  // Start the measuring/conversion on all devices; but do
  // not wait to readm them out.
  //
  st = millis();
  ds.startConversion(); // will return immediately; call ds.doConversion() if you want it to wait for completion

  // And then read them one by one. getTempC() will
  // check, and wait if needed, for the conversion to
  // complete - so the very first one is likely to be
  // slower than the remaining ones; as there is
  // still some 500-600 mS to go before the
  // conversion completes.
  //
  while (ds.selectNext())
    par.push_back(ds.getTempC(false));
  dt_par = millis() - st;

  Serial.print("Sequential:");
  for (auto temp : seq) {
    Serial.print(" ");
    Serial.print(temp);
  };
  Serial.println(" [˚C]");

  Serial.print("Parallel:  ");
  for (auto temp : par) {
    Serial.print(" ");
    Serial.print(temp);
  };
  Serial.println(" [˚C]");

  Serial.print("Time taken for a sequential run: ");
  Serial.print(dt_seq / 1000.);
  Serial.print("[sec] or ");
  Serial.print(n);
  Serial.print(" x ");
  Serial.print(dt_seq / 1000. / n);
  Serial.println("[sec]/#");

  Serial.print("Time taken for a parallel run:   ");
  Serial.print(dt_par / 1000.);
  Serial.print("[sec] or ");
  Serial.print(n);
  Serial.print(" x ");
  Serial.print(dt_par / 1000. / n);
  Serial.println("[sec]/# (average, first one may be slow, the rest are fast)");

  delay(1000);
  Serial.print("\n--------\n\n");
};
