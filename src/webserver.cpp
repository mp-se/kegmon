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
  Log.notice(F("WEB : webServer callback /api/scale/tare." CR));
  myScale.tare();

  DynamicJsonDocument doc(256);
  populateScaleJson(doc);

  String out;
  out.reserve(256);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

void WebServerHandler::webScaleFactor() {
  float weight = _server->arg(PARAM_WEIGHT).toFloat();
  Log.notice(F("WEB : webServer callback /api/scale/factor, weight=%F." CR), weight);

  myScale.findFactor(weight);

  DynamicJsonDocument doc(256);
  populateScaleJson(doc);

  String out;
  out.reserve(256);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

void WebServerHandler::populateScaleJson(DynamicJsonDocument& doc) {
  doc[PARAM_SCALE_FACTOR] = myConfig.getScaleFactor();
  doc[PARAM_SCALE_WEIGHT] = myScale.getValue();
  doc[PARAM_SCALE_RAW] = myScale.getRawValue();
  doc[PARAM_SCALE_OFFSET] = myConfig.getScaleOffset();

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
  if (_server->hasArg(PARAM_KEG_WEIGHT)) myConfig.setKegWeight(_server->arg(PARAM_KEG_WEIGHT).toFloat());
  if (_server->hasArg(PARAM_PINT_WEIGHT)) myConfig.setPintWeight(_server->arg(PARAM_PINT_WEIGHT).toFloat());

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
  doc[PARAM_PINTS] = myScale.calculateNoPints();

  String out;
  out.reserve(1024);
  serializeJson(doc, out);
  _server->send(200, "application/json", out.c_str());
}

// EOF
