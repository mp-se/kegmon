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
#ifndef SRC_KEGWEBHANDLER_HPP_
#define SRC_KEGWEBHANDLER_HPP_

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32S2)
#include <WiFi.h>
#endif

#if defined(ESP8266) && defined(USE_ASYNC_WEB)
#include <baseasyncwebhandler.hpp>
#elif defined(ESP8266)
#include <ESP8266WebServer.h>

#include <basewebhandler.hpp>
#elif defined(ESP32S2) && defined(USE_ASYNC_WEB)
#include <WebServer.h>

#include <baseasyncwebhandler.hpp>
#elif defined(ESP32S2)
#include <WebServer.h>

#include <basewebhandler.hpp>
#endif

#include <kegconfig.hpp>

#if defined(ESP8266)
#include <incbin.h>
INCBIN_EXTERN(CalibrateHtm);
INCBIN_EXTERN(BeerHtm);
INCBIN_EXTERN(StabilityHtm);
INCBIN_EXTERN(GraphHtm);
INCBIN_EXTERN(BackupHtm);
INCBIN_EXTERN(BrewpiHtm);
INCBIN_EXTERN(DashboardHtm);
#else
extern const uint8_t calibrationHtmStart[] asm(
    "_binary_html_calibration_min_htm_start");
extern const uint8_t calibrationHtmEnd[] asm(
    "_binary_html_calibration_min_htm_end");
extern const uint8_t beerHtmStart[] asm("_binary_html_beer_min_htm_start");
extern const uint8_t beerHtmEnd[] asm("_binary_html_beer_min_htm_end");
extern const uint8_t stabilityHtmStart[] asm(
    "_binary_html_stability_min_htm_start");
extern const uint8_t stabilityHtmEnd[] asm(
    "_binary_html_stability_min_htm_end");
extern const uint8_t graphHtmStart[] asm("_binary_html_graph_min_htm_start");
extern const uint8_t graphHtmEnd[] asm("_binary_html_graph_min_htm_end");
extern const uint8_t backupHtmStart[] asm("_binary_html_backup_min_htm_start");
extern const uint8_t backupHtmEnd[] asm("_binary_html_backup_min_htm_end");
extern const uint8_t brewpiHtmStart[] asm("_binary_html_brewpi_min_htm_start");
extern const uint8_t brewpiHtmEnd[] asm("_binary_html_brewpi_min_htm_end");
extern const uint8_t dashboardHtmStart[] asm(
    "_binary_html_dashboard_min_htm_start");
extern const uint8_t dashboardHtmEnd[] asm(
    "_binary_html_dashboard_min_htm_end");
#endif

#if defined(USE_ASYNC_WEB)
#define WS_BIND_URL(url, http, func) \
  _server->on(url, http, std::bind(func, this, std::placeholders::_1))
#define WS_PARAM AsyncWebServerRequest* request
#define WS_SEND_STATIC(ptr, size) \
  request->send_P(200, "text/html", (const uint8_t*)ptr, size)
#define WS_REQ_ARG(key) request->arg(key)
#define WS_REQ_ARG_NAME(idx) request->argName(idx)
#define WS_REQ_ARG_CNT() request->args()
#define WS_REQ_HAS_ARG(key) request->hasArg(key)
#define WS_SEND(code, type, text)                                              \
  AsyncWebServerResponse* response = request->beginResponse(code, type, text); \
  response->addHeader("Access-Control-Allow-Origin", "*");                     \
  request->send(response);
#else
#define WS_BIND_URL(url, http, func) \
  _server->on(url, http, std::bind(func, this))
#define WS_PARAM
#define WS_SEND_STATIC(ptr, size) \
  _server->send_P(200, "text/html", (const char*)ptr, size)
#define WS_REQ_ARG(key) _server->arg(key)
#define WS_REQ_ARG_NAME(idx) _server->argName(idx)
#define WS_REQ_ARG_CNT() _server->args()
#define WS_REQ_HAS_ARG(key) _server->hasArg(key)
#define WS_SEND(code, type, text) \
  _server->enableCORS(true);      \
  _server->send(code, type, text)
#endif

class KegWebHandler :
#if defined(USE_ASYNC_WEB)
    public BaseAsyncWebHandler
#else
    public BaseWebHandler
#endif
{
 protected:
  KegConfig* _config;

  void setupWebHandlers();
  void setupAsyncWebHandlers();
  void populateScaleJson(DynamicJsonDocument& doc);
  void webScale(WS_PARAM);
  void webScaleTare(WS_PARAM);
  void webScaleFactor(WS_PARAM);
  void webConfigGet(WS_PARAM);
  void webConfigPost(WS_PARAM);
  void webStatus(WS_PARAM);
  void webStability(WS_PARAM);
  void webStabilityClear(WS_PARAM);
  void webReset(WS_PARAM);
  void webHandleBeerWrite(WS_PARAM);
  void webHandleBrewspy(WS_PARAM);
  void webHandleLogsClear(WS_PARAM);

#if defined(ESP8266)
  void webCalibrateHtm(WS_PARAM) {
    WS_SEND_STATIC(gCalibrateHtmData, gCalibrateHtmSize);
  }
  void webBeerHtm(WS_PARAM) { WS_SEND_STATIC(gBeerHtmData, gBeerHtmSize); }
  void webStabilityHtm(WS_PARAM) {
    WS_SEND_STATIC(gStabilityHtmData, gStabilityHtmSize);
  }
  void webGraphHtm(WS_PARAM) { WS_SEND_STATIC(gGraphHtmData, gGraphHtmSize); }
  void webBackupHtm(WS_PARAM) {
    WS_SEND_STATIC(gBackupHtmData, gBackupHtmSize);
  }
  void webDashboardHtm(WS_PARAM) {
    WS_SEND_STATIC(gDashboardHtmData, gDashboardHtmSize);
  }
#else
  void webCalibrateHtm(WS_PARAM) {
    WS_SEND_STATIC(
        (const char*)calibrationHtmStart,
        strlen(reinterpret_cast<const char*>(&calibrationHtmStart[0])));
  }
  void webBeerHtm(WS_PARAM) {
    WS_SEND_STATIC((const char*)beerHtmStart,
                   strlen(reinterpret_cast<const char*>(&beerHtmStart[0])));
  }
  void webStabilityHtm(WS_PARAM) {
    WS_SEND_STATIC(
        (const char*)stabilityHtmStart,
        strlen(reinterpret_cast<const char*>(&stabilityHtmStart[0])));
  }
  void webGraphHtm(WS_PARAM) {
    WS_SEND_STATIC((const char*)graphHtmStart,
                   strlen(reinterpret_cast<const char*>(&graphHtmStart[0])));
  }
  void webBackupHtm(WS_PARAM) {
    WS_SEND_STATIC((const char*)backupHtmStart,
                   strlen(reinterpret_cast<const char*>(&backupHtmStart[0])));
  }
  void webDashboardHtm(WS_PARAM) {
    WS_SEND_STATIC(
        (const char*)dashboardHtmStart,
        strlen(reinterpret_cast<const char*>(&dashboardHtmStart[0])));
  }
#endif

 public:
  explicit KegWebHandler(KegConfig* config);
};

#endif  // SRC_KEGWEBHANDLER_HPP_

// EOF
