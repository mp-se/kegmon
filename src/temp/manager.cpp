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
      _sensor = new DHTSensor();
      break;
    case SensorDS18B20:
      _sensor = new DSSensor();
      break;
  }

  _sensor->setup();

  _lastTempC = NAN;
  _lastHumidity = NAN;
  read();
}

void TempHumidity::reset() {
  _sensor->reset();
}

void TempHumidity::read() {
  auto reading = _sensor->read();
  _lastTempC = reading.temprature;
  if (reading.humidity != NAN) {
    _lastHumidity = reading.humidity;
  }
}

// EOF
