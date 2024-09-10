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
#include <barhelper.hpp>
#include <kegconfig.hpp>
#include <log.hpp>

/*void Barhelper::sendPourInformation(UnitIndex idx, float pourVol) {
  //
  // API: https://europe-west1-barhelper-app.cloudfunctions.net/api/customKegMon
  // Descr: This method just reports the pourVolume.
  // Header: authorization with APIKEY
  // Payload:
  // {
  //   name: string, <name of monitor>
  //   amount: number <pour volume>
  //   type: string, <unit of volume>
  // }
  if (strlen(myConfig.getBarhelperApiKey()) == 0) return;

  DynamicJsonDocument doc(100);
  String header = "Authorization: " + String(myConfig.getBarhelperApiKey());

  doc["name"] = myConfig.getBarhelperMonitor(idx);
  doc["amount"] = pourVol;
  doc["type"] = "l";
  Log.notice(F("BARH: Sending TAP information to barhelper "
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
  String response = _push->sendHttpPost(
      out,
      "https://europe-west1-barhelper-app.cloudfunctions.net/api/customKegMon",
      "Content-Type: application/json", header.c_str());

  DeserializationError err = deserializeJson(doc, response);

  if (err) {
    Log.error(
        F("BARH: Failed to parse response from barhelper, Err: %s, %s." CR),
        err.c_str(), response.c_str());
    return;
  }

  // Typical response
  // {
  // "amount: 0.1"
  // "prevKegAmount": 6,
  // "newKegAmount": 5.9,
  // "success": true,
  // "message": "Keg Monitor updated successfully"
  // }

  JsonObject obj = doc.as<JsonObject>();
  Log.info(F("BARH: Server responded: %s, new level=%F, message=%s." CR),
           obj["success"].as<bool>() ? "True" : "False",
           obj["newKegAmount"].as<float>(),
           obj["message"].as<String>().c_str());
}*/

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

  DynamicJsonDocument doc(300);
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
  out = _push->sendHttpPost(
      out,
      "https://europe-west1-barhelper-app.cloudfunctions.net/api/customKegMon",
      "Content-Type: application/json", header.c_str());
  updateStatus(out);

  Log.info(F("BARH: Response %s." CR), out.c_str());

  // Note! the server does not return a valid json so it cant be parsed. Just
  // print the value for now.

  /*DeserializationError err = deserializeJson(doc, out);

  /*if (err) {
    Log.error(
        F("BARH: Failed to parse response from barhelper, Err: %s, %s." CR),
        err.c_str(), out.c_str());
    return;
  }*/

  // Typical response
  // {
  // "amount: 0.1"
  // "prevKegAmount": 6,
  // "newKegAmount": 5.9,
  // "success": true,
  // "message": "Keg Monitor updated successfully"
  // }

  /*JsonObject obj = doc.as<JsonObject>();
  Log.info(F("BARH: Server responded: %s, new level=%F, message=%s." CR),
           obj["success"].as<bool>() ? "True" : "False",
           obj["newKegAmount"].as<float>(),
           obj["message"].as<String>().c_str());*/
}

void Barhelper::updateStatus(String& response) {
  _lastTimestamp = millis();
  _lastStatus = _push->wasLastSuccessful();
  _lastHttpError = _push->getLastResponseCode();
  _lastResponse = response;
  _hasRun = true;
}

// EOF
