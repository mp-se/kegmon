/*
MIT License

Copyright (c) 2021-2024 Magnus

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
#include <OneWire.h>

#include <kegpush.hpp>
#include <kegwebhandler.hpp>
#include <levels.hpp>
#include <main.hpp>
#include <scale.hpp>
#include <temp_mgr.hpp>
#include <uptime.hpp>
#include <utils.hpp>

// Configuration or api params
constexpr auto PARAM_APP_VER = "app_ver";
constexpr auto PARAM_APP_BUILD = "app_build";
constexpr auto PARAM_PLATFORM = "platform";
constexpr auto PARAM_TEMP = "temperature";
constexpr auto PARAM_HUMIDITY = "humidity";
constexpr auto PARAM_PRESSURE = "pressure";

// Calibration input
constexpr auto PARAM_WEIGHT = "weight";
constexpr auto PARAM_SCALE = "scale_index";

// Additional scale values
constexpr auto PARAM_SCALE_BUSY = "scale_busy";
constexpr auto PARAM_SCALE_WEIGHT1 = "scale_weight1";
constexpr auto PARAM_SCALE_WEIGHT2 = "scale_weight2";
constexpr auto PARAM_BEER_WEIGHT1 = "beer_weight1";
constexpr auto PARAM_BEER_WEIGHT2 = "beer_weight2";
constexpr auto PARAM_BEER_VOLUME1 = "beer_volume1";
constexpr auto PARAM_BEER_VOLUME2 = "beer_volume2";
constexpr auto PARAM_SCALE_RAW1 = "scale_raw1";
constexpr auto PARAM_SCALE_RAW2 = "scale_raw2";
constexpr auto PARAM_GLASS1 = "glass1";
constexpr auto PARAM_GLASS2 = "glass2";
constexpr auto PARAM_SCALE_STABLE_WEIGHT1 = "scale_stable_weight1";
constexpr auto PARAM_SCALE_STABLE_WEIGHT2 = "scale_stable_weight2";
constexpr auto PARAM_LAST_POUR_WEIGHT1 = "last_pour_weight1";
constexpr auto PARAM_LAST_POUR_WEIGHT2 = "last_pour_weight2";
constexpr auto PARAM_LAST_POUR_VOLUME1 = "last_pour_volume1";
constexpr auto PARAM_LAST_POUR_VOLUME2 = "last_pour_volume2";
constexpr auto PARAM_SCALE_CONNECTED1 = "scale_connected1";
constexpr auto PARAM_SCALE_CONNECTED2 = "scale_connected2";

// Other values
constexpr auto PARAM_TOTAL_HEAP = "total_heap";
constexpr auto PARAM_FREE_HEAP = "free_heap";
constexpr auto PARAM_IP = "ip";
constexpr auto PARAM_I2C = "i2c";
constexpr auto PARAM_I2C_1 = "bus_1";
constexpr auto PARAM_I2C_2 = "bus_2";
// constexpr auto PARAM_ONEWIRE = "onewire";
// constexpr auto PARAM_RESOLUTION = "resolution";
constexpr auto PARAM_ADRESS = "adress";
constexpr auto PARAM_FAMILY = "family";
constexpr auto PARAM_CHIP = "chip";
constexpr auto PARAM_REVISION = "revision";
constexpr auto PARAM_CORES = "cores";
constexpr auto PARAM_FEATURES = "features";
constexpr auto PARAM_WIFI_SETUP = "wifi_setup";
constexpr auto PARAM_ONEWIRE = "onewire";
constexpr auto PARAM_RESOLUTION = "resolution";

constexpr auto PARAM_UPTIME_SECONDS = "uptime_seconds";
constexpr auto PARAM_UPTIME_MINUTES = "uptime_minutes";
constexpr auto PARAM_UPTIME_HOURS = "uptime_hours";
constexpr auto PARAM_UPTIME_DAYS = "uptime_days";

// Push status
constexpr auto PARAM_HOMEASSISTANT = "ha";
constexpr auto PARAM_BARHELPER = "barhelper";
constexpr auto PARAM_BREWSPY = "brewspy";
constexpr auto PARAM_BREWLOGGER = "brewlogger";
constexpr auto PARAM_PUSH_USED = "push_used";
constexpr auto PARAM_PUSH_AGE = "push_age";
constexpr auto PARAM_PUSH_STATUS = "push_status";
constexpr auto PARAM_PUSH_CODE = "push_code";
constexpr auto PARAM_PUSH_RESPONSE = "push_response";

KegWebHandler::KegWebHandler(KegConfig *config)
    : BaseWebServer(config, JSON_BUFFER) {
  _config = config;
}

void KegWebHandler::setupWebHandlers() {
  Log.notice(F("WEB : Setting up keg web handlers." CR));

  BaseWebServer::setupWebHandlers();

  MDNS.addService("kegmon", "tcp", 80);

  // Note! For the async implementation the order matters
  _server->serveStatic("/levels2", LittleFS, LEVELS_FILENAME2);
  _server->serveStatic("/levels", LittleFS, LEVELS_FILENAME);
  _server->serveStatic("/startup", LittleFS, STARTUP_FILENAME);

  AsyncCallbackJsonWebHandler *handler;
  handler = new AsyncCallbackJsonWebHandler(
      "/api/scale/tare",
      std::bind(&KegWebHandler::webScaleTare, this, std::placeholders::_1,
                std::placeholders::_2),
      JSON_BUFFER_SIZE_S);
  _server->addHandler(handler);
  handler = new AsyncCallbackJsonWebHandler(
      "/api/scale/factor",
      std::bind(&KegWebHandler::webScaleFactor, this, std::placeholders::_1,
                std::placeholders::_2),
      JSON_BUFFER_SIZE_S);
  _server->addHandler(handler);
  _server->on("/api/scale", HTTP_GET,
              std::bind(&KegWebHandler::webScale, this, std::placeholders::_1));
  handler = new AsyncCallbackJsonWebHandler(
      "/api/config",
      std::bind(&KegWebHandler::webConfigPost, this, std::placeholders::_1,
                std::placeholders::_2),
      JSON_BUFFER_SIZE_L);
  _server->addHandler(handler);
  _server->on(
      "/api/config", HTTP_GET,
      std::bind(&KegWebHandler::webConfigGet, this, std::placeholders::_1));
  _server->on("/api/stability/clear", HTTP_GET,
              std::bind(&KegWebHandler::webStabilityClear, this,
                        std::placeholders::_1));
  _server->on(
      "/api/stability", HTTP_GET,
      std::bind(&KegWebHandler::webStability, this, std::placeholders::_1));
  _server->on(
      "/api/status", HTTP_GET,
      std::bind(&KegWebHandler::webStatus, this, std::placeholders::_1));
  handler = new AsyncCallbackJsonWebHandler(
      "/api/brewspy/tap",
      std::bind(&KegWebHandler::webHandleBrewspy, this, std::placeholders::_1,
                std::placeholders::_2),
      JSON_BUFFER_SIZE_S);
  _server->addHandler(handler);
  _server->on("/api/logs/clear", HTTP_GET,
              std::bind(&KegWebHandler::webHandleLogsClear, this,
                        std::placeholders::_1));
  _server->on("/api/hardware/status", HTTP_GET,
              std::bind(&KegWebHandler::webHardwareScanStatus, this,
                        std::placeholders::_1));
  _server->on(
      "/api/hardware", HTTP_GET,
      std::bind(&KegWebHandler::webHardwareScan, this, std::placeholders::_1));
  _server->on("/api/factory", HTTP_GET,
              std::bind(&KegWebHandler::webHandleFactoryDefaults, this,
                        std::placeholders::_1));
}

void KegWebHandler::webConfigGet(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/config(read)." CR));
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_XL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  myConfig.createJson(obj);
  response->setLength();
  request->send(response);
}

void KegWebHandler::webHandleFactoryDefaults(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/factory." CR));
  myConfig.saveFileWifiOnly();
  LittleFS.remove(ERR_FILENAME);
  LittleFS.remove(LEVELS_FILENAME);
  LittleFS.remove(LEVELS_FILENAME2);
  LittleFS.end();
  Log.notice(F("WEB : Deleted files in filesystem, rebooting." CR));

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Factory reset completed, rebooting";
  response->setLength();
  request->send(response);
  delay(500);
  ESP_RESET();
}

void KegWebHandler::webConfigPost(AsyncWebServerRequest *request,
                                  JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/config(write)." CR));
  JsonObject obj = json.as<JsonObject>();

  /*for (JsonPair kv : obj) {
    Log.notice(F("WEB : %s=%s." CR), kv.key().c_str(),
  kv.value().as<String>().c_str());
  }*/

  myConfig.parseJson(obj);
  obj.clear();
  myConfig.saveFile();

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Configuration updated";
  response->setLength();
  request->send(response);
}

void KegWebHandler::webHandleLogsClear(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/logs/clear." CR));
  LittleFS.remove(LEVELS_FILENAME);
  LittleFS.remove(LEVELS_FILENAME2);

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Logfiles removed";
  response->setLength();
  request->send(response);
}

void KegWebHandler::webHandleBrewspy(AsyncWebServerRequest *request,
                                     JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  JsonObject obj = json.as<JsonObject>();
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_M);
  JsonObject obj2 = response->getRoot().as<JsonObject>();
  myPush.requestTapInfoFromBrewspy(obj2, obj[PARAM_TOKEN]);
  response->setLength();
  request->send(response);
}

void KegWebHandler::webScale(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback /api/scale." CR));

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_L);
  JsonObject obj = response->getRoot().as<JsonObject>();
  populateScaleJson(obj);
  obj[PARAM_WEIGHT_UNIT] = myConfig.getWeightUnit();
  obj[PARAM_VOLUME_UNIT] = myConfig.getVolumeUnit();
  response->setLength();
  request->send(response);
}

void KegWebHandler::webScaleTare(AsyncWebServerRequest *request,
                                 JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  JsonObject obj = json.as<JsonObject>();
  UnitIndex idx;

  // Request will contain 1 or 2, but we need 0 or 1 for indexing.
  if (obj[PARAM_SCALE].as<int>() == 1)
    idx = UnitIndex::U1;
  else
    idx = UnitIndex::U2;

  Log.notice(F("WEB : webServer callback /api/scale/tare." CR));

  myScale.scheduleTare(idx);

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  JsonObject obj2 = response->getRoot().as<JsonObject>();
  obj2[PARAM_SUCCESS] = true;
  obj2[PARAM_MESSAGE] = "Scale tare schedule";
  response->setLength();
  request->send(response);
}

void KegWebHandler::webScaleFactor(AsyncWebServerRequest *request,
                                   JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  JsonObject obj = json.as<JsonObject>();
  UnitIndex idx;
  float weight = convertIncomingWeight(obj[PARAM_WEIGHT].as<float>());

  // Request will contain 1 or 2, but we need 0 or 1 for indexing.
  if (obj[PARAM_SCALE].as<int>() == 1)
    idx = UnitIndex::U1;
  else
    idx = UnitIndex::U2;

  Log.notice(
      F("WEB : webServer callback /api/scale/factor, weight=%Fkg [%d]." CR),
      weight, idx);

  myScale.scheduleFindFactor(idx, weight);

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  JsonObject obj2 = response->getRoot().as<JsonObject>();
  obj2[PARAM_SUCCESS] = true;
  obj2[PARAM_MESSAGE] = "Scale tare is scheduled";
  response->setLength();
  request->send(response);
}

void KegWebHandler::populateScaleJson(JsonObject &doc) {
  // This will return the raw weight so that that we get the actual values.
  doc[PARAM_SCALE_BUSY] = myScale.isScheduleRunning();

  doc[PARAM_SCALE_CONNECTED1] = myScale.isConnected(UnitIndex::U1);
  doc[PARAM_SCALE_CONNECTED2] = myScale.isConnected(UnitIndex::U2);

  doc[PARAM_SCALE_FACTOR1] = myConfig.getScaleFactor(UnitIndex::U1);
  doc[PARAM_SCALE_FACTOR2] = myConfig.getScaleFactor(UnitIndex::U2);
  if (myScale.isConnected(UnitIndex::U1)) {
    float w = myLevelDetection.getTotalRawWeight(UnitIndex::U1);
    if (!isnan(w)) {
      doc[PARAM_SCALE_WEIGHT1] = serialized(
          String(convertOutgoingWeight(w), myConfig.getWeightPrecision()));
    }
    doc[PARAM_SCALE_RAW1] = myScale.readLastRaw(UnitIndex::U1);
    doc[PARAM_SCALE_OFFSET1] = myConfig.getScaleOffset(UnitIndex::U1);

    w = myLevelDetection.getBeerWeight(UnitIndex::U1);
    if (!isnan(w)) {
      doc[PARAM_BEER_WEIGHT1] = serialized(
          String(convertOutgoingWeight(w), myConfig.getWeightPrecision()));
    }
    doc[PARAM_BEER_VOLUME1] = serialized(String(
        convertOutgoingVolume(myLevelDetection.getBeerVolume(UnitIndex::U1)),
        myConfig.getVolumePrecision()));
  }

  if (myScale.isConnected(UnitIndex::U2)) {
    float w = myLevelDetection.getTotalRawWeight(UnitIndex::U2);
    if (!isnan(w)) {
      doc[PARAM_SCALE_WEIGHT2] = serialized(
          String(convertOutgoingWeight(w), myConfig.getWeightPrecision()));
    }
    doc[PARAM_SCALE_RAW2] = myScale.readLastRaw(UnitIndex::U2);
    doc[PARAM_SCALE_OFFSET2] = myConfig.getScaleOffset(UnitIndex::U2);

    w = myLevelDetection.getBeerWeight(UnitIndex::U2);
    if (!isnan(w)) {
      doc[PARAM_BEER_WEIGHT2] = serialized(
          String(convertOutgoingWeight(w), myConfig.getWeightPrecision()));
    }
    doc[PARAM_BEER_VOLUME2] = serialized(String(
        convertOutgoingVolume(myLevelDetection.getBeerVolume(UnitIndex::U2)),
        myConfig.getVolumePrecision()));
  }

  if (myLevelDetection.hasStableWeight(UnitIndex::U1)) {
    doc[PARAM_SCALE_STABLE_WEIGHT1] = serialized(
        String(convertOutgoingWeight(
                   myLevelDetection.getTotalStableWeight(UnitIndex::U1)),
               myConfig.getWeightPrecision()));
  }

  if (myLevelDetection.hasStableWeight(UnitIndex::U2)) {
    doc[PARAM_SCALE_STABLE_WEIGHT2] = serialized(
        String(convertOutgoingWeight(
                   myLevelDetection.getTotalStableWeight(UnitIndex::U2)),
               myConfig.getWeightPrecision()));
  }

  if (myLevelDetection.hasPourWeight(UnitIndex::U1)) {
    doc[PARAM_LAST_POUR_WEIGHT1] = serialized(String(
        convertOutgoingWeight(myLevelDetection.getPourWeight(UnitIndex::U1)),
        myConfig.getWeightPrecision()));
    doc[PARAM_LAST_POUR_VOLUME1] = serialized(String(
        convertOutgoingVolume(myLevelDetection.getPourVolume(UnitIndex::U1)),
        myConfig.getVolumePrecision()));
  }

  if (myLevelDetection.hasPourWeight(UnitIndex::U2)) {
    doc[PARAM_LAST_POUR_WEIGHT2] = serialized(String(
        convertOutgoingWeight(myLevelDetection.getPourWeight(UnitIndex::U2)),
        myConfig.getWeightPrecision()));
    doc[PARAM_LAST_POUR_VOLUME2] = serialized(String(
        convertOutgoingVolume(myLevelDetection.getPourVolume(UnitIndex::U2)),
        myConfig.getVolumePrecision()));
  }

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  EspSerial.print(CR);
#endif
}

void KegWebHandler::webStatus(AsyncWebServerRequest *request) {
  Log.notice(F("WEB : webServer callback /api/status." CR));

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_XL);
  JsonObject obj = response->getRoot().as<JsonObject>();
  populateScaleJson(obj);
  obj[PARAM_MDNS] = myConfig.getMDNS();
  obj[PARAM_ID] = myConfig.getID();
  obj[PARAM_RSSI] = WiFi.RSSI();
  obj[PARAM_SSID] = myConfig.getWifiSSID(0);
#if defined(ESP8266)
  obj[PARAM_PLATFORM] = "esp8266";
#elif defined(ESP32S2)
  obj[PARAM_PLATFORM] = "esp32s2";
#elif defined(ESP32S3)
  obj[PARAM_PLATFORM] = "esp32s3";
#else
#error "Undefined target"
#endif
  obj[PARAM_APP_VER] = CFG_APPVER;
  obj[PARAM_APP_BUILD] = CFG_GITREV;
  obj[PARAM_WEIGHT_UNIT] = myConfig.getWeightUnit();
  obj[PARAM_VOLUME_UNIT] = myConfig.getVolumeUnit();
  obj[PARAM_TEMP_FORMAT] = String(myConfig.getTempFormat());

  obj[PARAM_UPTIME_SECONDS] = myUptime.getSeconds();
  obj[PARAM_UPTIME_MINUTES] = myUptime.getMinutes();
  obj[PARAM_UPTIME_HOURS] = myUptime.getHours();
  obj[PARAM_UPTIME_DAYS] = myUptime.getDays();

  // For this we use the last value read from the scale to avoid having to much
  // communication. The value will be updated regulary second in the main loop.
  if (myLevelDetection.hasStableWeight(UnitIndex::U1)) {
    obj[PARAM_GLASS1] = serialized(
        String(myLevelDetection.getNoStableGlasses(UnitIndex::U1), 1));
  }
  if (myLevelDetection.hasStableWeight(UnitIndex::U2)) {
    obj[PARAM_GLASS2] = serialized(
        String(myLevelDetection.getNoStableGlasses(UnitIndex::U2), 1));
  }

  obj[PARAM_KEG_VOLUME1] =
      convertOutgoingVolume(myConfig.getKegVolume(UnitIndex::U1));
  obj[PARAM_KEG_VOLUME2] =
      convertOutgoingVolume(myConfig.getKegVolume(UnitIndex::U2));

  float f = myTemp.getLastTempC();

  if (!isnan(f)) {
    obj[PARAM_TEMP] = serialized(String(convertOutgoingTemperature(f), 2));
  }

  float h = myTemp.getLastHumidity();

  if (!isnan(h)) {
    obj[PARAM_HUMIDITY] = serialized(String(h, 2));
  }

  float p = myTemp.getLastPressure();

  if (!isnan(p)) {
    obj[PARAM_PRESSURE] = serialized(String(p, 2));
  }

#if defined(ESP8266)
  obj[PARAM_TOTAL_HEAP] = 81920;
  obj[PARAM_FREE_HEAP] = ESP.getFreeHeap();
  obj[PARAM_IP] = WiFi.localIP().toString();
#else
  obj[PARAM_TOTAL_HEAP] = ESP.getHeapSize();
  obj[PARAM_FREE_HEAP] = ESP.getFreeHeap();
  obj[PARAM_IP] = WiFi.localIP().toString();
#endif
  obj[PARAM_WIFI_SETUP] = (runMode == RunMode::wifiSetupMode) ? true : false;

  // Home Assistant
  if (myConfig.hasTargetMqtt()) {
    JsonObject o = obj.createNestedObject(PARAM_HOMEASSISTANT);
    HomeAssist *ha = myPush.getHomeAssist();
    o[PARAM_PUSH_AGE] = abs((int32_t)(millis() - ha->getLastTimeStamp()));
    o[PARAM_PUSH_STATUS] = ha->getLastStatus();
    o[PARAM_PUSH_CODE] = ha->getLastError();
    o[PARAM_PUSH_RESPONSE] = "";
    o[PARAM_PUSH_USED] = ha->hasRun();
  }

  // Bar helper
  if (strlen(myConfig.getBarhelperApiKey()) > 0) {
    JsonObject o = obj.createNestedObject(PARAM_BARHELPER);
    Barhelper *bar = myPush.getBarHelper();
    o[PARAM_PUSH_AGE] = abs((int32_t)(millis() - bar->getLastTimeStamp()));
    o[PARAM_PUSH_STATUS] = bar->getLastStatus();
    o[PARAM_PUSH_CODE] = bar->getLastError();
    o[PARAM_PUSH_RESPONSE] = bar->getLastResponse();
    o[PARAM_PUSH_USED] = bar->hasRun();
  }

  // Brewlogger helper
  if (strlen(myConfig.getBrewLoggerUrl()) > 0) {
    JsonObject o = obj.createNestedObject(PARAM_BREWLOGGER);
    BrewLogger *blog = myPush.getBrewLogger();
    o[PARAM_PUSH_AGE] = abs((int32_t)(millis() - blog->getLastTimeStamp()));
    o[PARAM_PUSH_STATUS] = blog->getLastStatus();
    o[PARAM_PUSH_CODE] = blog->getLastError();
    o[PARAM_PUSH_RESPONSE] = blog->getLastResponse();
    o[PARAM_PUSH_USED] = blog->hasRun();
  }

  // Brewspy
  if (strlen(myConfig.getBrewspyToken(UnitIndex::U1)) > 0 ||
      strlen(myConfig.getBrewspyToken(UnitIndex::U2)) > 0) {
    JsonObject o = obj.createNestedObject(PARAM_BREWSPY);
    Brewspy *brew = myPush.getBrewspy();
    o[PARAM_PUSH_AGE] = abs((int32_t)(millis() - brew->getLastTimeStamp()));
    o[PARAM_PUSH_STATUS] = brew->getLastStatus();
    o[PARAM_PUSH_CODE] = brew->getLastError();
    o[PARAM_PUSH_RESPONSE] = brew->getLastResponse();
    o[PARAM_PUSH_USED] = brew->hasRun();
  }

  response->setLength();
  request->send(response);
}

void KegWebHandler::webStability(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback /api/stability." CR));

  constexpr auto PARAM_STABILITY_COUNT1 = "stability_count1";
  constexpr auto PARAM_STABILITY_COUNT2 = "stability_count2";
  constexpr auto PARAM_STABILITY_SUM1 = "stability_sum1";
  constexpr auto PARAM_STABILITY_SUM2 = "stability_sum2";
  constexpr auto PARAM_STABILITY_MIN1 = "stability_min1";
  constexpr auto PARAM_STABILITY_MIN2 = "stability_min2";
  constexpr auto PARAM_STABILITY_MAX1 = "stability_max1";
  constexpr auto PARAM_STABILITY_MAX2 = "stability_max2";
  constexpr auto PARAM_STABILITY_AVE1 = "stability_ave1";
  constexpr auto PARAM_STABILITY_AVE2 = "stability_ave2";
  constexpr auto PARAM_STABILITY_VAR1 = "stability_var1";
  constexpr auto PARAM_STABILITY_VAR2 = "stability_var2";
  constexpr auto PARAM_STABILITY_POPDEV1 = "stability_popdev1";
  constexpr auto PARAM_STABILITY_POPDEV2 = "stability_popdev2";
  constexpr auto PARAM_STABILITY_UBIASDEV1 = "stability_ubiasdev1";
  constexpr auto PARAM_STABILITY_UBIASDEV2 = "stability_ubiasdev2";

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_L);
  JsonObject obj = response->getRoot().as<JsonObject>();

  Stability *stability1 = myLevelDetection.getStability(UnitIndex::U1);
  Stability *stability2 = myLevelDetection.getStability(UnitIndex::U2);

  obj[PARAM_WEIGHT_UNIT] = myConfig.getWeightUnit();

  // TODO: Fix formatting of the stability values

  if (stability1->count() > 1) {
    obj[PARAM_STABILITY_COUNT1] = stability1->count();
    obj[PARAM_STABILITY_SUM1] = stability1->sum();
    obj[PARAM_STABILITY_MIN1] = stability1->min();
    obj[PARAM_STABILITY_MAX1] = stability1->max();
    obj[PARAM_STABILITY_AVE1] = stability1->average();
    obj[PARAM_STABILITY_VAR1] = stability1->variance();
    obj[PARAM_STABILITY_POPDEV1] = stability1->popStdev();
    obj[PARAM_STABILITY_UBIASDEV1] = stability1->unbiasedStdev();
  }

  if (stability2->count() > 1) {
    obj[PARAM_STABILITY_COUNT2] = stability2->count();
    obj[PARAM_STABILITY_SUM2] = stability2->sum();
    obj[PARAM_STABILITY_MIN2] = stability2->min();
    obj[PARAM_STABILITY_MAX2] = stability2->max();
    obj[PARAM_STABILITY_AVE2] = stability2->average();
    obj[PARAM_STABILITY_VAR2] = stability2->variance();
    obj[PARAM_STABILITY_POPDEV2] = stability2->popStdev();
    obj[PARAM_STABILITY_UBIASDEV2] = stability2->unbiasedStdev();
  }

  constexpr auto PARAM_LEVEL_RAW1 = "level_raw1";
  constexpr auto PARAM_LEVEL_RAW2 = "level_raw2";
  constexpr auto PARAM_LEVEL_KALMAN1 = "level_kalman1";
  constexpr auto PARAM_LEVEL_KALMAN2 = "level_kalman2";
  constexpr auto PARAM_LEVEL_STATISTIC1 = "level_stable1";
  constexpr auto PARAM_LEVEL_STATISTIC2 = "level_stable2";

  if (myLevelDetection.getRawDetection(UnitIndex::U1)->hasRawValue())
    obj[PARAM_LEVEL_RAW1] =
        myLevelDetection.getRawDetection(UnitIndex::U1)->getRawValue();
  if (myLevelDetection.getRawDetection(UnitIndex::U1)->hasKalmanValue())
    obj[PARAM_LEVEL_KALMAN1] =
        myLevelDetection.getRawDetection(UnitIndex::U1)->getKalmanValue();
  if (myLevelDetection.getStatsDetection(UnitIndex::U1)->hasStableValue())
    obj[PARAM_LEVEL_STATISTIC1] =
        myLevelDetection.getStatsDetection(UnitIndex::U1)->getStableValue();

  if (myLevelDetection.getRawDetection(UnitIndex::U2)->hasRawValue())
    obj[PARAM_LEVEL_RAW2] =
        myLevelDetection.getRawDetection(UnitIndex::U2)->getRawValue();
  if (myLevelDetection.getRawDetection(UnitIndex::U2)->hasKalmanValue())
    obj[PARAM_LEVEL_KALMAN2] =
        myLevelDetection.getRawDetection(UnitIndex::U2)->getKalmanValue();
  if (myLevelDetection.getStatsDetection(UnitIndex::U2)->hasStableValue())
    obj[PARAM_LEVEL_STATISTIC2] =
        myLevelDetection.getStatsDetection(UnitIndex::U2)->getStableValue();

  float f = myTemp.getLastTempC();

  if (!isnan(f)) {
    obj[PARAM_TEMP] = convertOutgoingTemperature(f);
  }

  float h = myTemp.getLastHumidity();

  if (!isnan(h)) {
    obj[PARAM_HUMIDITY] = h;
  }

  response->setLength();
  request->send(response);
}

void KegWebHandler::webStabilityClear(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback /api/stability/clear." CR));

  myLevelDetection.getStability(UnitIndex::U1)->clear();
  myLevelDetection.getStability(UnitIndex::U2)->clear();

  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Stability data cleared";
  response->setLength();
  request->send(response);
}

void KegWebHandler::webHardwareScan(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/hardware." CR));
  _hardwareScanTask = true;
  _hardwareScanData = "";
  AsyncJsonResponse *response =
      new AsyncJsonResponse(false, JSON_BUFFER_SIZE_S);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Scheduled hardware scanning";
  response->setLength();
  request->send(response);
}

void KegWebHandler::webHardwareScanStatus(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/hardware/status." CR));

  if (_hardwareScanTask || !_hardwareScanData.length()) {
    AsyncJsonResponse *response =
        new AsyncJsonResponse(false, JSON_BUFFER_SIZE_L);
    JsonObject obj = response->getRoot().as<JsonObject>();
    obj[PARAM_STATUS] = static_cast<bool>(_hardwareScanTask);
    obj[PARAM_SUCCESS] = false;
    obj[PARAM_MESSAGE] =
        _hardwareScanTask ? "Hardware scanning running" : "No scanning running";
    response->setLength();
    request->send(response);
  } else {
    request->send(200, "application/json", _hardwareScanData);
  }
}

void KegWebHandler::loop() {
#if defined(ESP8266)
  MDNS.update();
#endif
  BaseWebServer::loop();

  if (_hardwareScanTask) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE_L);
    JsonObject obj = doc.createNestedObject();
    obj[PARAM_STATUS] = false;
    obj[PARAM_SUCCESS] = true;
    obj[PARAM_MESSAGE] = "";
    Log.notice(F("WEB : Scanning hardware." CR));

    // Scan the i2c bus for devices
    // Wire.begin(PIN_SDA, PIN_SCL); // Should already have been done in
    // gyro.cpp
    JsonObject i2c = obj.createNestedObject(PARAM_I2C);
    JsonArray i2c_1 = i2c.createNestedArray(PARAM_I2C_1);

    // Scan bus #1
    for (int i = 1; i < 127; i++) {
      // The i2c_scanner uses the return value of
      // the Write.endTransmisstion to see if
      // a device did acknowledge to the address.
      Wire.beginTransmission(i);
      int err = Wire.endTransmission();

      if (err == 0) {
        JsonObject sensor = i2c_1.createNestedObject();
        sensor[PARAM_ADRESS] = "0x" + String(i, 16);
      }
    }

#if defined(ESP32)
    JsonArray i2c_2 = i2c.createNestedArray(PARAM_I2C_2);

    // Scan bus #2
    for (int i = 1; i < 127; i++) {
      // The i2c_scanner uses the return value of
      // the Write.endTransmisstion to see if
      // a device did acknowledge to the address.
      Wire1.beginTransmission(i);
      int err = Wire1.endTransmission();

      if (err == 0) {
        JsonObject sensor = i2c_2.createNestedObject();
        sensor[PARAM_ADRESS] = "0x" + String(i, 16);
      }
    }
#endif

    // TODO: Scan for HX711 boards

    // TODO: Scan for all temperature sensors

    // Scan onewire
    /*
    JsonArray onew = obj.createNestedArray(PARAM_ONEWIRE);

    for (int i = 0; i < mySensors.getDS18Count(); i++) {
      DeviceAddress adr;
      JsonObject sensor = onew.createNestedObject();
      mySensors.getAddress(&adr[0], i);
      sensor[PARAM_ADRESS] = String(adr[0], 16) + String(adr[1], 16) +
                             String(adr[2], 16) + String(adr[3], 16) +
                             String(adr[4], 16) + String(adr[5], 16) +
                             String(adr[6], 16) + String(adr[7], 16);
      switch (adr[0]) {
        case DS18S20MODEL:
          sensor[PARAM_FAMILY] = "DS18S20";
          break;
        case DS18B20MODEL:
          sensor[PARAM_FAMILY] = "DS18B20";
          break;
        case DS1822MODEL:
          sensor[PARAM_FAMILY] = "DS1822";
          break;
        case DS1825MODEL:
          sensor[PARAM_FAMILY] = "DS1825";
          break;
        case DS28EA00MODEL:
          sensor[PARAM_FAMILY] = "DS28EA00";
          break;
      }
      sensor[PARAM_RESOLUTION] = mySensors.getResolution();
    }*/

    JsonObject cpu = obj.createNestedObject(PARAM_CHIP);

#if defined(ESP8266)
    cpu[PARAM_FAMILY] = "ESP8266";
#else
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    cpu[PARAM_REVISION] = chip_info.revision;
    cpu[PARAM_CORES] = chip_info.cores;

    JsonArray feature = cpu.createNestedArray(PARAM_FEATURES);

    if (chip_info.features & CHIP_FEATURE_EMB_FLASH)
      feature.add("embedded flash");
    if (chip_info.features & CHIP_FEATURE_WIFI_BGN)
      feature.add("Embedded Flash");
    if (chip_info.features & CHIP_FEATURE_EMB_FLASH) feature.add("2.4Ghz WIFI");
    if (chip_info.features & CHIP_FEATURE_BLE) feature.add("Bluetooth LE");
    if (chip_info.features & CHIP_FEATURE_BT) feature.add("Bluetooth Classic");
    if (chip_info.features & CHIP_FEATURE_IEEE802154)
      feature.add("IEEE 802.15.4/LR-WPAN");
    if (chip_info.features & CHIP_FEATURE_EMB_PSRAM)
      feature.add("Embedded PSRAM");

    switch (chip_info.model) {
      case CHIP_ESP32:
        cpu[PARAM_FAMILY] = "ESP32";
        break;
      case CHIP_ESP32S2:
        cpu[PARAM_FAMILY] = "ESP32S2";
        break;
      case CHIP_ESP32S3:
        cpu[PARAM_FAMILY] = "ESP32S3";
        break;
      case CHIP_ESP32C3:
        cpu[PARAM_FAMILY] = "ESP32C3";
        break;
      case CHIP_ESP32H2:
        cpu[PARAM_FAMILY] = "ESP32H2";
        break;
      default:
        cpu[PARAM_FAMILY] = String(chip_info.model);
        break;
    }
#endif

    serializeJson(obj, _hardwareScanData);
    Log.notice(F("WEB : Scan complete %s." CR), _hardwareScanData.c_str());
    _hardwareScanTask = false;
  }
}

// EOF
