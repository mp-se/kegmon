/*
MIT License

Copyright (c) 2021-2024 Magnus

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
#include <temp_ds.hpp>
#include <temp_mgr.hpp>

TempSensorManager::~TempSensorManager() {}

void TempSensorManager::setup() {
  Log.notice(F("SCAL: Initializing sensors on pins Data=%d." CR), PIN_DS);

  switch (myConfig.getTempSensorType()) {
    case SensorDS18B20:
      Log.info(F("TEMP: Initializing temp sensor DS18B20." CR));
      _sensor.reset(new TempSensorDS);
      break;


    default:
      Log.error(F("TEMP: Unable to find sensor type, exiting." CR));
      return;
  }

  if (_sensor) {
    _sensor->setup();
  } else {
    Log.error(F("TEMP: Unable to allocate sensor." CR));
    return;
  }

  read();

  if (!hasTemp())
    Log.error(F("TEMP: Failed to fetch temperature from sensor." CR));
}

void TempSensorManager::read() {
  if (!_sensor) return;

  for (int i = 0; i < getSensorCount() && i < 5; i++)
    _lastTemperature[i] = _sensor->read(i);
}

// EOF
