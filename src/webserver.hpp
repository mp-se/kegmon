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
#ifndef SRC_WEBSERVER_HPP_
#define SRC_WEBSERVER_HPP_

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <incbin.h>

INCBIN_EXTERN(IndexHtm);
INCBIN_EXTERN(ConfigHtm);
INCBIN_EXTERN(CalibrateHtm);
INCBIN_EXTERN(AboutHtm);
INCBIN_EXTERN(UploadHtm);

class WebServerHandler {
 private:
  ESP8266WebServer* _server = 0;
  File _uploadFile;

  void populateScaleJson(DynamicJsonDocument& doc);

  void webScale();
  void webScaleTare();
  void webScaleFactor();
  void webConfigGet();
  void webConfigPost();
  void webStatus();
  void webUpload();

  void webIndexHtm() {
    _server->send_P(200, "text/html", (const char*)gIndexHtmData,
                    gIndexHtmSize);
  }
  void webConfigHtm() {
    _server->send_P(200, "text/html", (const char*)gConfigHtmData,
                    gConfigHtmSize);
  }
  void webCalibrateHtm() {
    _server->send_P(200, "text/html", (const char*)gCalibrateHtmData,
                    gCalibrateHtmSize);
  }
  void webAboutHtm() {
    _server->send_P(200, "text/html", (const char*)gAboutHtmData,
                    gAboutHtmSize);
  }
  void webUploadHtm() {
    _server->send_P(200, "text/html", (const char*)gUploadHtmData,
                    gUploadHtmSize);
  }
  void webReturnOK() { _server->send(200); }

 public:
  bool setupWebServer();
  void loop();
};

extern WebServerHandler myWebServerHandler;

#endif  // SRC_WEBSERVER_HPP_

// EOF
