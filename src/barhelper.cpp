/*
MIT License

Copyright (c) 2024-2025 Magnus

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
#include <barhelper.hpp>
#include <kegconfig.hpp>
#include <log.hpp>

void Barhelper::sendKegInformation(UnitIndex idx, float kegVol) {
  //
  // API: https://europe-west1-barhelper-app.cloudfunctions.net/api/customKegMon
  // Descr: This method just reports the pourVolume.
  // Header: authorization with APIKEY
  // Payload:
  // {
  //   name: string, <name of monitor>
  //   volume: number <keg volume in liters>
  //   type: string, <unit of volume>
  // }
  if (strlen(myConfig.getBarhelperApiKey()) == 0) return;

  JsonDocument doc;
  String header = "Authorization: " + String(myConfig.getBarhelperApiKey());

  doc["name"] = myConfig.getBarhelperMonitor(idx);
  doc["volume"] = kegVol;
  doc["type"] = "l";
  Log.notice(F("BARH: Sending TAP information to barhelper "
               "keg %Fl [%d]" CR),
             kegVol, idx);

  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();
  // #if LOG_LEVEL == 6
  EspSerial.print(out.c_str());
  EspSerial.print(CR);
  // #endif

  constexpr auto BARHELPER_URL =
      "https://europe-west1-barhelper-app.cloudfunctions.net/api/customKegMon";
  Log.info(F("BARH: Using URL %s." CR), BARHELPER_URL);
  out = _push->sendHttpPost(out, BARHELPER_URL,
                            "Content-Type: application/json", header.c_str());
  updateStatus(out);
  Log.info(F("BARH: Response %s." CR), out.c_str());
}

void Barhelper::updateStatus(String& response) {
  _lastTimestamp = millis();
  _lastStatus = _push->wasLastSuccessful();
  _lastHttpError = _push->getLastResponseCode();
  _lastResponse = response;
  _hasRun = true;
}

// EOF
