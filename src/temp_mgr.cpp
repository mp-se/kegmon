/*
MIT License

Copyright (c) 2021-23 Magnus

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
#include <kegconfig.hpp>
#include <temp_dht.hpp>
#include <temp_ds.hpp>
#include <temp_mgr.hpp>

bool operator==(const TempReading& lhs, const TempReading& rhs) {
  return lhs.humidity == rhs.humidity && lhs.temperature == rhs.temperature;
}

TempSensorManager::~TempSensorManager() {
  if (_sensor) delete _sensor;
}

void TempSensorManager::setup() {
  pinMode(PIN_DH2_PWR, OUTPUT);
  reset();
  digitalWrite(PIN_DH2_PWR, HIGH);
  delay(100);

  if (_sensor) {
    delete _sensor;
    _sensor = 0;
  }

  switch (myConfig.getTempSensorType()) {
    case SensorDHT22:
      Log.info(F("TEMP: Initializing temp sensor DHT22." CR));
      _sensor = new TempSensorDHT;
      break;

    case SensorDS18B20:
      Log.info(F("TEMP: Initializing temp sensor DS18B20." CR));
      _sensor = new TempSensorDS;
      break;

    default:
      Log.error(F("TEMP: Unable to find sensor type, exiting." CR));
      return;
  }

  if (_sensor)
    _sensor->setup();
  else
    return;

  read();

  if (!hasTemp())
    Log.error(F("TEMP: Failed to fetch temperature from sensor." CR));
}

void TempSensorManager::reset() {
  Log.notice(F("TEMP: Reset temperature sensor." CR));
  digitalWrite(PIN_DH2_PWR, LOW);
  delay(100);
}

void TempSensorManager::read() {
  if (!_sensor) return;

  _last = _sensor->read();
}

// EOF
