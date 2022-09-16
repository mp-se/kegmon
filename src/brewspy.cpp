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
#include <brewspy.hpp>
#include <utils.hpp>
#include <scale.hpp>
#include <log.hpp>

void BrewspyPushHandler::sendTapInformation(UnitIndex idx) {
//
// API: https://brew-spy.com/api/tap/keg/set
// Descr: This method just sets all the keg related fields.
// Payload:
// {
//   token: string, *
//   unit: string, **
//   kegVolume: number, ***
//   beerLeft: number,
//   pour: number ****
// }
// * To get the token for a tap, click on it and look at the Webhook URL. The token is the alphanumeric string at the end of the url (right after /taplist/)
// ** can be: l, ml, or pint. Let me know if support for imperial units is needed. 
// *** Initial volume of the keg - used to calculate percentage left
// **** Last pour - this is optional - use it if you wish to control the amount of beer left yourself, otherwise you can use the api call below for setting the last pour
  DynamicJsonDocument doc(100);

  doc["token"] = myConfig.getBrewspyToken(idx);
  doc["unit"] = "l";
  doc["kegVolume"] = myConfig.getKegVolume(idx);
  doc["beerLeft"] = 0; // TODO: Convert to volume
  doc["pour"] = 0; // TODO: Fix apis

  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();
  sendHttpPost(out, "https://brew-spy.com/api/tap/keg/set", "", "");
}

void BrewspyPushHandler::sendPourInformation(UnitIndex idx) {
// API: https://brew-spy.com/api/tap/keg/pour
// Descr: Sets the last pour field and subtracts the volume from the beer left field
// Payload:
// {
//   token: string,
//   unit: string,
//   pour: number
// }
  DynamicJsonDocument doc(100);

  doc["token"] = myConfig.getBrewspyToken(idx);
  doc["unit"] = "l";
  doc["pour"] = 0; // TODO: Fix apis

  String out;
  out.reserve(100);
  serializeJson(doc, out);
  doc.clear();
  sendHttpPost(out, "https://brew-spy.com/api/tap/keg/pour", "", "");
}

void BrewspyPushHandler::clearKegInformation(UnitIndex idx) {
// API: https://brew-spy.com/api/tap/keg/clear
// Descr: Clears all fields for the tap
// Payload:
// {
//   token: string
// }
  DynamicJsonDocument doc(30);

  doc["token"] = myConfig.getBrewspyToken(idx);

  String out;
  out.reserve(30);
  serializeJson(doc, out);
  doc.clear();
  sendHttpPost(out, "https://brew-spy.com/api/tap/keg/clear", "", "");
}

String BrewspyPushHandler::getTapInformation(const String& token) {
// API: https://brew-spy.com/api/json/taplist/<TOKEN>
// Descr: To view the current data for a tap you can use the 
// Payload:
// {
//   "tap" : <tap number>,
//   "recipe" : <beer name>,
//   "abv" : <abv>,
//   "style" : <beer style>,
//   "date" : <date added on tap>,
//   "kegLevel" : {
//     "pcnt" : <perecentage beer left>,
//     "pour" : <last pour volume>,
//     "pourUnit" : <last pour volume unit>,
//     "vol" : <volume of beer left>,
//     "volUnit" : <volume of beer left unit>,
//   }
// }

  String url = "https://brew-spy.com/api/json/taplist/" + token;
  String resp = httpGet(url.c_str(), "", "");
  return resp;
}

String BrewspyPushHandler::httpGet(const char* target, const char* header1, const char* header2) {
  Log.notice(F("BSPY: Doing http get request" CR));
  _lastResponseCode = 0;
  _lastSuccess = false;
  String _response = "{}";

  if (isSecure(target)) {
    Log.notice(F("BSPY: HTTP, SSL enabled without validation." CR));
    _wifiSecure.setInsecure();
    probeMFLN(target);
    _httpSecure.setTimeout(_tcpTimeout * 1000);
    _httpSecure.begin(_wifiSecure, target);
    addHttpHeader(_httpSecure, header1);
    addHttpHeader(_httpSecure, header2);
    _lastResponseCode = _httpSecure.GET();
  } else {
    _http.setTimeout(_tcpTimeout * 1000);
    _http.begin(_wifi, target);
    addHttpHeader(_http, header1);
    addHttpHeader(_http, header2);
    _lastResponseCode = _http.GET();
  }

  if (_lastResponseCode == 200) {
    _lastSuccess = true;
    Log.notice(F("BSPY: HTTP get successful, response=%d" CR), _lastResponseCode);
  } else {
    Log.error(F("BSPY: HTTP get failed, response=%d" CR), _lastResponseCode);
  }

  if (isSecure(target)) {
    _response = _httpSecure.getString();
    _httpSecure.end();
    _wifiSecure.stop();
  } else {
    _response = _http.getString();
    _http.end();
    _wifi.stop();
  }

  tcp_cleanup();
  return _response;
}

// EOF
