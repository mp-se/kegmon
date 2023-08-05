/*
MIT License

Copyright (c) 2021-22 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#include "manager.hpp"
#include "temp_ds.hpp"
#include "temp_dht.hpp"
#include <kegconfig.hpp>

TempHumidity::TempHumidity() {}

void TempHumidity::setup() {
#if LOG_LEVEL == 6
  // Log.verbose(F("Temp: Initializing temperature/humidity sensor." CR));
#endif

  switch (myConfig.getTempSensorType()) {
    case SensorDHT22:
      Log.info(F("TEMP: choosing DHT22." CR));
      _sensor = std::make_unique<DHTSensor>();
      break;
    case SensorDS18B20:
      Log.info(F("TEMP: choosing DS18B20." CR));
      _sensor = std::make_unique<DSSensor>();
      break;
    default:
      Log.error(F("TEMP: unable to find sensor type, exiting." CR));
      return;
  }

  _sensor->setup();

  _lastTempC = NAN;
  _lastHumidity = NAN;
  read();
}

void TempHumidity::reset() {
  if (!_sensor) {
    Log.error(F("TEMP: reset called with no sensor." CR));
    return;
  }
  _sensor->reset();
}

void TempHumidity::read() {
  if (!_sensor) {
    Log.error(F("TEMP: no sensor, read failed." CR));
    return;
  }
  auto reading = _sensor->read();
  if (reading == failedReading) {
    Log.notice(F("TEMP: error reading sensor." CR));
  }

  _lastTempC = reading.temprature;
  if (!isnan(reading.humidity)) {
    _lastHumidity = reading.humidity;
  }
}

// EOF
