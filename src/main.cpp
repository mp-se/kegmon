/*
MIT License

Copyright (c) 2022 Magnus

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
#include <display.hpp>
#include <scale.hpp>
#include <webserver.hpp>

SerialDebug mySerial;
const int loopInterval = 1000; 

void printTimestamp(Print* _logOutput, int _logLevel) {
  char c[12];
  snprintf(c, sizeof(c), "%10lu ", millis());
  _logOutput->print(c);
}

SerialDebug::SerialDebug(const uint32_t serialSpeed) {
  Serial.flush();
  Serial.begin(serialSpeed);

  getLog()->begin(LOG_LEVEL, &Serial, true);
  getLog()->setPrefix(printTimestamp);
  getLog()->notice(F("SDBG: Serial logging started at %u." CR), serialSpeed);
}

void printHeap(String prefix = "Main") {
  Log.notice(F("%s: Free-heap %d kb, Heap-rag %d %%, Max-block %d kb Stack=%d b." CR),
      prefix.c_str(), ESP.getFreeHeap() / 1024, ESP.getHeapFragmentation(),
      ESP.getMaxFreeBlockSize() / 1024, ESP.getFreeContStack());
}

char* convertFloatToString(float f, char* buffer, int dec) {
  dtostrf(f, 6, dec, buffer);
  return buffer;
}

float reduceFloatPrecision(float f, int dec) {
  char buffer[5];
  dtostrf(f, 6, dec, &buffer[0]);
  return atof(&buffer[0]);
}

void setup() {
  // Log.verbose(F("Main: Reset reason %s." CR), ESP.getResetInfo().c_str());

  Log.notice(F("Main: Started setup for %s." CR), String(ESP.getChipId(), HEX).c_str());
  Log.notice(F("Main: Build options: %s (%s) LOGLEVEL %d "CR), CFG_APPVER, CFG_GITREV, LOG_LEVEL);

  myDisplay.setup();
  myConfig.checkFileSystem();
  myConfig.loadFile();
  myWifi.init();
  myScale.setup();

  ESP.wdtDisable();
  ESP.wdtEnable(5000);

  // No stored config, move to portal
  if (!myWifi.hasConfig() || myWifi.isDoubleResetDetected()) {
    Log.notice(F("Main: Missing wifi config or double reset detected, entering wifi setup." CR));

    myDisplay.clear();
    myDisplay.setFont(FontSize::FONT_16);
    myDisplay.printLineCentered(0, "Entering");
    myDisplay.printLineCentered(1, "WIFI Portal");
    myDisplay.printLineCentered(2, "192.168.4.1");
    myDisplay.show();
    myWifi.startPortal();
  }

  myWifi.connect();
  myWebServerHandler.setupWebServer();
  Log.notice(F("Main: Setup completed." CR));
}

uint32_t loopMillis = 0;
int loopCounter = 0;

void loop() {
  // TODO: Check if wifi is lost and do reconnect.
  myWebServerHandler.loop();
  myWifi.loop();

  if (abs((int32_t)(millis() - loopMillis)) > loopInterval) {
    loopMillis = millis();
    loopCounter++;

    char buf[20];
    int pint = myScale.calculateNoPints();
    convertFloatToString(myScale.getValue(), &buf[0], myConfig.getWeightPrecision());
    Log.verbose(F("Loop: Reading scale and updating display weight=%s pints=%d." CR), &buf[0], pint);

    myDisplay.clear();
    myDisplay.setFont(FontSize::FONT_16);
    myDisplay.printLineCentered(0, CFG_APPNAME);
    myDisplay.printLineCentered(1, &buf[0]);
    snprintf(&buf[0], sizeof(buf), "%d pints", pint);
    myDisplay.printLineCentered(2, &buf[0]);

    if (!(loopCounter % 8)) {
      myDisplay.setFont(FontSize::FONT_10);
      myDisplay.printLineCentered(5, myWifi.getIPAddress());
    } else if (!(loopCounter % 12)) {
      myDisplay.setFont(FontSize::FONT_10);
      myDisplay.printLineCentered(5, WiFi.SSID());
    }
    myDisplay.show();
  }
}

// EOF
