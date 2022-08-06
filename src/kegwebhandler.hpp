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

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <incbin.h>

#include <basewebhandler.hpp>
#include <kegconfig.hpp>

INCBIN_EXTERN(CalibrateHtm);
INCBIN_EXTERN(BeerHtm);
INCBIN_EXTERN(StabilityHtm);

class KegWebHandler : public BaseWebHandler {
 private:
  KegConfig* _config;

  void setupWebHandlers();
  void populateScaleJson(DynamicJsonDocument& doc);
  void webScale();
  void webScaleTare();
  void webScaleFactor();
  void webConfigGet();
  void webConfigPost();
  void webStatus();
  void webStability();
  void webStabilityClear();
  void webReset();

  void webCalibrateHtm() {
    _server->send_P(200, "text/html", (const char*)gCalibrateHtmData,
                    gCalibrateHtmSize);
  }
  void webBeerHtm() {
    _server->send_P(200, "text/html", (const char*)gBeerHtmData, gBeerHtmSize);
  }
  void webStabilityHtm() {
    _server->send_P(200, "text/html", (const char*)gStabilityHtmData,
                    gStabilityHtmSize);
  }

 public:
  explicit KegWebHandler(KegConfig* config);
};

#endif  // SRC_KEGWEBHANDLER_HPP_

// EOF
