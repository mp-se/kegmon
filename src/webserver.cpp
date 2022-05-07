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
#include <webserver.hpp>
#include <wifi.hpp>
#include <scale.hpp>

WebServerHandler myWebServerHandler; 

bool WebServerHandler::setupWebServer() {
  Log.notice(F("WEB : Configuring web server." CR));
  _server = new ESP8266WebServer();
  MDNS.begin(myConfig.getMDNS());
  MDNS.addService("http", "tcp", 80);

  FSInfo fs;
  LittleFS.info(fs);
  Log.notice(F("WEB : File system Total=%d, Used=%d." CR), fs.totalBytes, fs.usedBytes);
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    Log.notice(F("WEB : File=%s, %d bytes" CR), dir.fileName().c_str(), dir.fileSize());
  }

  _server->on("/api/scale", HTTP_GET, std::bind(&WebServerHandler::webScale, this));
  _server->on("/api/scale/tare", HTTP_GET, std::bind(&WebServerHandler::webScaleTare, this));
  _server->on("/api/scale/factor", HTTP_GET, std::bind(&WebServerHandler::webScaleFactor, this));
  _server->on("/api/config", HTTP_GET, std::bind(&WebServerHandler::webConfigGet, this));
  _server->on("/api/config", HTTP_POST, std::bind(&WebServerHandler::webConfigPost, this));
  _server->on("/api/status", HTTP_GET, std::bind(&WebServerHandler::webStatus, this));

  _server->on("/", HTTP_GET, std::bind(&WebServerHandler::webIndexHtm, this));
  _server->on("/index.htm", HTTP_GET, std::bind(&WebServerHandler::webIndexHtm, this));
  _server->on("/config.htm", HTTP_GET, std::bind(&WebServerHandler::webConfigHtm, this));
  _server->on("/calibration.htm", HTTP_GET, std::bind(&WebServerHandler::webCalibrateHtm, this));
  _server->on("/about.htm", HTTP_GET, std::bind(&WebServerHandler::webAboutHtm, this));
  _server->on("/upload.htm", HTTP_GET, std::bind(&WebServerHandler::webUploadHtm, this));

  _server->on("/api/upload", HTTP_POST, std::bind(&WebServerHandler::webReturnOK, this), std::bind(&WebServerHandler::webUpload,this));

  _server->begin();
  Log.notice(F("WEB : Web server started." CR));
  return true;
}

void WebServerHandler::loop() {
  MDNS.update();
  _server->handleClient();
}

void WebServerHandler::webScale() {
  Log.notice(F("WEB : webServer callback /api/scale." CR));

  DynamicJsonDocument doc(256);
  populateScaleJson(doc);

  String out;
  out.reserve(256);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

void WebServerHandler::webScaleTare() {
  int idx = _server->arg(PARAM_SCALE).toInt();

  Log.notice(F("WEB : webServer callback /api/scale/tare." CR));
  if (idx == 1)
    myScale.tare(UnitIndex::UNIT_1);
  else
    myScale.tare(UnitIndex::UNIT_2);

  DynamicJsonDocument doc(256);
  populateScaleJson(doc);

  String out;
  out.reserve(256);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

void WebServerHandler::webScaleFactor() {
  float weight = _server->arg(PARAM_WEIGHT).toFloat();
  int idx = _server->arg(PARAM_SCALE).toInt();
  Log.notice(F("WEB : webServer callback /api/scale/factor, weight=%F [%d]." CR), weight, idx);

  if (idx == 1)
    myScale.findFactor(UnitIndex::UNIT_1, weight);
  else
    myScale.findFactor(UnitIndex::UNIT_2, weight);

  DynamicJsonDocument doc(256);
  populateScaleJson(doc);

  String out;
  out.reserve(256);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

void WebServerHandler::populateScaleJson(DynamicJsonDocument& doc) {
  doc[PARAM_SCALE_FACTOR1] = myConfig.getScaleFactor(0);
  doc[PARAM_SCALE_FACTOR2] = myConfig.getScaleFactor(1);
  doc[PARAM_SCALE_WEIGHT1] = myScale.getValue(UnitIndex::UNIT_1);
  doc[PARAM_SCALE_WEIGHT2] = myScale.getValue(UnitIndex::UNIT_2);
  doc[PARAM_SCALE_RAW1] = myScale.getRawValue(UnitIndex::UNIT_1);
  doc[PARAM_SCALE_RAW2] = myScale.getRawValue(UnitIndex::UNIT_2);
  doc[PARAM_SCALE_OFFSET1] = myConfig.getScaleOffset(0);
  doc[PARAM_SCALE_OFFSET2] = myConfig.getScaleOffset(1);

#if LOG_LEVEL == 6
  serializeJson(doc, Serial);
  Serial.print(CR);
#endif
}

void WebServerHandler::webConfigGet() {
  Log.notice(F("WEB : webServer callback /api/config(get)." CR));

  DynamicJsonDocument doc(1024);
  myConfig.createJson(doc);

  doc[PARAM_PASS] = "**hidden**";
  doc[PARAM_PASS2] = "**hidden**";

  String out;
  out.reserve(1024);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

void WebServerHandler::webConfigPost() {
  Log.notice(F("WEB : webServer callback /api/config(post)." CR));

  if (_server->hasArg(PARAM_MDNS)) myConfig.setMDNS(_server->arg(PARAM_MDNS));
  // if (_server->hasArg(PARAM_SSID)) myConfig.setWifiSSID(_server->arg(PARAM_SSID), 0);
  // if (_server->hasArg(PARAM_PASS)) myConfig.setWifiPass(_server->arg(PARAM_PASS), 0);
  // if (_server->hasArg(PARAM_SSID2)) myConfig.setWifiSSID(_server->arg(PARAM_SSID2), 1);
  // if (_server->hasArg(PARAM_PASS2)) myConfig.setWifiPass(_server->arg(PARAM_PASS2), 1);
  if (_server->hasArg(PARAM_TEMPFORMAT)) myConfig.setTempFormat(_server->arg(PARAM_TEMPFORMAT).charAt(0));
  if (_server->hasArg(PARAM_WEIGHT_PRECISION)) myConfig.setWeightPrecision(_server->arg(PARAM_WEIGHT_PRECISION).toInt());
  if (_server->hasArg(PARAM_BREWFATHER_APIKEY)) myConfig.setBrewfatherApiKey(_server->arg(PARAM_BREWFATHER_APIKEY));
  if (_server->hasArg(PARAM_BREWFATHER_USERKEY)) myConfig.setBrewfatherUserKey(_server->arg(PARAM_BREWFATHER_USERKEY));

  if (_server->hasArg(PARAM_KEG_WEIGHT1)) myConfig.setKegWeight(0, _server->arg(PARAM_KEG_WEIGHT1).toFloat());
  if (_server->hasArg(PARAM_KEG_WEIGHT2)) myConfig.setKegWeight(1, _server->arg(PARAM_KEG_WEIGHT2).toFloat());
  if (_server->hasArg(PARAM_PINT_WEIGHT1)) myConfig.setPintWeight(0, _server->arg(PARAM_PINT_WEIGHT1).toFloat());
  if (_server->hasArg(PARAM_PINT_WEIGHT2)) myConfig.setPintWeight(1, _server->arg(PARAM_PINT_WEIGHT2).toFloat());

  if (_server->hasArg(PARAM_BEER_NAME1)) myConfig.setBeerName(0, _server->arg(PARAM_BEER_NAME1));
  if (_server->hasArg(PARAM_BEER_NAME2)) myConfig.setBeerName(1, _server->arg(PARAM_BEER_NAME2));
  if (_server->hasArg(PARAM_BEER_EBC1)) myConfig.setBeerEBC(0, _server->arg(PARAM_BEER_EBC1).toInt());
  if (_server->hasArg(PARAM_BEER_EBC2)) myConfig.setBeerEBC(1, _server->arg(PARAM_BEER_EBC2).toInt());
  if (_server->hasArg(PARAM_BEER_ABV1)) myConfig.setBeerABV(0, _server->arg(PARAM_BEER_ABV1).toFloat());
  if (_server->hasArg(PARAM_BEER_ABV2)) myConfig.setBeerABV(1, _server->arg(PARAM_BEER_ABV2).toFloat());
  if (_server->hasArg(PARAM_BEER_IBU1)) myConfig.setBeerIBU(0, _server->arg(PARAM_BEER_IBU1).toInt());
  if (_server->hasArg(PARAM_BEER_IBU2)) myConfig.setBeerIBU(1, _server->arg(PARAM_BEER_IBU2).toInt());


  myConfig.saveFile();
  _server->sendHeader("Location", "/config.htm", true);
  _server->send(302, "text/plain", "Config updated");
}

void WebServerHandler::webStatus() {
  Log.notice(F("WEB : webServer callback /api/status." CR));

  DynamicJsonDocument doc(1024);
  populateScaleJson(doc);

  doc[PARAM_MDNS] = myConfig.getMDNS();
  doc[PARAM_ID] = myConfig.getID();
  doc[PARAM_SSID] = myConfig.getWifiSSID(0);
  doc[PARAM_APP_VER] = CFG_APPVER;
  doc[PARAM_APP_BUILD] = CFG_GITREV;

  doc[PARAM_PINTS1] = myScale.calculateNoPints(UnitIndex::UNIT_1);
  doc[PARAM_PINTS2] = myScale.calculateNoPints(UnitIndex::UNIT_2);

  String out;
  out.reserve(1024);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

void WebServerHandler::webUpload() {
  Log.verbose(F("WEB : webServer callback for /api/upload." CR));
  HTTPUpload& upload = _server->upload();
  String f = upload.filename;
  bool validFilename = false;

  Log.verbose(F("WEB : webServer callback for /api/upload, receiving file %s, %d(%d)." CR), f.c_str(), upload.currentSize, upload.totalSize);

  #define MAX_SKETCH_SPACE 1044464  

  // Handle firmware update, hardcode since function return wrong value.
  // (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  uint32_t maxSketchSpace = MAX_SKETCH_SPACE;

  if (upload.status == UPLOAD_FILE_START) {
    Log.notice(F("WEB : Start firmware upload, max sketch size %d kb." CR), maxSketchSpace / 1024);

    if (!Update.begin(maxSketchSpace, U_FLASH, PIN_LED)) {
        Log.error(F("WEB : Not enough space to store for this firmware." CR));
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      Log.notice(F("WEB : Writing firmware upload %d (%d)." CR), upload.totalSize, maxSketchSpace);

      if (upload.totalSize > maxSketchSpace) {
        Log.error(F("WEB : Firmware file is to large." CR));
      } else if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Log.warning(F("WEB : Firmware write was unsuccessful." CR));
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      Log.notice(F("WEB : Finish firmware upload." CR));
      if (Update.end(true)) {
        _server->send(200);
        delay(500);
        ESP_RESET();
      } else {
        Log.error(F("WEB : Failed to finish firmware flashing error=%d" CR), Update.getError());
      }
    } else {
      Update.end();
      Log.notice(F("WEB : Firmware flashing aborted." CR));
    }

    delay(0);
}

// EOF
