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
#include <temp.hpp>
#include <config.hpp>

TempHumidity myTemp;

TempHumidity::TempHumidity() {
  Log.verbose(F("Temp: Initializing humidity sensor." CR));
  _temp = new DHT(PIN_DH2, DHT22, 1);
  _temp->begin();
}

float TempHumidity::getTempValueC() {
  if (!_temp)
    return NAN;

  float f = _temp->readTemperature();
  Log.verbose(F("Temp: Reading temp %F C." CR), f);

  if(isnan(f)) {
    Log.error(F("Temp: Error reading temperature." CR));
    return NAN;
  }

  return f;
}

float TempHumidity::getTempValueF() {
  if (!_temp)
    return NAN;

  float f = _temp->readTemperature(true);
  Log.verbose(F("Temp: Reading temp %F F." CR), f);

  if(isnan(f)) {
    Log.error(F("Temp: Error reading temperature." CR));
    return NAN;
  }

  return f;
}

float TempHumidity::getHumidityValue() {
  if (!_temp)
    return NAN;

  float h = _temp->readHumidity();
  Log.verbose(F("Temp: Reading humidity %F." CR), h);

  if(isnan(h)) {
    Log.error(F("Temp: Error reading humidity." CR));
    return NAN;
  }

  return h;
}

// EOF
