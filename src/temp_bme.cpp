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
#include <temp_bme.hpp>

TempSensorBME::~TempSensorBME() {}

bool setup_bme(Adafruit_BME280* bme, uint8_t addr) {
  bool status = bme->begin(addr);
  if (status) {
    // pass as int to keep the stack clean. %x expeects 4 bytes
    Log.info(F("TEMP: setup BME280 on 0x%x" CR), static_cast<int>(addr));
  } else {
    Log.info(F("TEMP: unbale to setup BME280 on 0x%x" CR),
             static_cast<int>(addr));
  }
  return status;
}

void TempSensorBME::setup() {
  _status = setup_bme(&_bme, BME280_ADDRESS);
  if (_status) {
    return;
  }
  _status = setup_bme(&_bme, BME280_ADDRESS_ALTERNATE);
  if (!_status) {
    Log.notice(F("TEMP: unable to setup BME280 on 0x76 or 0x77" CR));
  }
}

TempReading TempSensorBME::read() {
  if (!_status) return TEMP_READING_FAILED;

  TempReading reading;

  reading.temperature = _bme.readTemperature();
  reading.humidity = _bme.readHumidity();
  return reading;
}

// EOF
