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
#include <kegwebhandler.hpp>
#include <main.hpp>
#include <scale.hpp>
#include <temp.hpp>
#include <utils.hpp>

// Configuration or api params
#define PARAM_APP_VER "app-ver"
#define PARAM_APP_BUILD "app-build"
#define PARAM_TEMP "temperature"
#define PARAM_HUMIDITY "humidity"

// Calibration input
#define PARAM_WEIGHT "weight"
#define PARAM_SCALE "scale-index"

KegWebHandler::KegWebHandler(KegConfig* config) : BaseWebHandler(config) {
  _config = config;
};

void KegWebHandler::setupWebHandlers() {
  Log.notice(F("WEB : Setting up keg web handlers." CR));
  BaseWebHandler::setupWebHandlers();

  _server->on("/api/scale", HTTP_GET,
              std::bind(&KegWebHandler::webScale, this));
  _server->on("/api/scale/tare", HTTP_GET,
              std::bind(&KegWebHandler::webScaleTare, this));
  _server->on("/api/scale/factor", HTTP_GET,
              std::bind(&KegWebHandler::webScaleFactor, this));
  _server->on("/api/status", HTTP_GET,
              std::bind(&KegWebHandler::webStatus, this));
  _server->on("/calibration.htm", HTTP_GET,
              std::bind(&KegWebHandler::webCalibrateHtm, this));
}

void KegWebHandler::webScale() {
  Log.notice(F("WEB : webServer callback /api/scale." CR));

  DynamicJsonDocument doc(256);
  populateScaleJson(doc);

  String out;
  out.reserve(256);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

void KegWebHandler::webScaleTare() {
  UnitIndex idx;

  // Request will contain 1 or 2, but we need 0 or 1 for indexing.
  if (_server->arg(PARAM_SCALE).toInt() == 1)
    idx = UnitIndex::UNIT_1;
  else  
    idx = UnitIndex::UNIT_2;

  Log.notice(F("WEB : webServer callback /api/scale/tare." CR));
  myScale.tare(idx);

  Log.notice(
      F("WEB : webServer callback /api/scale/factor, offset=%d [%d]." CR),
      myConfig.getScaleOffset(idx), idx);

  DynamicJsonDocument doc(256);
  populateScaleJson(doc);

  String out;
  out.reserve(256);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

void KegWebHandler::webScaleFactor() {
  float weight = _server->arg(PARAM_WEIGHT).toFloat();
  UnitIndex idx;

  // Request will contain 1 or 2, but we need 0 or 1 for indexing.
  if (_server->arg(PARAM_SCALE).toInt() == 1)
    idx = UnitIndex::UNIT_1;
  else  
    idx = UnitIndex::UNIT_2;

  Log.notice(
      F("WEB : webServer callback /api/scale/factor, weight=%F [%d]." CR),
      weight, idx);

  myScale.findFactor(idx, weight);

  Log.notice(
      F("WEB : webServer callback /api/scale/factor, factor=%F [%d]." CR),
      myConfig.getScaleFactor(idx), idx);

  DynamicJsonDocument doc(256);
  populateScaleJson(doc);

  String out;
  out.reserve(256);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

void KegWebHandler::populateScaleJson(DynamicJsonDocument& doc) {
  doc[PARAM_SCALE_FACTOR1] = myConfig.getScaleFactor(0);
  doc[PARAM_SCALE_FACTOR2] = myConfig.getScaleFactor(1);

  if (myScale.isConnected(UnitIndex::UNIT_1)) {
    doc[PARAM_SCALE_WEIGHT1] = reduceFloatPrecision(
        myScale.getLastValue(UnitIndex::UNIT_1), myConfig.getWeightPrecision());
    doc[PARAM_SCALE_RAW1] = myScale.getRawValue(UnitIndex::UNIT_1);
    doc[PARAM_SCALE_OFFSET1] = myConfig.getScaleOffset(0);
  }

  if (myScale.isConnected(UnitIndex::UNIT_2)) {
    doc[PARAM_SCALE_WEIGHT2] = reduceFloatPrecision(
        myScale.getLastValue(UnitIndex::UNIT_2), myConfig.getWeightPrecision());
    doc[PARAM_SCALE_RAW2] = myScale.getRawValue(UnitIndex::UNIT_2);
    doc[PARAM_SCALE_OFFSET2] = myConfig.getScaleOffset(1);
  }

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif
}

void KegWebHandler::webStatus() {
  Log.notice(F("WEB : webServer callback /api/status." CR));

  DynamicJsonDocument doc(1024);
  populateScaleJson(doc);

  doc[PARAM_MDNS] = myConfig.getMDNS();
  doc[PARAM_ID] = myConfig.getID();
  doc[PARAM_SSID] = myConfig.getWifiSSID(0);
  doc[PARAM_APP_VER] = CFG_APPVER;
  doc[PARAM_APP_BUILD] = CFG_GITREV;

  // For this we use the last value read from the scale to avoid having to much
  // communication. The value will be updated regulary second in the main loop.
  doc[PARAM_PINTS1] = myScale.calculateNoPints(
      UnitIndex::UNIT_1, myScale.getLastValue(UnitIndex::UNIT_1));
  doc[PARAM_PINTS2] = myScale.calculateNoPints(
      UnitIndex::UNIT_2, myScale.getLastValue(UnitIndex::UNIT_2));

  float f = myTemp.getTempValueC();

  if (!isnan(f)) {
    doc[PARAM_TEMP] = f;
    doc[PARAM_HUMIDITY] = myTemp.getHumidityValue();
  }

  String out;
  out.reserve(1024);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

// EOF
