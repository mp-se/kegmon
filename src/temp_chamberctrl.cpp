/*
MIT License

Copyright (c) 2025 Magnus

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
#include <temp_chamberctrl.hpp>
#include <utils.hpp>

TempSensorChamberCtrl::TempSensorChamberCtrl() {
  _push = new BasePush(&myConfig);
}

TempSensorChamberCtrl::~TempSensorChamberCtrl() {
  if (_push) delete _push;
}

TempReading TempSensorChamberCtrl::read() {
  TempReading reading = TEMP_READING_FAILED;
  String url = myConfig.getChamberCtrlUrl();

  if (url.length() > 0 || _push != NULL) {
    String ret;

    url += "/api/temps";

    Log.notice(F("TEMP: Fetching temperature from %s." CR), url.c_str());

    ret = _push->sendHttpGet(ret, url.c_str(), "", "");

    if (_push->getLastResponseCode() == 200) {
      Log.notice(F("TEMP: Data received %s." CR), ret.c_str());

      JsonDocument doc;
      DeserializationError err = deserializeJson(doc, ret);

      /* This is the  payload structure from BrewPI-ESP by Thorrak
      {
          "pid_beer_temp": 0,
          "pid_fridge_temp": 19.5,
          "pid_beer_target_temp": 0,
          "pid_fridge_target_temp": 7,
          "pid_temp_format": "C"
      }
     */

      if (!err) {
        reading.temperature = doc["pid_fridge_temp"].as<float>();
        _hasSensor = true;
        return reading;
      }
    }
  }

  _hasSensor = false;
  return reading;
}

// EOF
