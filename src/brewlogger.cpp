/*
MIT License

Copyright (c) 2024 Magnus

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
#include <brewlogger.hpp>
#include <kegconfig.hpp>
#include <log.hpp>

void BrewLogger::sendPourInformation(UnitIndex idx, float pourVol,
                                     float kegVol) {
  if (strlen(myConfig.getBrewLoggerUrl()) == 0) return;

  DynamicJsonDocument doc(300);

  doc["id"] = myConfig.getBeerId(idx);
  doc["pour"] = pourVol;
  doc["volume"] = kegVol;
  doc["maxVolume"] = myConfig.getKegVolume(idx);

  Log.notice(F("BLOG: Sending pour information to brewlogger "
               "pour %Fl [%d]" CR),
             pourVol, idx);

  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();
  // #if LOG_LEVEL == 6
  EspSerial.print(out.c_str());
  EspSerial.print(CR);
  // #endif

  out = _push->sendHttpPost(out, myConfig.getBrewLoggerUrl(),
                            "Content-Type: application/json", "");
  updateStatus(out);
  Log.info(F("BLOG: Response %s." CR), out.c_str());
}

void BrewLogger::sendKegInformation(UnitIndex idx, float kegVol) {
  if (strlen(myConfig.getBrewLoggerUrl()) == 0) return;

  DynamicJsonDocument doc(300);

  doc["id"] = myConfig.getBeerId(idx);
  doc["volume"] = kegVol;
  doc["maxVolume"] = myConfig.getKegVolume(idx);

  Log.notice(F("BLOG: Sending level information to brewlogger "
               "kegVol %Fl [%d]" CR),
             kegVol, idx);

  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();
  // #if LOG_LEVEL == 6
  EspSerial.print(out.c_str());
  EspSerial.print(CR);
  // #endif

  out = _push->sendHttpPost(out, myConfig.getBrewLoggerUrl(),
                            "Content-Type: application/json", "");
  updateStatus(out);
  Log.info(F("BLOG: Response %s." CR), out.c_str());
}

void BrewLogger::updateStatus(String& response) {
  _lastTimestamp = millis();
  _lastStatus = _push->wasLastSuccessful();
  _lastHttpError = _push->getLastResponseCode();
  _lastResponse = response;
  _hasRun = true;
}

// EOF
