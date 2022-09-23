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
#include <kegpush.hpp>
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

// Additional scale values
#define PARAM_SCALE_WEIGHT1 "scale-weight1"
#define PARAM_SCALE_WEIGHT2 "scale-weight2"
#define PARAM_BEER_WEIGHT1 "beer-weight1"
#define PARAM_BEER_WEIGHT2 "beer-weight2"
#define PARAM_BEER_VOLUME1 "beer-volume1"
#define PARAM_BEER_VOLUME2 "beer-volume2"
#define PARAM_SCALE_RAW1 "scale-raw1"
#define PARAM_SCALE_RAW2 "scale-raw2"
#define PARAM_GLASS1 "glass1"
#define PARAM_GLASS2 "glass2"
#define PARAM_SCALE_STABLE_WEIGHT1 "scale-stable-weight1"
#define PARAM_SCALE_STABLE_WEIGHT2 "scale-stable-weight2"
#define PARAM_LAST_POUR_WEIGHT1 "last-pour-weight1"
#define PARAM_LAST_POUR_WEIGHT2 "last-pour-weight2"
#define PARAM_LAST_POUR_VOLUME1 "last-pour-volume1"
#define PARAM_LAST_POUR_VOLUME2 "last-pour-volume2"

KegWebHandler::KegWebHandler(KegConfig* config) : BaseWebHandler(config) {
  _config = config;
}

void KegWebHandler::setupWebHandlers() {
  Log.notice(F("WEB : Setting up keg web handlers." CR));
  BaseWebHandler::setupWebHandlers();

  _server->serveStatic("/levels", LittleFS, LEVELS_FILENAME);
  _server->serveStatic("/levels2", LittleFS, LEVELS_FILENAME2);
  _server->on("/api/reset", HTTP_GET,
              std::bind(&KegWebHandler::webReset, this));
  _server->on("/api/scale", HTTP_GET,
              std::bind(&KegWebHandler::webScale, this));
  _server->on("/api/stability", HTTP_GET,
              std::bind(&KegWebHandler::webStability, this));
  _server->on("/api/stability/clear", HTTP_GET,
              std::bind(&KegWebHandler::webStabilityClear, this));
  _server->on("/api/scale/tare", HTTP_GET,
              std::bind(&KegWebHandler::webScaleTare, this));
  _server->on("/api/scale/factor", HTTP_GET,
              std::bind(&KegWebHandler::webScaleFactor, this));
  _server->on("/api/status", HTTP_GET,
              std::bind(&KegWebHandler::webStatus, this));
  _server->on("/calibration.htm", HTTP_GET,
              std::bind(&KegWebHandler::webCalibrateHtm, this));
  _server->on("/beer.htm", HTTP_GET,
              std::bind(&KegWebHandler::webBeerHtm, this));
  _server->on("/stability.htm", HTTP_GET,
              std::bind(&KegWebHandler::webStabilityHtm, this));
  _server->on("/graph.htm", HTTP_GET,
              std::bind(&KegWebHandler::webGraphHtm, this));
  _server->on("/api/brewspy/tap", HTTP_GET,
              std::bind(&KegWebHandler::webHandleBrewspy, this));
  _server->on("/api/beer", HTTP_POST,
              std::bind(&KegWebHandler::webHandleBeerWrite, this));
  _server->on("/api/logs/clear", HTTP_GET,
              std::bind(&KegWebHandler::webHandleLogsClear, this));
}

void KegWebHandler::webHandleLogsClear() {
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/logs/clear." CR));

  if (!id.compareTo(myConfig.getID())) {
    _server->send(200, "text/plain", "Removing logfiles...");
    LittleFS.remove(LEVELS_FILENAME);
    LittleFS.remove(LEVELS_FILENAME2);
    _server->send(200, "text/plain", "Level logfiles cleared.");
  } else {
    _server->send(400, "text/plain", "Unknown ID.");
  }
}

void KegWebHandler::webHandleBrewspy() {
  String token = _server->arg("token");
  Log.notice(F("WEB : webServer callback /api/brewspy/tap %s." CR),
             token.c_str());

  String json = myPush.requestTapInfoFromBrewspy(token);
  _server->send(200, "application/json", json.c_str());
}

void KegWebHandler::webScale() {
  Log.notice(F("WEB : webServer callback /api/scale." CR));

  DynamicJsonDocument doc(300);
  populateScaleJson(doc);

  doc[PARAM_WEIGHT_UNIT] = myConfig.getWeightUnit();
  doc[PARAM_VOLUME_UNIT] = myConfig.getVolumeUnit();

  String out;
  out.reserve(300);
  serializeJson(doc, out);
  doc.clear();
  _server->send(200, "application/json", out.c_str());
}

void KegWebHandler::webScaleTare() {
  UnitIndex idx;

  // Request will contain 1 or 2, but we need 0 or 1 for indexing.
  if (_server->arg(PARAM_SCALE).toInt() == 1)
    idx = UnitIndex::U1;
  else
    idx = UnitIndex::U2;

  Log.notice(F("WEB : webServer callback /api/scale/tare." CR));
  myScale.tare(idx);

  Log.notice(
      F("WEB : webServer callback /api/scale/factor, offset=%d [%d]." CR),
      myConfig.getScaleOffset(idx), idx);

  DynamicJsonDocument doc(300);
  populateScaleJson(doc);

  String out;
  out.reserve(300);
  serializeJson(doc, out);
  doc.clear();
  _server->send(200, "application/json", out.c_str());
}

void KegWebHandler::webScaleFactor() {
  float weight = convertIncomingWeight(_server->arg(PARAM_WEIGHT).toFloat());
  UnitIndex idx;

  // Request will contain 1 or 2, but we need 0 or 1 for indexing.
  if (_server->arg(PARAM_SCALE).toInt() == 1)
    idx = UnitIndex::U1;
  else
    idx = UnitIndex::U2;

  Log.notice(
      F("WEB : webServer callback /api/scale/factor, weight=%Fkg [%d]." CR),
      weight, idx);

  myScale.findFactor(idx, weight);

  Log.notice(
      F("WEB : webServer callback /api/scale/factor, factor=%F [%d]." CR),
      myConfig.getScaleFactor(idx), idx);

  DynamicJsonDocument doc(300);
  populateScaleJson(doc);

  doc[PARAM_WEIGHT_UNIT] = myConfig.getWeightUnit();
  doc[PARAM_VOLUME_UNIT] = myConfig.getVolumeUnit();

  String out;
  out.reserve(300);
  serializeJson(doc, out);
  doc.clear();
  _server->send(200, "application/json", out.c_str());
}

void KegWebHandler::populateScaleJson(DynamicJsonDocument& doc) {
  doc[PARAM_SCALE_FACTOR1] = myConfig.getScaleFactor(0);
  doc[PARAM_SCALE_FACTOR2] = myConfig.getScaleFactor(1);

  if (myScale.isConnected(UnitIndex::U1)) {
    doc[PARAM_SCALE_WEIGHT1] = reduceFloatPrecision(
        convertOutgoingWeight(myScale.getTotalStableWeight(UnitIndex::U1)),
        myConfig.getWeightPrecision());
    doc[PARAM_SCALE_RAW1] = myScale.readRaw(UnitIndex::U1);
    doc[PARAM_SCALE_OFFSET1] = myConfig.getScaleOffset(0);
    doc[PARAM_BEER_WEIGHT1] = reduceFloatPrecision(
        convertOutgoingWeight(myScale.getBeerWeight(UnitIndex::U1)),
        myConfig.getWeightPrecision());
    doc[PARAM_BEER_VOLUME1] = reduceFloatPrecision(
        convertOutgoingVolume(myScale.getBeerVolume(UnitIndex::U1)),
        myConfig.getVolumePrecision());
  }

  if (myScale.isConnected(UnitIndex::U2)) {
    doc[PARAM_SCALE_WEIGHT2] = reduceFloatPrecision(
        convertOutgoingWeight(myScale.getTotalStableWeight(UnitIndex::U2)),
        myConfig.getWeightPrecision());
    doc[PARAM_SCALE_RAW2] = myScale.readRaw(UnitIndex::U2);
    doc[PARAM_SCALE_OFFSET2] = myConfig.getScaleOffset(1);
    doc[PARAM_BEER_WEIGHT2] = reduceFloatPrecision(
        convertOutgoingWeight(myScale.getBeerWeight(UnitIndex::U2)),
        myConfig.getWeightPrecision());
    doc[PARAM_BEER_VOLUME2] = reduceFloatPrecision(
        convertOutgoingVolume(myScale.getBeerVolume(UnitIndex::U2)),
        myConfig.getVolumePrecision());
  }

  if (myScale.hasStableWeight(UnitIndex::U1)) {
    doc[PARAM_SCALE_STABLE_WEIGHT1] = reduceFloatPrecision(
        convertOutgoingWeight(myScale.getTotalStableWeight(UnitIndex::U1)),
        myConfig.getWeightPrecision());
  }

  if (myScale.hasStableWeight(UnitIndex::U2)) {
    doc[PARAM_SCALE_STABLE_WEIGHT2] = reduceFloatPrecision(
        convertOutgoingWeight(myScale.getTotalStableWeight(UnitIndex::U2)),
        myConfig.getWeightPrecision());
  }

  if (myScale.hasPourWeight(UnitIndex::U1)) {
    doc[PARAM_LAST_POUR_WEIGHT1] = reduceFloatPrecision(
        convertOutgoingWeight(myScale.getPourWeight(UnitIndex::U1)),
        myConfig.getWeightPrecision());
    doc[PARAM_LAST_POUR_VOLUME1] = reduceFloatPrecision(
        convertOutgoingVolume(myScale.getPourVolume(UnitIndex::U1)),
        myConfig.getVolumePrecision());
  }

  if (myScale.hasPourWeight(UnitIndex::U2)) {
    doc[PARAM_LAST_POUR_WEIGHT2] = reduceFloatPrecision(
        convertOutgoingWeight(myScale.getPourWeight(UnitIndex::U2)),
        myConfig.getWeightPrecision());
    doc[PARAM_LAST_POUR_VOLUME2] = reduceFloatPrecision(
        convertOutgoingVolume(myScale.getPourVolume(UnitIndex::U2)),
        myConfig.getVolumePrecision());
  }

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif
}

void KegWebHandler::webStatus() {
  Log.notice(F("WEB : webServer callback /api/status." CR));

  DynamicJsonDocument doc(1000);
  populateScaleJson(doc);

  doc[PARAM_MDNS] = myConfig.getMDNS();
  doc[PARAM_ID] = myConfig.getID();
  doc[PARAM_SSID] = myConfig.getWifiSSID(0);
  doc[PARAM_APP_VER] = CFG_APPVER;
  doc[PARAM_APP_BUILD] = CFG_GITREV;
  doc[PARAM_WEIGHT_UNIT] = myConfig.getWeightUnit();
  doc[PARAM_VOLUME_UNIT] = myConfig.getVolumeUnit();
  doc[PARAM_TEMP_FORMAT] = String(myConfig.getTempFormat());

  // For this we use the last value read from the scale to avoid having to much
  // communication. The value will be updated regulary second in the main loop.
  doc[PARAM_GLASS1] =
      reduceFloatPrecision(myScale.getNoStableGlasses(UnitIndex::U1), 1);
  doc[PARAM_GLASS2] =
      reduceFloatPrecision(myScale.getNoStableGlasses(UnitIndex::U2), 1);

  doc[PARAM_KEG_VOLUME1] = convertOutgoingVolume(myConfig.getKegVolume(0));
  doc[PARAM_KEG_VOLUME2] = convertOutgoingVolume(myConfig.getKegVolume(1));

  float f = myTemp.getTempC();

  if (!isnan(f)) {
    doc[PARAM_TEMP] = convertOutgoingTemperature(f);
    doc[PARAM_HUMIDITY] = myTemp.getHumidity();
  }

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  out.reserve(1000);
  serializeJson(doc, out);
  doc.clear();
  _server->send(200, "application/json", out.c_str());
}

void KegWebHandler::webStability() {
  Log.notice(F("WEB : webServer callback /api/stability." CR));

#define PARAM_STABILITY_COUNT1 "stability-count1"
#define PARAM_STABILITY_COUNT2 "stability-count2"
#define PARAM_STABILITY_SUM1 "stability-sum1"
#define PARAM_STABILITY_SUM2 "stability-sum2"
#define PARAM_STABILITY_MIN1 "stability-min1"
#define PARAM_STABILITY_MIN2 "stability-min2"
#define PARAM_STABILITY_MAX1 "stability-max1"
#define PARAM_STABILITY_MAX2 "stability-max2"
#define PARAM_STABILITY_AVE1 "stability-ave1"
#define PARAM_STABILITY_AVE2 "stability-ave2"
#define PARAM_STABILITY_VAR1 "stability-var1"
#define PARAM_STABILITY_VAR2 "stability-var2"
#define PARAM_STABILITY_POPDEV1 "stability-popdev1"
#define PARAM_STABILITY_POPDEV2 "stability-popdev2"
#define PARAM_STABILITY_UBIASDEV1 "stability-ubiasdev1"
#define PARAM_STABILITY_UBIASDEV2 "stability-ubiasdev2"

  DynamicJsonDocument doc(500);

  Stability* stability1 = myScale.getStability(UnitIndex::U1);
  Stability* stability2 = myScale.getStability(UnitIndex::U2);

  doc[PARAM_WEIGHT_UNIT] = myConfig.getWeightUnit();

  if (stability1->count() > 1) {
    doc[PARAM_STABILITY_COUNT1] = stability1->count();
    doc[PARAM_STABILITY_SUM1] = stability1->sum();
    doc[PARAM_STABILITY_MIN1] = stability1->min();
    doc[PARAM_STABILITY_MAX1] = stability1->max();
    doc[PARAM_STABILITY_AVE1] = stability1->average();
    doc[PARAM_STABILITY_VAR1] = stability1->variance();
    doc[PARAM_STABILITY_POPDEV1] = stability1->popStdev();
    doc[PARAM_STABILITY_UBIASDEV1] = stability1->unbiasedStdev();
  }

  if (stability2->count() > 1) {
    doc[PARAM_STABILITY_COUNT2] = stability2->count();
    doc[PARAM_STABILITY_SUM2] = stability2->sum();
    doc[PARAM_STABILITY_MIN2] = stability2->min();
    doc[PARAM_STABILITY_MAX2] = stability2->max();
    doc[PARAM_STABILITY_AVE2] = stability2->average();
    doc[PARAM_STABILITY_VAR2] = stability2->variance();
    doc[PARAM_STABILITY_POPDEV2] = stability2->popStdev();
    doc[PARAM_STABILITY_UBIASDEV2] = stability2->unbiasedStdev();
  }

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  String out;
  out.reserve(500);
  serializeJson(doc, out);
  doc.clear();
  _server->send(200, "application/json", out.c_str());
}

void KegWebHandler::webReset() {
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback /api/reset." CR));

  if (!id.compareTo(myConfig.getID())) {
    _server->send(200, "text/plain", "Performing reset...");
    LittleFS.end();
    delay(500);
    ESP_RESET();
  } else {
    _server->send(400, "text/plain", "Unknown ID.");
  }
}

void KegWebHandler::webStabilityClear() {
  Log.notice(F("WEB : webServer callback /api/stability/clear." CR));

  myScale.getStability(UnitIndex::U1)->clear();
  myScale.getStability(UnitIndex::U2)->clear();

  _server->send(200, "application/json", "{}");
}

void KegWebHandler::webHandleBeerWrite() {
  String id = _server->arg(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/beer." CR));

  if (!id.equalsIgnoreCase(_webConfig->getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              _webConfig->getID());
    _server->send(400, "text/plain", "Invalid ID.");
    return;
  }

  DynamicJsonDocument doc(2000);

  // Mapping post format to json for parsing in config class
  for (int i = 0; i < _server->args(); i++) {
    String arg = _server->argName(i);

    if (!arg.compareTo("plain") || !arg.compareTo(PARAM_ID) ||
        !arg.compareTo(PARAM_PASS) || !arg.compareTo(PARAM_PASS2) ||
        !arg.compareTo(PARAM_SSID) || !arg.compareTo(PARAM_SSID2)) {
      Log.verbose(F("WEB : Skipping param %ss" CR), arg.c_str());
    } else {
      Log.verbose(F("WEB : Adding param %s=%s" CR), arg.c_str(),
                  _server->arg(i).c_str());
      doc[arg] = _server->arg(i);
    }
  }

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  _webConfig->parseJson(doc);
  _webConfig->saveFile();

  String path = "/beer.htm";

  if (_server->hasArg("section")) path += _server->arg("section").c_str();

  _server->sendHeader("Location", path, true);
  _server->send(302, "text/plain", "Config saved");
}

// EOF
