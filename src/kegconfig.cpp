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
#include <kegconfig.hpp>
#include <log.hpp>
#include <main.hpp>
#include <utils.hpp>

KegConfig::KegConfig(String baseMDNS, String fileName)
    : BaseConfig(baseMDNS, fileName) {}

void KegConfig::createJson(JsonObject& doc) const {
  // Call base class functions
  createJsonBase(doc);
  createJsonWifi(doc);
  createJsonOta(doc);
  createJsonPush(doc);

  // Handle project specific config
  doc[PARAM_WEIGHT_UNIT] = getWeightUnit();
  doc[PARAM_VOLUME_UNIT] = getVolumeUnit();

  doc[PARAM_DISPLAY_LAYOUT] = getDisplayLayoutTypeAsInt();
  // UNUSED: Currently not persisted in config
  // doc[PARAM_TEMP_SENSOR] = getTempSensorTypeAsInt();

  doc[PARAM_BREWFATHER_APIKEY] = getBrewfatherApiKey();
  doc[PARAM_BREWFATHER_USERKEY] = getBrewfatherUserKey();

  doc[PARAM_BREWPI_URL] = getBrewpiUrl();
  doc[PARAM_CHAMBERCTRL_URL] = getChamberCtrlUrl();

  doc[PARAM_BREWLOGGER_URL] = getBrewLoggerUrl();

  // Brewspy tokens as array
  JsonArray brewspy = doc[PARAM_BREWSPY_TOKENS].to<JsonArray>();
  for (int i = 0; i < MAX_SCALES; i++) {
    brewspy.add(getBrewspyToken(static_cast<UnitIndex>(i)));
  }

  doc[PARAM_BARHELPER_APIKEY] = getBarhelperApiKey();

  // Barhelper monitors as array
  JsonArray barhelper_monitors = doc[PARAM_BARHELPER_MONITORS].to<JsonArray>();
  for (int i = 0; i < MAX_SCALES; i++) {
    barhelper_monitors.add(getBarhelperMonitor(static_cast<UnitIndex>(i)));
  }

  // Save scale and beer data as arrays
  JsonArray scales = doc[PARAM_SCALES].to<JsonArray>();
  for (int i = 0; i < MAX_SCALES; i++) {
    UnitIndex idx = static_cast<UnitIndex>(i);
    JsonObject scale = scales.add<JsonObject>();

    scale[PARAM_SCALE_FACTOR] = serialized(String(getScaleFactor(idx), 5));
    scale[PARAM_SCALE_OFFSET] = getScaleOffset(idx);
    scale[PARAM_KEG_WEIGHT] = serialized(
        String(convertOutgoingWeight(getKegWeight(idx)), getWeightPrecision()));
    scale[PARAM_KEG_VOLUME] =
        serialized(String(getKegVolume(idx), getWeightPrecision()));
    scale[PARAM_GLASS_VOLUME] =
        serialized(String(getGlassVolume(idx), getWeightPrecision()));
    scale[PARAM_TEMP_SENSOR_ID] = getTempSensorId(idx);
  }

  // Save beer data as arrays
  JsonArray beers = doc[PARAM_BEERS].to<JsonArray>();
  for (int i = 0; i < MAX_SCALES; i++) {
    UnitIndex idx = static_cast<UnitIndex>(i);
    JsonObject beer = beers.add<JsonObject>();

    beer[PARAM_BEER_NAME] = getBeerName(idx);
    beer[PARAM_BEER_ID] = getBeerId(idx);
    beer[PARAM_BEER_ABV] = serialized(String(getBeerABV(idx), 2));
    beer[PARAM_BEER_FG] = serialized(String(getBeerFG(idx), 2));
    beer[PARAM_BEER_EBC] = getBeerEBC(idx);
    beer[PARAM_BEER_IBU] = getBeerIBU(idx);
  }
}

void KegConfig::parseJson(JsonObject& doc) {
  // Call base class functions
  parseJsonBase(doc);
  parseJsonWifi(doc);
  parseJsonOta(doc);
  parseJsonPush(doc);

  // Handle project specific config
  if (!doc[PARAM_WEIGHT_UNIT].isNull()) setWeightUnit(doc[PARAM_WEIGHT_UNIT]);
  if (!doc[PARAM_VOLUME_UNIT].isNull()) setVolumeUnit(doc[PARAM_VOLUME_UNIT]);

  if (!doc[PARAM_BREWFATHER_APIKEY].isNull())
    setBrewfatherApiKey(doc[PARAM_BREWFATHER_APIKEY]);
  if (!doc[PARAM_BREWFATHER_USERKEY].isNull())
    setBrewfatherUserKey(doc[PARAM_BREWFATHER_USERKEY]);

  if (!doc[PARAM_BREWPI_URL].isNull()) setBrewpiUrl(doc[PARAM_BREWPI_URL]);
  if (!doc[PARAM_CHAMBERCTRL_URL].isNull())
    setChamberCtrlUrl(doc[PARAM_CHAMBERCTRL_URL]);

  if (!doc[PARAM_BREWLOGGER_URL].isNull())
    setBrewLoggerUrl(doc[PARAM_BREWLOGGER_URL]);

  // Load brewspy tokens from array
  if (!doc[PARAM_BREWSPY_TOKENS].isNull()) {
    JsonArray brewspy = doc[PARAM_BREWSPY_TOKENS].as<JsonArray>();
    for (int i = 0; i < MAX_SCALES && i < static_cast<int>(brewspy.size());
         i++) {
      if (!brewspy[i].isNull()) {
        setBrewspyToken(static_cast<UnitIndex>(i), brewspy[i].as<String>());
      }
    }
  }

  if (!doc[PARAM_BARHELPER_APIKEY].isNull())
    setBarhelperApiKey(doc[PARAM_BARHELPER_APIKEY]);

  // Load barhelper monitors from array
  if (!doc[PARAM_BARHELPER_MONITORS].isNull()) {
    JsonArray monitors = doc[PARAM_BARHELPER_MONITORS].as<JsonArray>();
    for (int i = 0; i < MAX_SCALES && i < static_cast<int>(monitors.size());
         i++) {
      if (!monitors[i].isNull()) {
        setBarhelperMonitor(static_cast<UnitIndex>(i),
                            monitors[i].as<String>());
      }
    }
  }

  if (!doc[PARAM_DISPLAY_LAYOUT].isNull())
    setDisplayLayoutType(doc[PARAM_DISPLAY_LAYOUT].as<int>());

  // Load scale data from array
  if (!doc[PARAM_SCALES].isNull()) {
    JsonArray scales = doc[PARAM_SCALES].as<JsonArray>();
    for (int i = 0; i < MAX_SCALES && i < static_cast<int>(scales.size());
         i++) {
      UnitIndex idx = static_cast<UnitIndex>(i);
      JsonObject scale = scales[i].as<JsonObject>();

      if (!scale[PARAM_SCALE_FACTOR].isNull())
        setScaleFactor(idx, scale[PARAM_SCALE_FACTOR].as<float>());
      if (!scale[PARAM_SCALE_OFFSET].isNull())
        setScaleOffset(idx, scale[PARAM_SCALE_OFFSET].as<float>());
      if (!scale[PARAM_KEG_WEIGHT].isNull())
        setKegWeight(
            idx, convertIncomingWeight(scale[PARAM_KEG_WEIGHT].as<float>()));
      if (!scale[PARAM_KEG_VOLUME].isNull())
        setKegVolume(idx, scale[PARAM_KEG_VOLUME].as<float>());
      if (!scale[PARAM_GLASS_VOLUME].isNull())
        setGlassVolume(idx, scale[PARAM_GLASS_VOLUME].as<float>());
      if (!scale[PARAM_TEMP_SENSOR_ID].isNull())
        setTempSensorId(idx, scale[PARAM_TEMP_SENSOR_ID].as<String>());
    }
  }

  // Load beer data from array
  if (!doc[PARAM_BEERS].isNull()) {
    JsonArray beers = doc[PARAM_BEERS].as<JsonArray>();
    for (int i = 0; i < MAX_SCALES && i < static_cast<int>(beers.size()); i++) {
      UnitIndex idx = static_cast<UnitIndex>(i);
      JsonObject beer = beers[i].as<JsonObject>();

      if (!beer[PARAM_BEER_NAME].isNull())
        setBeerName(idx, beer[PARAM_BEER_NAME]);
      if (!beer[PARAM_BEER_ID].isNull()) setBeerId(idx, beer[PARAM_BEER_ID]);
      if (!beer[PARAM_BEER_EBC].isNull())
        setBeerEBC(idx, beer[PARAM_BEER_EBC].as<int>());
      if (!beer[PARAM_BEER_ABV].isNull())
        setBeerABV(idx, beer[PARAM_BEER_ABV].as<float>());
      if (!beer[PARAM_BEER_IBU].isNull())
        setBeerIBU(idx, beer[PARAM_BEER_IBU].as<int>());
      if (!beer[PARAM_BEER_FG].isNull())
        setBeerFG(idx, beer[PARAM_BEER_FG].as<float>());
    }
  }
}

float convertIncomingWeight(float w) {
  float r;

  if (myConfig.isWeightUnitKG())
    r = w;
  else
    r = convertLBStoKG(w);

#if LOG_LEVEL == 6
  // Log.verbose(F("CFG : Convering %F to %F (%s)." CR), w, r,
  // myConfig.getWeightUnit());
#endif
  return r;
}

float convertIncomingVolume(float v) {
  float r;

  if (myConfig.isVolumeUnitCL())
    r = v * 100.0;
  else if (myConfig.isVolumeUnitUSOZ())
    r = convertUSOZtoCL(v);
  else
    r = convertUKOZtoCL(v);

#if LOG_LEVEL == 6
  // Log.verbose(F("CFG : Converting incoming volume %F to %F (%s)." CR), v,
  // r, myConfig.getVolumeUnit());
#endif
  return r;
}

float convertOutgoingWeight(float w) {
  float r;

  if (myConfig.isWeightUnitKG())
    r = w;
  else
    r = convertKGtoLBS(w);

#if LOG_LEVEL == 6
  // Log.verbose(F("CFG : Converting outgoing weight %F to %F (%s)." CR), w,
  // r, myConfig.getWeightUnit());
#endif
  return r;
}

float convertOutgoingVolume(float v) {
  float r;

  if (myConfig.isVolumeUnitCL())
    r = (v == 0.0 ? 0.0 : v * 100.0);
  else if (myConfig.isVolumeUnitUSOZ())
    r = convertCLtoUSOZ(v == 0.0 ? 0.0 : v * 100.0);
  else
    r = convertCLtoUKOZ(v == 0.0 ? 0.0 : v * 100.0);

#if LOG_LEVEL == 6
  // Log.verbose(F("CFG : Converting outgoing volume %F to %F (%s)." CR), v,
  // r, myConfig.getVolumeUnit());
#endif
  return r;
}

float convertOutgoingTemperature(float t) {
  if (myConfig.isTempFormatC()) return t;
  return convertCtoF(t);
}

void KegConfig::migrateSettings() {
  constexpr auto CFG_FILENAME_OLD = "/kegmon.json";
  constexpr auto CFG_FILENAME_OLD_SAVE = "/kegmon_old.json";

  if (!LittleFS.exists(CFG_FILENAME_OLD)) {
    return;
  }

  File configFile = LittleFS.open(CFG_FILENAME_OLD, "r");

  if (!configFile) {
    Serial.println("Failed to open old config file");
    return;
  }

  JsonDocument doc;
  JsonDocument doc2;

  DeserializationError err = deserializeJson(doc, configFile);
  configFile.close();

  if (err) {
    Serial.println("Failed to open parse old config file");
    return;
  }

  JsonObject obj = doc.to<JsonObject>();
  JsonObject obj2 = doc2.to<JsonObject>();

  serializeJson(obj, EspSerial);
  EspSerial.print(CR);

  for (JsonPair kv : obj) {
    String k = kv.key().c_str();
    k.replace("-", "_");
    obj2[k] = obj[kv.key().c_str()];
  }

  obj.clear();
#if LOG_LEVEL == 6
  serializeJson(obj2, EspSerial);
  EspSerial.print(CR);
#endif
  parseJson(obj2);
  obj2.clear();

  if (saveFile()) {
    LittleFS.rename(CFG_FILENAME_OLD, CFG_FILENAME_OLD_SAVE);
    // LittleFS.remove(CFG_FILENAME_OLD);
  }

  Log.notice(F("CFG : Migrated old config /kegmon.json." CR));
}

// EOF
