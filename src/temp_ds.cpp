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
#include <main.hpp>
#include <temp_ds.hpp>
#include <utils.hpp>

TempSensorDS::~TempSensorDS() {
  if (_oneWire) delete _oneWire;
  if (_dallas) delete _dallas;
}

void TempSensorDS::setup() {
  _oneWire = new OneWire(PIN_DH2);
  _dallas = new DallasTemperature(_oneWire);
  _dallas->setResolution(12);
  _dallas->begin();
}

TempReading TempSensorDS::read() {
  TempReading reading = TEMP_READING_FAILED;

  if (!_dallas) return reading;

  if (_dallas->getDS18Count()) {
    _dallas->requestTemperatures();
    reading.temperature = _dallas->getTempCByIndex(0);
    reading.humidity = NAN;
    reading.pressure = NAN;
  } else {
    Log.error(F("TEMP: No DS18B20 sensors found." CR));
  }

  return reading;
}

// EOF
