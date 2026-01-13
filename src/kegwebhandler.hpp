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
#ifndef SRC_KEGWEBHANDLER_HPP_
#define SRC_KEGWEBHANDLER_HPP_
#include <FS.h>
#include <LittleFS.h>
#include <WiFi.h>

#include <atomic>
#include <basewebserver.hpp>
#include <changedetection.hpp>
#include <kegconfig.hpp>

class KegWebHandler : public BaseWebServer {
 protected:
  KegConfig *_config;
  volatile bool _hardwareScanTask = false;

  String _hardwareScanData;

  // Ringbuffer for recent events (last 10)
  static constexpr size_t RECENT_EVENTS_SIZE = 10;
  ChangeDetectionEvent _recentEvents[RECENT_EVENTS_SIZE];
  std::atomic<size_t> _eventHead = 0;  // Write position
  std::atomic<size_t> _eventCount = 0;  // Number of events stored
  mutable portMUX_TYPE _eventLock = portMUX_INITIALIZER_UNLOCKED;  // Thread safety

  void setupWebHandlers();

  void webFeature(AsyncWebServerRequest *request);
  void webScale(AsyncWebServerRequest *request);
  void webScaleTare(AsyncWebServerRequest *request, JsonVariant &json);
  void webScaleFactor(AsyncWebServerRequest *request, JsonVariant &json);
  void webHardwareScan(AsyncWebServerRequest *request);
  void webHardwareScanStatus(AsyncWebServerRequest *request);
  void webConfigGet(AsyncWebServerRequest *request);
  void webConfigPost(AsyncWebServerRequest *request, JsonVariant &json);
  void webStatus(AsyncWebServerRequest *request);
  void webStatistic(AsyncWebServerRequest *request);
  void webStatisticClear(AsyncWebServerRequest *request);
  void webHandleBrewspy(AsyncWebServerRequest *request, JsonVariant &json);
  void webHandleFactoryDefaults(AsyncWebServerRequest *request);

 public:
  explicit KegWebHandler(KegConfig *config);

  void loop();
  
  // Queue an event for publishing in status endpoint
  void queueEvent(const ChangeDetectionEvent& event);
  
  // Get recent events (called by status endpoint)
  void getRecentEvents(ChangeDetectionEvent* outEvents, size_t& count);
};

#endif  // SRC_KEGWEBHANDLER_HPP_

// EOF
