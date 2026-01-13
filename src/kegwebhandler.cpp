/*
MIT License

Copyright (c) 2021-2026 Magnus

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
#include <DallasTemperature.h>
#include <OneWire.h>
#include <esp_chip_info.h>
#include <freertos/FreeRTOS.h>

#include <changedetection.hpp>
#include <kegpush.hpp>
#include <kegwebhandler.hpp>
#include <main.hpp>
#include <scale.hpp>
#include <temp_mgr.hpp>
#include <uptime.hpp>
#include <utils.hpp>

// Configuration or api params
constexpr auto PARAM_APP_VER = "app_ver";
constexpr auto PARAM_APP_BUILD = "app_build";
constexpr auto PARAM_PLATFORM = "platform";
constexpr auto PARAM_BOARD = "board";
constexpr auto PARAM_CHIP_ID = "chip_id";
constexpr auto PARAM_FIRMWARE_FILE = "firmware_file";
constexpr auto PARAM_FEATURE_NO_SCALES = "no_scales";
constexpr auto PARAM_FEATURE_TFT = "tft";

// Other values
constexpr auto PARAM_TOTAL_HEAP = "total_heap";
constexpr auto PARAM_FREE_HEAP = "free_heap";
constexpr auto PARAM_IP = "ip";
constexpr auto PARAM_ADRESS = "adress";
constexpr auto PARAM_FAMILY = "family";
constexpr auto PARAM_CHIP = "chip";
constexpr auto PARAM_REVISION = "revision";
constexpr auto PARAM_CORES = "cores";
constexpr auto PARAM_FEATURES = "features";
constexpr auto PARAM_WIFI_SETUP = "wifi_setup";
constexpr auto PARAM_ONEWIRE = "onewire";

constexpr auto PARAM_UPTIME_SECONDS = "uptime_seconds";
constexpr auto PARAM_UPTIME_MINUTES = "uptime_minutes";
constexpr auto PARAM_UPTIME_HOURS = "uptime_hours";
constexpr auto PARAM_UPTIME_DAYS = "uptime_days";

// Temperature sensor parameters
constexpr auto PARAM_SENSORS = "sensors";
constexpr auto PARAM_SENSOR_INDEX = "index";
constexpr auto PARAM_SENSOR_ID = "id";
constexpr auto PARAM_SENSOR_TEMPERATURE = "temperature";

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

// Scale parameters
constexpr auto PARAM_SCALE_INDEX = "index";
constexpr auto PARAM_STATE = "state";
constexpr auto PARAM_STABLE_WEIGHT = "stable_weight";
constexpr auto PARAM_POUR_VOLUME = "pour_volume";
constexpr auto PARAM_CONNECTED = "connected";
constexpr auto PARAM_WEIGHT = "weight";
constexpr auto PARAM_SCALE = "scale_index";
constexpr auto PARAM_SCALE_BUSY = "scale_busy";
constexpr auto PARAM_RECENT_EVENTS = "recent_events";
const auto PARAM_TYPE = "type";
const auto PARAM_UNIT = "unit";
const auto PARAM_TIMESTAMP_MS = "timestamp_ms";
const auto PARAM_NAME = "name";
const auto PARAM_DATA = "data";
const auto PARAM_STABLE_WEIGHT_KG = "stable_weight_kg";
const auto PARAM_STABLE_VOLUME_L = "stable_volume_l";
const auto PARAM_DURATION_MS = "duration_ms";
const auto PARAM_PRE_WEIGHT_KG = "pre_weight_kg";
const auto PARAM_POST_WEIGHT_KG = "post_weight_kg";
const auto PARAM_WEIGHT_KG = "weight_kg";
const auto PARAM_VOLUME_L = "volume_l";
const auto PARAM_AVG_SLOPE_KG_SEC = "avg_slope_kg_sec";
const auto PARAM_PREVIOUS_WEIGHT_KG = "previous_weight_kg";
const auto PARAM_CURRENT_WEIGHT_KG = "current_weight_kg";
const auto PARAM_EVENT_STARTUP = "startup";
const auto PARAM_EVENT_STABLE_DETECTED = "stable_detected";
const auto PARAM_EVENT_POUR_STARTED = "pour_started";
const auto PARAM_EVENT_POUR_COMPLETED = "pour_completed";
const auto PARAM_EVENT_KEG_REMOVED = "keg_removed";
const auto PARAM_EVENT_KEG_REPLACED = "keg_replaced";
const auto PARAM_EVENT_INVALID_WEIGHT = "invalid_weight";
const auto PARAM_INVALID_WEIGHT_KG = "weight_kg";
const auto PARAM_MIN_VALID_WEIGHT_KG = "min_valid_weight_kg";
const auto PARAM_MAX_VALID_WEIGHT_KG = "max_valid_weight_kg";

// Forward declaration
static void eventToJson(const ChangeDetectionEvent &event, JsonObject obj);
constexpr auto PARAM_SCALE_WEIGHT = "scale_weight";
constexpr auto PARAM_SCALE_RAW = "scale_raw";
constexpr auto PARAM_GLASS = "glass";

KegWebHandler::KegWebHandler(KegConfig *config) : BaseWebServer(config) {
  _config = config;
}

void KegWebHandler::setupWebHandlers() {
  Log.notice(F("WEB : Setting up keg web handlers." CR));

  BaseWebServer::setupWebHandlers();

  MDNS.addService("kegmon", "tcp", 80);

  // TODO(mpse) : Serve static files from SD card
  // Note! For the async implementation the order matters
  // _server->serveStatic("/levels2", LittleFS, LEVELS_FILENAME2);
  // _server->serveStatic("/levels", LittleFS, LEVELS_FILENAME);
  // _server->serveStatic("/startup", LittleFS, STARTUP_FILENAME);

  AsyncCallbackJsonWebHandler *handler;
  handler = new AsyncCallbackJsonWebHandler(
      "/api/scale/tare",
      std::bind(&KegWebHandler::webScaleTare, this, std::placeholders::_1,
                std::placeholders::_2));
  _server->addHandler(handler);
  handler = new AsyncCallbackJsonWebHandler(
      "/api/scale/factor",
      std::bind(&KegWebHandler::webScaleFactor, this, std::placeholders::_1,
                std::placeholders::_2));
  _server->addHandler(handler);
  _server->on("/api/scale", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->webScale(request);
  });
  _server->on("/api/feature", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->webFeature(request);
  });
  _server->on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->webConfigGet(request);
  });
  handler = new AsyncCallbackJsonWebHandler(
      "/api/config", std::bind(&KegWebHandler::webConfigPost, this,
                               std::placeholders::_1, std::placeholders::_2));
  _server->addHandler(handler);
  _server->on("/api/statistic/clear", HTTP_GET,
              [this](AsyncWebServerRequest *request) {
                this->webStatisticClear(request);
              });
  _server->on(
      "/api/statistic", HTTP_GET,
      [this](AsyncWebServerRequest *request) { this->webStatistic(request); });
  _server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->webStatus(request);
  });
  handler = new AsyncCallbackJsonWebHandler(
      "/api/brewspy/tap",
      std::bind(&KegWebHandler::webHandleBrewspy, this, std::placeholders::_1,
                std::placeholders::_2));
  _server->addHandler(handler);
  _server->on("/api/hardware/status", HTTP_GET,
              [this](AsyncWebServerRequest *request) {
                this->webHardwareScanStatus(request);
              });
  _server->on("/api/hardware", HTTP_GET,
              [this](AsyncWebServerRequest *request) {
                this->webHardwareScan(request);
              });
  _server->on("/api/factory", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->webHandleFactoryDefaults(request);
  });
}

void KegWebHandler::webConfigGet(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback for /api/config(read)." CR));
  AsyncJsonResponse *response = new AsyncJsonResponse(false);
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
  LittleFS.end();
  Log.notice(F("WEB : Deleted files in filesystem, rebooting." CR));

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
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
  myConfig.parseJson(obj);
  obj.clear();
  myConfig.saveFile();

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Configuration updated";
  response->setLength();
  request->send(response);
}

void KegWebHandler::webHandleBrewspy(AsyncWebServerRequest *request,
                                     JsonVariant &json) {
  if (!isAuthenticated(request)) {
    return;
  }

  JsonObject obj = json.as<JsonObject>();
  AsyncJsonResponse *response = new AsyncJsonResponse(false);
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

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_WEIGHT_UNIT] = myConfig.getWeightUnit();
  obj[PARAM_VOLUME_UNIT] = myConfig.getVolumeUnit();
  obj[PARAM_TEMP_UNIT] = String(myConfig.getTempUnit());

  obj[PARAM_SCALE_BUSY] = myScale.isScheduleRunning();

  // Return array of all scales with current state
  JsonArray scales_array = obj[PARAM_SCALES].to<JsonArray>();
  for (int i = 0; i < MAX_SCALES; i++) {
    UnitIndex idx = static_cast<UnitIndex>(i);
    JsonObject scale = scales_array.add<JsonObject>();

    scale[PARAM_SCALE_INDEX] = i;
    scale[PARAM_CONNECTED] = myScale.isConnected(idx);

    scale[PARAM_SCALE_FACTOR] = myConfig.getScaleFactor(idx);
    scale[PARAM_SCALE_OFFSET] = myConfig.getScaleOffset(idx);

    // Only add other data if scale is connected
    if (myScale.isConnected(idx)) {
      scale[PARAM_STATE] = myChangeDetection.getStateString(idx);

      scale[PARAM_STABLE_WEIGHT] = myChangeDetection.getStableWeight(idx);
      scale[PARAM_SCALE_RAW] = myScale.readLastRaw(idx);
    }
  }

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

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
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
  float weight = obj[PARAM_WEIGHT].as<float>();

  // Request will contain 1 or 2, but we need 0 or 1 for indexing.
  if (obj[PARAM_SCALE].as<int>() == 1)
    idx = UnitIndex::U1;
  else
    idx = UnitIndex::U2;

  Log.notice(
      F("WEB : webServer callback /api/scale/factor, weight=%Fkg [%d]." CR),
      weight, idx);

  myScale.scheduleFindFactor(idx, weight);

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj2 = response->getRoot().as<JsonObject>();
  obj2[PARAM_SUCCESS] = true;
  obj2[PARAM_MESSAGE] = "Scale tare is scheduled";
  response->setLength();
  request->send(response);
}

void KegWebHandler::webFeature(AsyncWebServerRequest *request) {
  Log.notice(F("WEB : webServer callback /api/feature." CR));

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();

#if defined(ESP32S3)
  obj[PARAM_PLATFORM] = "esp32s3";
#else
#error "Undefined target"
#endif
#if defined(BOARD)
  obj[PARAM_BOARD] = BOARD;
#else
  obj[PARAM_BOARD] = "UNDEFINED";
#endif
  obj[PARAM_APP_VER] = CFG_APPVER;
  obj[PARAM_APP_BUILD] = CFG_GITREV;
  obj[PARAM_CHIP_ID] = String(ESP.getEfuseMac(), HEX);
  obj[PARAM_FIRMWARE_FILE] = CFG_FILENAMEBIN;
  obj[PARAM_FEATURE_NO_SCALES] = MAX_SCALES;
#if defined(ENABLE_TFT)
  obj[PARAM_FEATURE_TFT] = true;
#else
  obj[PARAM_FEATURE_TFT] = false;
#endif

  response->setLength();
  request->send(response);
}

void KegWebHandler::webStatus(AsyncWebServerRequest *request) {
  Log.notice(F("WEB : webServer callback /api/status." CR));

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_MDNS] = myConfig.getMDNS();
  obj[PARAM_ID] = myConfig.getID();
  obj[PARAM_RSSI] = WiFi.RSSI();
  obj[PARAM_SSID] = myConfig.getWifiSSID(0);
  obj[PARAM_WEIGHT_UNIT] = myConfig.getWeightUnit();
  obj[PARAM_VOLUME_UNIT] = myConfig.getVolumeUnit();
  obj[PARAM_TEMP_FORMAT] = String(myConfig.getTempFormat());

  obj[PARAM_UPTIME_SECONDS] = myUptime.getSeconds();
  obj[PARAM_UPTIME_MINUTES] = myUptime.getMinutes();
  obj[PARAM_UPTIME_HOURS] = myUptime.getHours();
  obj[PARAM_UPTIME_DAYS] = myUptime.getDays();

  obj[PARAM_SCALE_BUSY] = myScale.isScheduleRunning();

  // Return array of all scales with current state
  JsonArray scales_array = obj[PARAM_SCALES].to<JsonArray>();
  for (int i = 0; i < MAX_SCALES; i++) {
    UnitIndex idx = static_cast<UnitIndex>(i);
    JsonObject scale = scales_array.add<JsonObject>();

    scale[PARAM_SCALE_INDEX] = i;
    scale[PARAM_CONNECTED] = myScale.isConnected(idx);
    // scale[PARAM_SCALE_FACTOR] = myConfig.getScaleFactor(idx);
    // scale[PARAM_SCALE_OFFSET] = myConfig.getScaleOffset(idx);

    // Only add other data if scale is connected
    if (myScale.isConnected(idx)) {
      scale[PARAM_STATE] = myChangeDetection.getStateString(idx);

      // Weight and volume data (in kg and cl)
      scale[PARAM_STABLE_WEIGHT] = myChangeDetection.getStableWeight(idx);
      scale[PARAM_POUR_VOLUME] = myChangeDetection.getPourVolume(idx);

      // Keg volume in centiliters
      float keg_volume_liters = myConfig.getKegVolume(idx);
      scale[PARAM_KEG_VOLUME] =
          serialized(String(keg_volume_liters * 100.0f, 0));

      // Calculate number of glasses (glass volume from config)
      float glass_volume_liters = myConfig.getGlassVolume(idx);
      if (glass_volume_liters > 0.0f &&
          myChangeDetection.getStableWeight(idx) > 0.0f) {
        // Approximate: remaining weight / keg weight * keg liters / glass
        // liters
        float keg_weight = myConfig.getKegWeight(idx);
        float remaining_liters =
            (myChangeDetection.getStableWeight(idx) / keg_weight) *
            keg_volume_liters;
        scale[PARAM_GLASS] =
            serialized(String(remaining_liters / glass_volume_liters, 1));
      }

      // scale[PARAM_SCALE_RAW] = myScale.readLastRaw(idx);
    }
  }

  // Return array of all temperature sensors
  if (myTemp.hasSensor()) {
    JsonArray sensors_array = obj[PARAM_SENSORS].to<JsonArray>();
    int sensor_count = myTemp.getSensorCount();
    for (int i = 0; i < sensor_count; i++) {
      JsonObject sensor = sensors_array.add<JsonObject>();

      sensor[PARAM_SENSOR_INDEX] = i;
      sensor[PARAM_SENSOR_ID] = myTemp.getSensorId(i);

      float temp_c = myTemp.getLastTempC(i);
      if (!isnan(temp_c)) {
        sensor[PARAM_SENSOR_TEMPERATURE] = serialized(String(temp_c, 2));
      }
    }
  }

  obj[PARAM_TOTAL_HEAP] = ESP.getHeapSize();
  obj[PARAM_FREE_HEAP] = ESP.getFreeHeap();
  obj[PARAM_IP] = WiFi.localIP().toString();
  obj[PARAM_WIFI_SETUP] = (runMode == RunMode::wifiSetupMode) ? true : false;

  // Home Assistant
  if (myConfig.hasTargetMqtt()) {
    JsonObject o = obj[PARAM_HOMEASSISTANT].to<JsonObject>();
    HomeAssist *ha = myPush.getHomeAssist();
    o[PARAM_PUSH_AGE] =
        abs(static_cast<int32_t>((millis() - ha->getLastTimeStamp())));
    o[PARAM_PUSH_STATUS] = ha->getLastStatus();
    o[PARAM_PUSH_CODE] = ha->getLastError();
    o[PARAM_PUSH_RESPONSE] = "";
    o[PARAM_PUSH_USED] = ha->hasRun();
  }

  // Bar helper
  if (strlen(myConfig.getBarhelperApiKey()) > 0) {
    JsonObject o = obj[PARAM_BARHELPER].to<JsonObject>();
    Barhelper *bar = myPush.getBarHelper();
    o[PARAM_PUSH_AGE] =
        abs(static_cast<int32_t>((millis() - bar->getLastTimeStamp())));
    o[PARAM_PUSH_STATUS] = bar->getLastStatus();
    o[PARAM_PUSH_CODE] = bar->getLastError();
    o[PARAM_PUSH_RESPONSE] = bar->getLastResponse();
    o[PARAM_PUSH_USED] = bar->hasRun();
  }

  // Brewlogger helper
  if (strlen(myConfig.getBrewLoggerUrl()) > 0) {
    JsonObject o = obj[PARAM_BREWLOGGER].to<JsonObject>();
    BrewLogger *blog = myPush.getBrewLogger();
    o[PARAM_PUSH_AGE] =
        abs(static_cast<int32_t>((millis() - blog->getLastTimeStamp())));
    o[PARAM_PUSH_STATUS] = blog->getLastStatus();
    o[PARAM_PUSH_CODE] = blog->getLastError();
    o[PARAM_PUSH_RESPONSE] = blog->getLastResponse();
    o[PARAM_PUSH_USED] = blog->hasRun();
  }

  // Brewspy
  if (strlen(myConfig.getBrewspyToken(UnitIndex::U1)) > 0 ||
      strlen(myConfig.getBrewspyToken(UnitIndex::U2)) > 0) {
    JsonObject o = obj[PARAM_BREWSPY].to<JsonObject>();
    Brewspy *brew = myPush.getBrewspy();
    o[PARAM_PUSH_AGE] =
        abs(static_cast<int32_t>((millis() - brew->getLastTimeStamp())));
    o[PARAM_PUSH_STATUS] = brew->getLastStatus();
    o[PARAM_PUSH_CODE] = brew->getLastError();
    o[PARAM_PUSH_RESPONSE] = brew->getLastResponse();
    o[PARAM_PUSH_USED] = brew->hasRun();
  }

  // Recent events from change detection
  {
    ChangeDetectionEvent recentEvents[10];  // RECENT_EVENTS_SIZE
    size_t eventCount = 0;
    this->getRecentEvents(recentEvents, eventCount);

    JsonArray events_array = obj[PARAM_RECENT_EVENTS].to<JsonArray>();
    for (size_t i = 0; i < eventCount; i++) {
      JsonObject event_obj = events_array.add<JsonObject>();
      eventToJson(recentEvents[i], event_obj);
    }
  }

  response->setLength();
  request->send(response);
}

void KegWebHandler::webStatistic(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback /api/stability." CR));

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();

  // Level statistics array (ChangeDetection v2 statistics)
  JsonArray level_statistics = obj["level_statistics"].to<JsonArray>();
  for (int i = 0; i < MAX_SCALES; i++) {
    UnitIndex idx = static_cast<UnitIndex>(i);
    JsonObject stat = level_statistics.add<JsonObject>();

    // Basic scale info
    stat[PARAM_SCALE_INDEX] = i;
    stat[PARAM_CONNECTED] = myScale.isConnected(idx);
    stat[PARAM_STATE] = myChangeDetection.getStateString(idx);
    stat["confidence"] = myChangeDetection.getConfidence(idx);

    // Get statistics for this scale
    const auto &stats = myChangeDetection.getStatistics(idx);

    // Pour statistics (in liters)
    stat["total_pours"] = stats.totalPours;
    stat["total_pour_volume"] = stats.totalPourVolume;
    stat["avg_pour_volume"] = stats.avgPourVolume;
    stat["max_pour_volume"] = stats.maxPourVolume;
    stat["min_pour_volume"] = stats.minPourVolume;
    stat["avg_pour_duration_sec"] = stats.avgPourDurationMs / 1000.0f;

    // Keg statistics
    stat["keg_replacements"] = stats.kegReplacements;
    stat["current_keg_age_hours"] = stats.getCurrentKegAgeHours();
    stat["last_keg_weight"] = stats.lastKegWeight;

    // State machine statistics
    stat["state_transitions"] = stats.stateTransitions;
    stat["stabilization_count"] = stats.stabilizationCount;
    stat["avg_stabilization_time_ms"] = stats.avgStabilizationTimeMs;

    // Event timestamps
    stat["last_pour_time_ms"] = static_cast<uint64_t>(stats.lastPourTimeMs);
    stat["last_stable_time_ms"] = static_cast<uint64_t>(stats.lastStableTimeMs);
    stat["last_keg_removal_time_ms"] =
        static_cast<uint64_t>(stats.lastKegRemovalTimeMs);
  }

  // Scale statistics array (hardware reading statistics)
  JsonArray scale_stats = obj["scale_statistics"].to<JsonArray>();
  for (int i = 0; i < MAX_SCALES; i++) {
    JsonObject ss = scale_stats.add<JsonObject>();
    const auto &scaleStats = myScale.getStatistics(static_cast<UnitIndex>(i));

    ss[PARAM_SCALE_INDEX] = i;
    ss["total_readings"] = scaleStats.totalReadings;
    ss["valid_readings"] = scaleStats.validReadings;
    ss["invalid_readings"] = scaleStats.invalidReadings;
    ss["reading_quality"] =
        serialized(String(scaleStats.getReadingQuality(), 1));
    ss["reading_frequency"] =
        serialized(String(scaleStats.getReadingFrequency(), 2));
    ss["last_reading_time_ms"] =
        static_cast<uint64_t>(scaleStats.lastReadingTimeMs);
    ss["first_reading_time_ms"] =
        static_cast<uint64_t>(scaleStats.firstReadingTimeMs);
    ss["raw_min"] = serialized(String(scaleStats.rawMin, 0));
    ss["raw_max"] = serialized(String(scaleStats.rawMax, 0));
    ss["raw_average"] = serialized(String(scaleStats.getRawAverage(), 0));
    ss["current_variance"] = serialized(String(scaleStats.currentVariance, 2));
    ss["stable_state_variance"] =
        serialized(String(scaleStats.stableStateVariance, 2));
    ss["stable_state_samples"] = scaleStats.stableStateSamples;
    ss["calibration_drift_per_hour"] =
        serialized(String(scaleStats.calibrationDriftPerHour, 3));
  }

  response->setLength();
  request->send(response);
}

void KegWebHandler::webStatisticClear(AsyncWebServerRequest *request) {
  if (!isAuthenticated(request)) {
    return;
  }

  Log.notice(F("WEB : webServer callback /api/statistic/clear." CR));

  // Clear statistics and reset state for all scales
  for (int i = 0; i < MAX_SCALES; i++) {
    UnitIndex idx = static_cast<UnitIndex>(i);
    myChangeDetection.resetStatistics(idx);
    myScale.resetStatistics(idx);
  }

  AsyncJsonResponse *response = new AsyncJsonResponse(false);
  JsonObject obj = response->getRoot().as<JsonObject>();
  obj[PARAM_SUCCESS] = true;
  obj[PARAM_MESSAGE] = "Statistics cleared for all scales";
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
  AsyncJsonResponse *response = new AsyncJsonResponse(false);
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
    AsyncJsonResponse *response = new AsyncJsonResponse(false);
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
  BaseWebServer::loop();

  if (_hardwareScanTask) {
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    obj[PARAM_STATUS] = false;
    obj[PARAM_SUCCESS] = true;
    obj[PARAM_MESSAGE] = "";
    Log.notice(F("WEB : Scanning hardware." CR));

    // For this we use the last value read from the scale to avoid having to
    // much communication. The value will be updated regulary second in the main
    // loop. Return array of all scales with current state
    JsonArray scales_array = obj[PARAM_SCALES].to<JsonArray>();
    for (int i = 0; i < MAX_SCALES; i++) {
      UnitIndex idx = static_cast<UnitIndex>(i);
      JsonObject scale = scales_array.add<JsonObject>();

      scale[PARAM_SCALE_INDEX] = i;
      scale[PARAM_CONNECTED] = myScale.isConnected(idx);
    }

    // Scan onewire
    JsonArray sensors_array = obj[PARAM_ONEWIRE].to<JsonArray>();

    for (int i = 0; i < myTemp.getSensorCount(); i++) {
      JsonObject sensor = sensors_array.add<JsonObject>();
      String id = myTemp.getSensorId(i);
      int type = id.length() > 2
                     ? (strtol(id.substring(0, 2).c_str(), nullptr, 16))
                     : 0;
      sensor[PARAM_SENSOR_INDEX] = i;
      sensor[PARAM_ADRESS] = id;

      switch (type) {
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
        default:
          sensor[PARAM_FAMILY] = String(type, HEX);
          break;
      }
    }

    JsonObject cpu = obj[PARAM_CHIP].to<JsonObject>();

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    cpu[PARAM_REVISION] = chip_info.revision;
    cpu[PARAM_CORES] = chip_info.cores;

    JsonArray feature_array = cpu[PARAM_FEATURES].to<JsonArray>();

    if (chip_info.features & CHIP_FEATURE_EMB_FLASH)
      feature_array.add("embedded flash");
    if (chip_info.features & CHIP_FEATURE_WIFI_BGN)
      feature_array.add("Embedded Flash");
    if (chip_info.features & CHIP_FEATURE_EMB_FLASH)
      feature_array.add("2.4Ghz WIFI");
    if (chip_info.features & CHIP_FEATURE_BLE)
      feature_array.add("Bluetooth LE");
    if (chip_info.features & CHIP_FEATURE_BT)
      feature_array.add("Bluetooth Classic");
    if (chip_info.features & CHIP_FEATURE_IEEE802154)
      feature_array.add("IEEE 802.15.4/LR-WPAN");
    if (chip_info.features & CHIP_FEATURE_EMB_PSRAM)
      feature_array.add("Embedded PSRAM");
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

    serializeJson(obj, _hardwareScanData);
    Log.notice(F("WEB : Scan complete %s." CR), _hardwareScanData.c_str());
    _hardwareScanTask = false;
  }
}

void KegWebHandler::queueEvent(const ChangeDetectionEvent &event) {
  portENTER_CRITICAL(&_eventLock);

  // Store event in ringbuffer at head position
  size_t head = _eventHead.load();
  _recentEvents[head] = event;
  _eventHead.store((head + 1) % RECENT_EVENTS_SIZE);

  // Track count (max out at RECENT_EVENTS_SIZE)
  size_t count = _eventCount.load();
  if (count < RECENT_EVENTS_SIZE) {
    _eventCount.store(count + 1);
  }

  portEXIT_CRITICAL(&_eventLock);
}

void KegWebHandler::getRecentEvents(ChangeDetectionEvent *outEvents,
                                    size_t &count) {
  // Snapshot the count to ensure consistent read
  count = _eventCount.load();

  // Copy events in chronological order (oldest first)
  if (count > 0) {
    size_t startIdx = (count < RECENT_EVENTS_SIZE) ? 0 : _eventHead.load();

    for (size_t i = 0; i < count; i++) {
      size_t srcIdx = (startIdx + i) % RECENT_EVENTS_SIZE;
      outEvents[i] = _recentEvents[srcIdx];
    }
  }
}

// Convert ChangeDetectionEvent to JSON
static void eventToJson(const ChangeDetectionEvent &event, JsonObject obj) {
  // Base event fields
  obj[PARAM_TYPE] = static_cast<int>(event.type);
  obj[PARAM_UNIT] = static_cast<int>(event.unitIndex);
  obj[PARAM_TIMESTAMP_MS] = static_cast<uint32_t>(event.timestampMs);

  // Event-specific data
  switch (event.type) {
    case ChangeDetectionEventType::SYSTEM_STARTUP:
      obj[PARAM_NAME] = PARAM_EVENT_STARTUP;
      break;

    case ChangeDetectionEventType::STABLE_DETECTED:
      obj[PARAM_NAME] = PARAM_EVENT_STABLE_DETECTED;
      {
        JsonObject stable = obj[PARAM_DATA].to<JsonObject>();
        stable[PARAM_STABLE_WEIGHT_KG] =
            serialized(String(event.stable.stableWeightKg, 4));
        stable[PARAM_STABLE_VOLUME_L] =
            serialized(String(event.stable.stableVolumeL, 4));
        stable[PARAM_DURATION_MS] =
            static_cast<uint32_t>(event.stable.durationMs);
      }
      break;

    case ChangeDetectionEventType::POUR_STARTED:
      obj[PARAM_NAME] = PARAM_EVENT_POUR_STARTED;
      {
        JsonObject pour = obj[PARAM_DATA].to<JsonObject>();
        pour[PARAM_PRE_WEIGHT_KG] =
            serialized(String(event.pour.prePourWeightKg, 4));
      }
      break;

    case ChangeDetectionEventType::POUR_COMPLETED:
      obj[PARAM_NAME] = PARAM_EVENT_POUR_COMPLETED;
      {
        JsonObject pour = obj[PARAM_DATA].to<JsonObject>();
        pour[PARAM_PRE_WEIGHT_KG] =
            serialized(String(event.pour.prePourWeightKg, 4));
        pour[PARAM_POST_WEIGHT_KG] =
            serialized(String(event.pour.postPourWeightKg, 4));
        pour[PARAM_WEIGHT_KG] = serialized(String(event.pour.pourWeightKg, 4));
        pour[PARAM_VOLUME_L] = serialized(String(event.pour.pourVolumeL, 4));
        pour[PARAM_DURATION_MS] = static_cast<uint32_t>(event.pour.durationMs);
        pour[PARAM_AVG_SLOPE_KG_SEC] =
            serialized(String(event.pour.averageSlopeKgSec, 6));
      }
      break;

    case ChangeDetectionEventType::KEG_REMOVED:
      obj[PARAM_NAME] = PARAM_EVENT_KEG_REMOVED;
      {
        JsonObject weight = obj[PARAM_DATA].to<JsonObject>();
        weight[PARAM_PREVIOUS_WEIGHT_KG] =
            serialized(String(event.weight.previousWeightKg, 4));
        weight[PARAM_CURRENT_WEIGHT_KG] =
            serialized(String(event.weight.currentWeightKg, 4));
      }
      break;

    case ChangeDetectionEventType::KEG_REPLACED:
      obj[PARAM_NAME] = PARAM_EVENT_KEG_REPLACED;
      {
        JsonObject weight = obj[PARAM_DATA].to<JsonObject>();
        weight[PARAM_PREVIOUS_WEIGHT_KG] =
            serialized(String(event.weight.previousWeightKg, 4));
        weight[PARAM_CURRENT_WEIGHT_KG] =
            serialized(String(event.weight.currentWeightKg, 4));
      }
      break;

    case ChangeDetectionEventType::INVALID_WEIGHT:
      obj[PARAM_NAME] = PARAM_EVENT_INVALID_WEIGHT;
      {
        JsonObject invalid = obj[PARAM_DATA].to<JsonObject>();
        invalid[PARAM_INVALID_WEIGHT_KG] =
            serialized(String(event.invalid.weightKg, 4));
        invalid[PARAM_MIN_VALID_WEIGHT_KG] =
            serialized(String(event.invalid.minValidWeightKg, 4));
        invalid[PARAM_MAX_VALID_WEIGHT_KG] =
            serialized(String(event.invalid.maxValidWeightKg, 4));
      }
      break;
  }
}

// EOF
