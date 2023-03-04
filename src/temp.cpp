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
#include <kegconfig.hpp>
#include <temp.hpp>

TempHumidity::TempHumidity() {}

void TempHumidity::setup() {
#if LOG_LEVEL == 6
  // Log.verbose(F("Temp: Initializing humidity sensor." CR));
#endif

  // Log.notice(F("Temp: Setup." CR));

  pinMode(PIN_DH2_PWR, OUTPUT);
  digitalWrite(PIN_DH2_PWR, HIGH);
  delay(100);

  if (_temp) delete _temp;
  _temp = new DHT(PIN_DH2, DHT22, 1);
  _temp->begin();
  read();

  if(hasSensor()) {
    Log.notice(F("Temp: Temp sensor found." CR));
  } else {
    Log.warning(F("Temp: Temp sensor NOT found." CR));
  }
}

void TempHumidity::reset() {
  // Log.notice(F("Temp: Reset." CR));

  digitalWrite(PIN_DH2_PWR, LOW);
  delay(100);
}

void TempHumidity::read() {
  if (!_temp) return;

  _lastTempC = _temp->readTemperature(false, false);
  _lastHumidity = _temp->readHumidity(false);

  // Log.notice(F("Temp: Reading temp %F C." CR), _lastTempC);

  if (isnan(_lastTempC)) {
    Log.error(F("Temp: Error reading temperature, disable sensor." CR));
    delete _temp;
    _temp = 0;
  }
}

// EOF
