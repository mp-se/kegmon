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
#include <levels.hpp>
#include <main.hpp>
#include <scale.hpp>
#include <temp_mgr.hpp>
#include <utils.hpp>

// Configuration or api params
constexpr auto PARAM_APP_VER = "app-ver";
constexpr auto PARAM_APP_BUILD = "app-build";
constexpr auto PARAM_PLATFORM = "platform";
constexpr auto PARAM_TEMP = "temperature";
constexpr auto PARAM_HUMIDITY = "humidity";

// Calibration input
constexpr auto PARAM_WEIGHT = "weight";
constexpr auto PARAM_SCALE = "scale-index";

// Additional scale values
constexpr auto PARAM_SCALE_WEIGHT1 = "scale-weight1";
constexpr auto PARAM_SCALE_WEIGHT2 = "scale-weight2";
constexpr auto PARAM_BEER_WEIGHT1 = "beer-weight1";
constexpr auto PARAM_BEER_WEIGHT2 = "beer-weight2";
constexpr auto PARAM_BEER_VOLUME1 = "beer-volume1";
constexpr auto PARAM_BEER_VOLUME2 = "beer-volume2";
constexpr auto PARAM_SCALE_RAW1 = "scale-raw1";
constexpr auto PARAM_SCALE_RAW2 = "scale-raw2";
constexpr auto PARAM_GLASS1 = "glass1";
constexpr auto PARAM_GLASS2 = "glass2";
constexpr auto PARAM_SCALE_STABLE_WEIGHT1 = "scale-stable-weight1";
constexpr auto PARAM_SCALE_STABLE_WEIGHT2 = "scale-stable-weight2";
constexpr auto PARAM_LAST_POUR_WEIGHT1 = "last-pour-weight1";
constexpr auto PARAM_LAST_POUR_WEIGHT2 = "last-pour-weight2";
constexpr auto PARAM_LAST_POUR_VOLUME1 = "last-pour-volume1";
constexpr auto PARAM_LAST_POUR_VOLUME2 = "last-pour-volume2";

#if defined(USE_ASYNC_WEB)
KegWebHandler::KegWebHandler(KegConfig* config)
    : BaseAsyncWebHandler(config, JSON_BUFFER) {
  _config = config;
}
#else
KegWebHandler::KegWebHandler(KegConfig* config)
    : BaseWebHandler(config, JSON_BUFFER) {
  _config = config;
}
#endif

void KegWebHandler::setupAsyncWebHandlers() { setupWebHandlers(); }

void KegWebHandler::setupWebHandlers() {
  Log.notice(F("WEB : Setting up keg web handlers." CR));

#if defined(USE_ASYNC_WEB)
  BaseAsyncWebHandler::setupAsyncWebHandlers();
#else
  BaseWebHandler::setupWebHandlers();
#endif

  // Note! For the async implementation the order matters
  _server->serveStatic("/levels2", LittleFS, LEVELS_FILENAME2);
  _server->serveStatic("/levels", LittleFS, LEVELS_FILENAME);
  _server->serveStatic("/startup", LittleFS, STARTUP_FILENAME);
  WS_BIND_URL("/api/reset", HTTP_GET, &KegWebHandler::webReset);
  WS_BIND_URL("/api/scale/tare", HTTP_GET, &KegWebHandler::webScaleTare);
  WS_BIND_URL("/api/scale/factor", HTTP_GET, &KegWebHandler::webScaleFactor);
  WS_BIND_URL("/api/scale", HTTP_GET, &KegWebHandler::webScale);
  WS_BIND_URL("/api/stability/clear", HTTP_GET,
              &KegWebHandler::webStabilityClear);
  WS_BIND_URL("/api/stability", HTTP_GET, &KegWebHandler::webStability);
  WS_BIND_URL("/api/status", HTTP_GET, &KegWebHandler::webStatus);
  WS_BIND_URL("/calibration.htm", HTTP_GET, &KegWebHandler::webCalibrateHtm);
  WS_BIND_URL("/beer.htm", HTTP_GET, &KegWebHandler::webBeerHtm);
  WS_BIND_URL("/stability.htm", HTTP_GET, &KegWebHandler::webStabilityHtm);
  WS_BIND_URL("/graph.htm", HTTP_GET, &KegWebHandler::webGraphHtm);
  WS_BIND_URL("/backup.htm", HTTP_GET, &KegWebHandler::webBackupHtm);
  WS_BIND_URL("/api/brewspy/tap", HTTP_GET, &KegWebHandler::webHandleBrewspy);
  WS_BIND_URL("/api/beer", HTTP_POST, &KegWebHandler::webHandleBeerWrite);
  WS_BIND_URL("/api/logs/clear", HTTP_GET, &KegWebHandler::webHandleLogsClear);
  WS_BIND_URL("/dashboard", HTTP_GET, &KegWebHandler::webDashboardHtm);
}

void KegWebHandler::webHandleLogsClear(WS_PARAM) {
  String id = WS_REQ_ARG(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/logs/clear." CR));

  if (!id.compareTo(myConfig.getID())) {
    // WS_SEND(200, "text/plain", "Removing logfiles...");
    LittleFS.remove(LEVELS_FILENAME);
    LittleFS.remove(LEVELS_FILENAME2);
    WS_SEND(200, "text/plain", "Level logfiles cleared.");
  } else {
    WS_SEND(400, "text/plain", "Unknown ID.");
  }
}

void KegWebHandler::webHandleBrewspy(WS_PARAM) {
  String token = WS_REQ_ARG("token");
  Log.notice(F("WEB : webServer callback /api/brewspy/tap %s." CR),
             token.c_str());

  String json = myPush.requestTapInfoFromBrewspy(token);
  WS_SEND(200, "application/json", json.c_str());
}

void KegWebHandler::webScale(WS_PARAM) {
  Log.notice(F("WEB : webServer callback /api/scale." CR));

  DynamicJsonDocument doc(500);
  populateScaleJson(doc);

  doc[PARAM_WEIGHT_UNIT] = myConfig.getWeightUnit();
  doc[PARAM_VOLUME_UNIT] = myConfig.getVolumeUnit();

  String out;
  out.reserve(500);
  serializeJson(doc, out);
  doc.clear();
  WS_SEND(200, "application/json", out.c_str());
}

void KegWebHandler::webScaleTare(WS_PARAM) {
  UnitIndex idx;

  // Request will contain 1 or 2, but we need 0 or 1 for indexing.
  if (WS_REQ_ARG(PARAM_SCALE).toInt() == 1)
    idx = UnitIndex::U1;
  else
    idx = UnitIndex::U2;

  Log.notice(F("WEB : webServer callback /api/scale/tare." CR));

  myScale.scheduleTare(idx);
  WS_SEND(200, "text/plain", "");
}

void KegWebHandler::webScaleFactor(WS_PARAM) {
  float weight = convertIncomingWeight(WS_REQ_ARG(PARAM_WEIGHT).toFloat());
  UnitIndex idx;

  // Request will contain 1 or 2, but we need 0 or 1 for indexing.
  if (WS_REQ_ARG(PARAM_SCALE).toInt() == 1)
    idx = UnitIndex::U1;
  else
    idx = UnitIndex::U2;

  Log.notice(
      F("WEB : webServer callback /api/scale/factor, weight=%Fkg [%d]." CR),
      weight, idx);

  myScale.scheduleFindFactor(idx, weight);
  WS_SEND(200, "text/plain", "");
}

void KegWebHandler::populateScaleJson(DynamicJsonDocument& doc) {
  // This will return the raw weight so that that we get the actual values.
  doc[PARAM_SCALE_FACTOR1] = myConfig.getScaleFactor(UnitIndex::U1);
  doc[PARAM_SCALE_FACTOR2] = myConfig.getScaleFactor(UnitIndex::U2);
  if (myScale.isConnected(UnitIndex::U1)) {
    doc[PARAM_SCALE_WEIGHT1] = reduceFloatPrecision(
        convertOutgoingWeight(
            myLevelDetection.getTotalRawWeight(UnitIndex::U1)),
        myConfig.getWeightPrecision());
    doc[PARAM_SCALE_RAW1] = myScale.readLastRaw(UnitIndex::U1);
    doc[PARAM_SCALE_OFFSET1] = myConfig.getScaleOffset(UnitIndex::U1);
    doc[PARAM_BEER_WEIGHT1] = reduceFloatPrecision(
        convertOutgoingWeight(myLevelDetection.getBeerWeight(UnitIndex::U1)),
        myConfig.getWeightPrecision());
    doc[PARAM_BEER_VOLUME1] = reduceFloatPrecision(
        convertOutgoingVolume(myLevelDetection.getBeerVolume(UnitIndex::U1)),
        myConfig.getVolumePrecision());
  }

  if (myScale.isConnected(UnitIndex::U2)) {
    doc[PARAM_SCALE_WEIGHT2] = reduceFloatPrecision(
        convertOutgoingWeight(
            myLevelDetection.getTotalRawWeight(UnitIndex::U2)),
        myConfig.getWeightPrecision());
    doc[PARAM_SCALE_RAW2] = myScale.readLastRaw(UnitIndex::U2);
    doc[PARAM_SCALE_OFFSET2] = myConfig.getScaleOffset(UnitIndex::U2);
    doc[PARAM_BEER_WEIGHT2] = reduceFloatPrecision(
        convertOutgoingWeight(myLevelDetection.getBeerWeight(UnitIndex::U2)),
        myConfig.getWeightPrecision());
    doc[PARAM_BEER_VOLUME2] = reduceFloatPrecision(
        convertOutgoingVolume(myLevelDetection.getBeerVolume(UnitIndex::U2)),
        myConfig.getVolumePrecision());
  }

  if (myLevelDetection.hasStableWeight(UnitIndex::U1)) {
    doc[PARAM_SCALE_STABLE_WEIGHT1] = reduceFloatPrecision(
        convertOutgoingWeight(
            myLevelDetection.getTotalStableWeight(UnitIndex::U1)),
        myConfig.getWeightPrecision());
  }

  if (myLevelDetection.hasStableWeight(UnitIndex::U2)) {
    doc[PARAM_SCALE_STABLE_WEIGHT2] = reduceFloatPrecision(
        convertOutgoingWeight(
            myLevelDetection.getTotalStableWeight(UnitIndex::U2)),
        myConfig.getWeightPrecision());
  }

  if (myLevelDetection.hasPourWeight(UnitIndex::U1)) {
    doc[PARAM_LAST_POUR_WEIGHT1] = reduceFloatPrecision(
        convertOutgoingWeight(myLevelDetection.getPourWeight(UnitIndex::U1)),
        myConfig.getWeightPrecision());
    doc[PARAM_LAST_POUR_VOLUME1] = reduceFloatPrecision(
        convertOutgoingVolume(myLevelDetection.getPourVolume(UnitIndex::U1)),
        myConfig.getVolumePrecision());
  }

  if (myLevelDetection.hasPourWeight(UnitIndex::U2)) {
    doc[PARAM_LAST_POUR_WEIGHT2] = reduceFloatPrecision(
        convertOutgoingWeight(myLevelDetection.getPourWeight(UnitIndex::U2)),
        myConfig.getWeightPrecision());
    doc[PARAM_LAST_POUR_VOLUME2] = reduceFloatPrecision(
        convertOutgoingVolume(myLevelDetection.getPourVolume(UnitIndex::U2)),
        myConfig.getVolumePrecision());
  }

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  EspSerial.print(CR);
#endif
}

void KegWebHandler::webStatus(WS_PARAM) {
  Log.notice(F("WEB : webServer callback /api/status." CR));

  DynamicJsonDocument doc(1000);
  populateScaleJson(doc);

  doc[PARAM_MDNS] = myConfig.getMDNS();
  doc[PARAM_ID] = myConfig.getID();
  doc[PARAM_SSID] = myConfig.getWifiSSID(0);
#if defined(ESP8266) && defined(USE_ASYNC_WEB)
  doc[PARAM_PLATFORM] = "esp8266 async";
#elif defined(ESP8266)
  doc[PARAM_PLATFORM] = "esp8266";
#elif defined(ESP32S2) && defined(USE_ASYNC_WEB)
  doc[PARAM_PLATFORM] = "esp32s2 async";
#elif defined(ESP32S2)
  doc[PARAM_PLATFORM] = "esp32s2";
#endif
  doc[PARAM_APP_VER] = CFG_APPVER;
  doc[PARAM_APP_BUILD] = CFG_GITREV;
  doc[PARAM_WEIGHT_UNIT] = myConfig.getWeightUnit();
  doc[PARAM_VOLUME_UNIT] = myConfig.getVolumeUnit();
  doc[PARAM_TEMP_FORMAT] = String(myConfig.getTempFormat());

  // For this we use the last value read from the scale to avoid having to much
  // communication. The value will be updated regulary second in the main loop.
  doc[PARAM_GLASS1] = reduceFloatPrecision(
      myLevelDetection.getNoStableGlasses(UnitIndex::U1), 1);
  doc[PARAM_GLASS2] = reduceFloatPrecision(
      myLevelDetection.getNoStableGlasses(UnitIndex::U2), 1);

  doc[PARAM_KEG_VOLUME1] =
      convertOutgoingVolume(myConfig.getKegVolume(UnitIndex::U1));
  doc[PARAM_KEG_VOLUME2] =
      convertOutgoingVolume(myConfig.getKegVolume(UnitIndex::U2));

  float f = myTemp.getLastTempC();

  if (!isnan(f)) {
    doc[PARAM_TEMP] = convertOutgoingTemperature(f);
  }

  float h = myTemp.getLastHumidity();

  if (!isnan(h)) {
    doc[PARAM_HUMIDITY] = h;
  }

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  EspSerial.print(CR);
#endif

  String out;
  out.reserve(1000);
  serializeJson(doc, out);
  doc.clear();
  WS_SEND(200, "application/json", out.c_str());
}

void KegWebHandler::webStability(WS_PARAM) {
  Log.notice(F("WEB : webServer callback /api/stability." CR));

  constexpr auto PARAM_STABILITY_COUNT1 = "stability-count1";
  constexpr auto PARAM_STABILITY_COUNT2 = "stability-count2";
  constexpr auto PARAM_STABILITY_SUM1 = "stability-sum1";
  constexpr auto PARAM_STABILITY_SUM2 = "stability-sum2";
  constexpr auto PARAM_STABILITY_MIN1 = "stability-min1";
  constexpr auto PARAM_STABILITY_MIN2 = "stability-min2";
  constexpr auto PARAM_STABILITY_MAX1 = "stability-max1";
  constexpr auto PARAM_STABILITY_MAX2 = "stability-max2";
  constexpr auto PARAM_STABILITY_AVE1 = "stability-ave1";
  constexpr auto PARAM_STABILITY_AVE2 = "stability-ave2";
  constexpr auto PARAM_STABILITY_VAR1 = "stability-var1";
  constexpr auto PARAM_STABILITY_VAR2 = "stability-var2";
  constexpr auto PARAM_STABILITY_POPDEV1 = "stability-popdev1";
  constexpr auto PARAM_STABILITY_POPDEV2 = "stability-popdev2";
  constexpr auto PARAM_STABILITY_UBIASDEV1 = "stability-ubiasdev1";
  constexpr auto PARAM_STABILITY_UBIASDEV2 = "stability-ubiasdev2";

  DynamicJsonDocument doc(800);

  Stability* stability1 = myLevelDetection.getStability(UnitIndex::U1);
  Stability* stability2 = myLevelDetection.getStability(UnitIndex::U2);

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

  constexpr auto PARAM_LEVEL_RAW1 = "level-raw1";
  constexpr auto PARAM_LEVEL_RAW2 = "level-raw2";
  constexpr auto PARAM_LEVEL_KALMAN1 = "level-kalman1";
  constexpr auto PARAM_LEVEL_KALMAN2 = "level-kalman2";
  constexpr auto PARAM_LEVEL_STATISTIC1 = "level-stable1";
  constexpr auto PARAM_LEVEL_STATISTIC2 = "level-stable2";

  if (myLevelDetection.getRawDetection(UnitIndex::U1)->hasRawValue())
    doc[PARAM_LEVEL_RAW1] =
        myLevelDetection.getRawDetection(UnitIndex::U1)->getRawValue();
  if (myLevelDetection.getRawDetection(UnitIndex::U1)->hasKalmanValue())
    doc[PARAM_LEVEL_KALMAN1] =
        myLevelDetection.getRawDetection(UnitIndex::U1)->getKalmanValue();
  if (myLevelDetection.getStatsDetection(UnitIndex::U1)->hasStableValue())
    doc[PARAM_LEVEL_STATISTIC1] =
        myLevelDetection.getStatsDetection(UnitIndex::U1)->getStableValue();

  if (myLevelDetection.getRawDetection(UnitIndex::U2)->hasRawValue())
    doc[PARAM_LEVEL_RAW2] =
        myLevelDetection.getRawDetection(UnitIndex::U2)->getRawValue();
  if (myLevelDetection.getRawDetection(UnitIndex::U2)->hasKalmanValue())
    doc[PARAM_LEVEL_KALMAN2] =
        myLevelDetection.getRawDetection(UnitIndex::U2)->getKalmanValue();
  if (myLevelDetection.getStatsDetection(UnitIndex::U2)->hasStableValue())
    doc[PARAM_LEVEL_STATISTIC2] =
        myLevelDetection.getStatsDetection(UnitIndex::U2)->getStableValue();

  float f = myTemp.getLastTempC();

  if (!isnan(f)) {
    doc[PARAM_TEMP] = convertOutgoingTemperature(f);
  }

  float h = myTemp.getLastHumidity();

  if (!isnan(h)) {
    doc[PARAM_HUMIDITY] = h;
  }

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  EspSerial.print(CR);
#endif

  String out;
  out.reserve(800);
  serializeJson(doc, out);
  doc.clear();
  WS_SEND(200, "application/json", out.c_str());
}

void KegWebHandler::webReset(WS_PARAM) {
  String id = WS_REQ_ARG(PARAM_ID);
  Log.notice(F("WEB : webServer callback /api/reset." CR));

  if (!id.compareTo(myConfig.getID())) {
    WS_SEND(200, "text/plain", "Performing reset...");
    LittleFS.end();
    delay(500);
    ESP_RESET();
  } else {
    WS_SEND(400, "text/plain", "Unknown ID.");
  }
}

void KegWebHandler::webStabilityClear(WS_PARAM) {
  Log.notice(F("WEB : webServer callback /api/stability/clear." CR));

  myLevelDetection.getStability(UnitIndex::U1)->clear();
  myLevelDetection.getStability(UnitIndex::U2)->clear();
  WS_SEND(200, "application/json", "{}");
}

void KegWebHandler::webHandleBeerWrite(WS_PARAM) {
  String id = WS_REQ_ARG(PARAM_ID);
  Log.notice(F("WEB : webServer callback for /api/beer." CR));

  if (!id.equalsIgnoreCase(_webConfig->getID())) {
    Log.error(F("WEB : Wrong ID received %s, expected %s" CR), id.c_str(),
              _webConfig->getID());
    WS_SEND(400, "text/plain", "Invalid ID.");
    return;
  }

  DynamicJsonDocument doc(2000);

  // Mapping post format to json for parsing in config class
  for (int i = 0; i < WS_REQ_ARG_CNT(); i++) {
    String arg = WS_REQ_ARG_NAME(i);

    if (!arg.compareTo("plain") || !arg.compareTo(PARAM_ID) ||
        !arg.compareTo(PARAM_PASS) || !arg.compareTo(PARAM_PASS2) ||
        !arg.compareTo(PARAM_SSID) || !arg.compareTo(PARAM_SSID2)) {
      Log.verbose(F("WEB : Skipping param %ss" CR), arg.c_str());
    } else {
      Log.verbose(F("WEB : Adding param %s=%s" CR), arg.c_str(),
                  WS_REQ_ARG(i).c_str());
      doc[arg] = WS_REQ_ARG(i);
    }
  }

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  EspSerial.print(CR);
#endif

  _webConfig->parseJson(doc);
  _webConfig->saveFile();

  String path = "/beer.htm";

  if (WS_REQ_HAS_ARG("section")) path += WS_REQ_ARG("section").c_str();

#if defined(USE_ASYNC_WEB)
  request->redirect(path);
#else
  _server->sendHeader("Location", path, true);
  WS_SEND(302, "text/plain", "Config saved");
#endif
}

// EOF
