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
#include <config.hpp>
#include <main.hpp>
#include <resources.hpp>
#include <wifi.hpp>

Config myConfig;

Config::Config() {
  char buf[30];

  snprintf(&buf[0], sizeof(buf), "%06x", (unsigned int)ESP.getChipId());
  _id = String(&buf[0]);
  snprintf(&buf[0], sizeof(buf), "" WIFI_MDNS "%s", getID());
  _mDNS = String(&buf[0]);

  // Log.verbose(F("Cfg : Created config for %s (%s)." CR), _id.c_str(),
  // _mDNS.c_str());
}

void Config::createJson(DynamicJsonDocument& doc) {
  doc[PARAM_MDNS] = getMDNS();
  doc[PARAM_ID] = getID();
  doc[PARAM_SSID] = getWifiSSID(0);
  doc[PARAM_PASS] = getWifiPass(0);
  doc[PARAM_SSID2] = getWifiSSID(1);
  doc[PARAM_PASS2] = getWifiPass(1);
  doc[PARAM_TEMPFORMAT] = String(getTempFormat());
  doc[PARAM_WEIGHT_PRECISION] = getWeightPrecision();
  doc[PARAM_BREWFATHER_APIKEY] = getBrewfatherApiKey();
  doc[PARAM_BREWFATHER_USERKEY] = getBrewfatherUserKey();

  doc[PARAM_SCALE_FACTOR1] = getScaleFactor(0);
  doc[PARAM_SCALE_FACTOR2] = getScaleFactor(1);
  doc[PARAM_SCALE_OFFSET1] = getScaleOffset(0);
  doc[PARAM_SCALE_OFFSET2] = getScaleOffset(1);
  doc[PARAM_KEG_WEIGHT1] = getKegWeight(0);
  doc[PARAM_KEG_WEIGHT2] = getKegWeight(1);
  doc[PARAM_PINT_WEIGHT1] = getPintWeight(0);
  doc[PARAM_PINT_WEIGHT2] = getPintWeight(1);
  doc[PARAM_BEER_NAME1] = getBeerName(0);
  doc[PARAM_BEER_NAME2] = getBeerName(1);
  doc[PARAM_BEER_ABV1] = getBeerABV(0);
  doc[PARAM_BEER_ABV2] = getBeerABV(1);
  doc[PARAM_BEER_EBC1] = getBeerEBC(0);
  doc[PARAM_BEER_EBC2] = getBeerEBC(1);
  doc[PARAM_BEER_IBU1] = getBeerIBU(0);
  doc[PARAM_BEER_IBU2] = getBeerIBU(1);
}

bool Config::saveFile() {
  if (!_saveNeeded) {
    Log.verbose(F("Cfg : Skipping save, not needed." CR));
    return true;
  }

  File configFile = LittleFS.open(CFG_FILENAME, "w");

  if (!configFile) {
    Log.error(F("Cfg : Failed to save configuration."));
    return false;
  }

  DynamicJsonDocument doc(2048);
  createJson(doc);

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  serializeJson(doc, configFile);
  configFile.flush();
  configFile.close();

  _saveNeeded = false;
  Log.notice(F("Cfg : Configuration saved to " CFG_FILENAME "." CR));
  return true;
}

bool Config::loadFile() {
  // Log.verbose(F("Cfg : Loading configuration from file." CR));

  if (!LittleFS.exists(CFG_FILENAME)) {
    Log.error(F("Cfg : Configuration file does not exist."));
    return false;
  }

  File configFile = LittleFS.open(CFG_FILENAME, "r");

  if (!configFile) {
    Log.error(F("Cfg : Failed to load configuration."));
    return false;
  }

  Log.verbose(F("Cfg : Size of configuration file=%d bytes." CR),
              configFile.size());

  DynamicJsonDocument doc(2048);
  DeserializationError err = deserializeJson(doc, configFile);

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  configFile.close();

  if (err) {
    Log.error(F("Cfg : Failed to parse configuration (json)"));
    return false;
  }

  // Log.verbose(F("Cfg : Parsed configuration file." CR));

  if (!doc[PARAM_MDNS].isNull()) setMDNS(doc[PARAM_MDNS]);
  if (!doc[PARAM_SSID].isNull()) setWifiSSID(doc[PARAM_SSID], 0);
  if (!doc[PARAM_PASS].isNull()) setWifiPass(doc[PARAM_PASS], 0);
  if (!doc[PARAM_SSID2].isNull()) setWifiSSID(doc[PARAM_SSID2], 1);
  if (!doc[PARAM_PASS2].isNull()) setWifiPass(doc[PARAM_PASS2], 1);
  if (!doc[PARAM_TEMPFORMAT].isNull()) {
    String s = doc[PARAM_TEMPFORMAT];
    setTempFormat(s.charAt(0));
  }
  if (!doc[PARAM_WEIGHT_PRECISION].isNull())
    setWeightPrecision(doc[PARAM_WEIGHT_PRECISION].as<int>());
  if (!doc[PARAM_BREWFATHER_APIKEY].isNull())
    setBrewfatherApiKey(doc[PARAM_BREWFATHER_APIKEY]);
  if (!doc[PARAM_BREWFATHER_USERKEY].isNull())
    setBrewfatherUserKey(doc[PARAM_BREWFATHER_USERKEY]);

  if (!doc[PARAM_SCALE_FACTOR1].isNull())
    setScaleFactor(0, doc[PARAM_SCALE_FACTOR1].as<float>());
  if (!doc[PARAM_SCALE_FACTOR2].isNull())
    setScaleFactor(1, doc[PARAM_SCALE_FACTOR2].as<float>());
  if (!doc[PARAM_SCALE_OFFSET1].isNull())
    setScaleOffset(0, doc[PARAM_SCALE_OFFSET1].as<float>());
  if (!doc[PARAM_SCALE_OFFSET2].isNull())
    setScaleOffset(1, doc[PARAM_SCALE_OFFSET2].as<float>());

  if (!doc[PARAM_KEG_WEIGHT1].isNull())
    setKegWeight(0, doc[PARAM_KEG_WEIGHT1].as<float>());
  if (!doc[PARAM_KEG_WEIGHT2].isNull())
    setKegWeight(1, doc[PARAM_KEG_WEIGHT2].as<float>());
  if (!doc[PARAM_PINT_WEIGHT1].isNull())
    setPintWeight(0, doc[PARAM_PINT_WEIGHT1].as<float>());
  if (!doc[PARAM_PINT_WEIGHT2].isNull())
    setPintWeight(1, doc[PARAM_PINT_WEIGHT2].as<float>());

  if (!doc[PARAM_BEER_NAME1].isNull()) setBeerName(0, doc[PARAM_BEER_NAME1]);
  if (!doc[PARAM_BEER_NAME2].isNull()) setBeerName(1, doc[PARAM_BEER_NAME2]);
  if (!doc[PARAM_BEER_EBC1].isNull())
    setBeerEBC(0, doc[PARAM_BEER_EBC1].as<int>());
  if (!doc[PARAM_BEER_EBC2].isNull())
    setBeerEBC(1, doc[PARAM_BEER_EBC2].as<int>());
  if (!doc[PARAM_BEER_ABV1].isNull())
    setBeerABV(0, doc[PARAM_BEER_ABV1].as<float>());
  if (!doc[PARAM_BEER_ABV2].isNull())
    setBeerABV(1, doc[PARAM_BEER_ABV2].as<float>());
  if (!doc[PARAM_BEER_IBU1].isNull())
    setBeerIBU(0, doc[PARAM_BEER_IBU1].as<int>());
  if (!doc[PARAM_BEER_IBU2].isNull())
    setBeerIBU(1, doc[PARAM_BEER_IBU2].as<int>());

  _saveNeeded = false;
  Log.notice(F("Cfg : Configuration file " CFG_FILENAME " loaded." CR));
  return true;
}

void Config::formatFileSystem() {
  Log.notice(F("Cfg : Formating filesystem." CR));
  LittleFS.format();
}

void Config::checkFileSystem() {
  // Log.verbose(F("Cfg : Checking if filesystem is valid." CR));

  if (LittleFS.begin()) {
    Log.notice(F("Cfg : Filesystem mounted." CR));
  } else {
    Log.error(F("Cfg : Unable to mount file system, formatting..." CR));
    LittleFS.format();
  }
}

// EOF
