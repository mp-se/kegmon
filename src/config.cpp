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
#include <wifi.hpp>
#include <resources.hpp>

Config myConfig;

Config::Config() {
  char buf[30];

  snprintf(&buf[0], sizeof(buf), "%06x", (unsigned int)ESP.getChipId());
  _id = String(&buf[0]);
  snprintf(&buf[0], sizeof(buf), "" WIFI_MDNS "%s", getID());
  _mDNS = String(&buf[0]);

  Log.verbose(F("CFG : Created config for %s (%s)." CR), _id.c_str(), _mDNS.c_str());
}

void Config::createJson(DynamicJsonDocument& doc) {
  doc[PARAM_MDNS] = getMDNS();
  doc[PARAM_ID] = getID();
  doc[PARAM_SSID] = getWifiSSID(0);
  doc[PARAM_PASS] = getWifiPass(0);
  doc[PARAM_SSID2] = getWifiSSID(1);
  doc[PARAM_PASS2] = getWifiPass(1);
  doc[PARAM_TEMPFORMAT] = String(getTempFormat());
}

bool Config::saveFile() {
  if (!_saveNeeded) {
    Log.verbose(F("CFG : Skipping save, not needed." CR));
    return true;
  }

  Log.verbose(F("CFG : Saving configuration to file." CR));

  File configFile = LittleFS.open(CFG_FILENAME, "w");

  if (!configFile) {
    Log.error(F("CFG : Failed to save configuration."));
    return false;
  }

  DynamicJsonDocument doc(512);
  createJson(doc);

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  serializeJson(doc, configFile);
  configFile.flush();
  configFile.close();

  _saveNeeded = false;
  Log.notice(F("CFG : Configuration saved to " CFG_FILENAME "." CR));
  return true;
}

bool Config::loadFile() {
  Log.verbose(F("CFG : Loading configuration from file." CR));

  if (!LittleFS.exists(CFG_FILENAME)) {
    Log.error(F("CFG : Configuration file does not exist."));
    return false;
  }

  File configFile = LittleFS.open(CFG_FILENAME, "r");

  if (!configFile) {
    Log.error(F("CFG : Failed to load configuration."));
    return false;
  }

  Log.notice(F("CFG : Size of configuration file=%d bytes." CR), configFile.size());

  DynamicJsonDocument doc(512);
  DeserializationError err = deserializeJson(doc, configFile);

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif

  configFile.close();

  if (err) {
    Log.error(F("CFG : Failed to parse configuration (json)"));
    return false;
  }

  Log.verbose(F("CFG : Parsed configuration file." CR));

  if (!doc[PARAM_MDNS].isNull()) setMDNS(doc[PARAM_MDNS]);
  if (!doc[PARAM_SSID].isNull()) setWifiSSID(doc[PARAM_SSID], 0);
  if (!doc[PARAM_PASS].isNull()) setWifiPass(doc[PARAM_PASS], 0);
  if (!doc[PARAM_SSID2].isNull()) setWifiSSID(doc[PARAM_SSID2], 1);
  if (!doc[PARAM_PASS2].isNull()) setWifiPass(doc[PARAM_PASS2], 1);
  if (!doc[PARAM_TEMPFORMAT].isNull()) {
    String s = doc[PARAM_TEMPFORMAT];
    setTempFormat(s.charAt(0));
  }

  _saveNeeded = false;
  Log.notice(F("CFG : Configuration file " CFG_FILENAME " loaded." CR));
  return true;
}

void Config::formatFileSystem() {
  Log.notice(F("CFG : Formating filesystem." CR));
  LittleFS.format();
}

void Config::checkFileSystem() {
  Log.verbose(F("CFG : Checking if filesystem is valid." CR));

  if (LittleFS.begin()) {
    Log.notice(F("CFG : Filesystem mounted." CR));
  } else {
    Log.error(F("CFG : Unable to mount file system, formatting..." CR));
    LittleFS.format();
  }
}

// EOF
