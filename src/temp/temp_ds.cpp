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
#include "temp_ds.hpp"
#include <main.hpp>
#include <utils.hpp>

void DSSensor::setup() {
  Log.notice(F("TEMP: Initializing DS18B20 sensor." CR));
  reset();

  _oneWire = std::make_unique<OneWire>(PIN_DH2);
  _dallas = std::make_unique<DallasTemperature>(_oneWire.get());
  _dallas->setResolution(12);
  _dallas->begin();
}

void DSSensor::reset() {
  _dallas.reset();
  _oneWire.reset();
}

tempReading DSSensor::read() {
  float temp;   
  if (!_dallas) return failedReading;

  if (_dallas->getDS18Count()) {
    _dallas->requestTemperatures();
    temp = _dallas->getTempCByIndex(0);
  }

  if (isnan(temp)) {
    Log.error(F("TEMP: Error reading temperature, disable sensor." CR));
    reset();
  }

  return {
    temp,
    NAN
  };
}
