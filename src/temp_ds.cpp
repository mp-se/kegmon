/*
MIT License

Copyright (c) 2021-2026 Magnus

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
#include <main.hpp>
#include <temp_ds.hpp>
#include <utils.hpp>

TempSensorDS::~TempSensorDS() {
  if (_oneWire) delete _oneWire;
  if (_dallas) delete _dallas;
}

void TempSensorDS::setup() {
  _oneWire = new OneWire(PIN_DS);
  _dallas = new DallasTemperature(_oneWire);
  _dallas->setResolution(12);
  _dallas->begin();

  if (_dallas->getDS18Count())
    _hasSensor = true;
  else
    _hasSensor = false;
}

float TempSensorDS::read(int index) {
  float temp = NAN;

  if (!_dallas) return temp;

  int cnt = _dallas->getDS18Count();

  if (cnt > index) {
    _dallas->requestTemperatures();
    temp = _dallas->getTempCByIndex(index);
  } else {
    Log.error(F("TEMP: No DS18B20 sensor found at index %d." CR), index);
  }

  return temp;
}

int TempSensorDS::getSensorCount() const {
  if (_dallas)
    return _dallas->getDS18Count();
  else
    return 0;
}

String TempSensorDS::getSensorId(int index) const {
  DeviceAddress adr;
  _dallas->getAddress(&adr[0], index);
  return String(adr[0], 16) + String(adr[1], 16) + String(adr[2], 16) +
         String(adr[3], 16) + String(adr[4], 16) + String(adr[5], 16) +
         String(adr[6], 16) + String(adr[7], 16);
}

// EOF
