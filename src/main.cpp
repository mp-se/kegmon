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
#include <display.hpp>
#include <main.hpp>
#include <scale.hpp>
#include <temp.hpp>
#include <webserver.hpp>
#include <wifi.hpp>

SerialDebug mySerial;
const int loopInterval = 2000;
int loopCounter = 0;
uint32_t loopMillis = 0;

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
  Log.notice(
      F("%s: Free-heap %d kb, Heap-rag %d %%, Max-block %d kb Stack=%d b." CR),
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

  Log.notice(F("Main: Started setup for %s." CR),
             String(ESP.getChipId(), HEX).c_str());
  Log.notice(F("Main: Build options: %s (%s) LOGLEVEL %d " CR), CFG_APPVER,
             CFG_GITREV, LOG_LEVEL);

  myDisplay.setup(UnitIndex::UNIT_1);
  myDisplay.setup(UnitIndex::UNIT_2);
  myConfig.checkFileSystem();
  myConfig.loadFile();
  myWifi.init();
  myScale.setup();

  ESP.wdtDisable();
  ESP.wdtEnable(5000);

  // No stored config, move to portal
  if (!myWifi.hasConfig() || myWifi.isDoubleResetDetected()) {
    Log.notice(
        F("Main: Missing wifi config or double reset detected, entering wifi "
          "setup." CR));

    myDisplay.clear(UnitIndex::UNIT_1);
    myDisplay.setFont(UnitIndex::UNIT_1, FontSize::FONT_16);
    myDisplay.printLineCentered(UnitIndex::UNIT_1, 0, "Entering");
    myDisplay.printLineCentered(UnitIndex::UNIT_1, 1, "WIFI Portal");
    myDisplay.printLineCentered(UnitIndex::UNIT_1, 2, "192.168.4.1");
    myDisplay.show(UnitIndex::UNIT_1);
    myWifi.startPortal();
  }

  myWifi.connect();
  myWebServerHandler.setupWebServer();
  Log.notice(F("Main: Setup completed." CR));

  // Show what sensors has been detected on display 1
  char buf[30];

  myDisplay.clear(UnitIndex::UNIT_1);
  myDisplay.setFont(UnitIndex::UNIT_1, FontSize::FONT_16);
  snprintf(&buf[0], sizeof(buf), "Scale 1: %s",
           myScale.isConnected(UnitIndex::UNIT_1) ? "Yes" : "No");
  myDisplay.printLine(UnitIndex::UNIT_1, 0, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Scale 2: %s",
           myScale.isConnected(UnitIndex::UNIT_2) ? "Yes" : "No");
  myDisplay.printLine(UnitIndex::UNIT_1, 1, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Temp   : %s",
           !isnan(myTemp.getTempValueC()) ? "Yes" : "No");
  myDisplay.printLine(UnitIndex::UNIT_1, 2, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Version: %s", CFG_APPVER);
  myDisplay.printLine(UnitIndex::UNIT_1, 3, &buf[0]);
  myDisplay.show(UnitIndex::UNIT_1);
  delay(3000);
}

bool showPints = false;

void drawScreen(UnitIndex idx) {
  myDisplay.clear(idx);
  myDisplay.setFont(idx, FontSize::FONT_16);

  char buf[20];

  snprintf(&buf[0], sizeof(buf), "%s", myConfig.getBeerName(idx));
  myDisplay.printPosition(idx, -1, 0, &buf[0]);

  if (!(loopCounter % 10)) showPints = !showPints;

  if (myScale.isConnected(idx)) {
    float weight = myScale.getValue(idx);  // Just read the scale once per loop.
    int pint = myScale.calculateNoPints(idx, weight);

    Log.notice(F("Loop: Weight=%F kg Pints=%d [%d]." CR), weight, pint, idx);

    snprintf(&buf[0], sizeof(buf), "%.1f%%", myConfig.getBeerABV(idx));
    myDisplay.printPosition(idx, -1, 16, &buf[0]);

    if (!showPints) {
      snprintf(&buf[0], sizeof(buf), "%d pints", pint);
      myDisplay.printPosition(idx, -1, 32, &buf[0]);
    } else {
      convertFloatToString(weight, &buf[0], myConfig.getWeightPrecision());
      String s(&buf[0]);
      s += " kg";
      myDisplay.printPosition(idx, -1, 32, s.c_str());
    }
  } else {
    myDisplay.printPosition(idx, -1, 32, "No scale");
  }

  // Lets draw the footer here (only on display 1).
  if (idx == UnitIndex::UNIT_1) {
    myDisplay.setFont(idx, FontSize::FONT_10);
    if (!showPints)
      myDisplay.printPosition(
          idx, -1,
          myDisplay.getHeight(idx) - myDisplay.getCurrentFontSize(idx) - 1,
          myWifi.getIPAddress());
    else
      myDisplay.printPosition(
          idx, -1,
          myDisplay.getHeight(idx) - myDisplay.getCurrentFontSize(idx) - 1,
          WiFi.SSID());
  }

  myDisplay.show(idx);
}

void loop() {
  if (!myWifi.isConnected()) myWifi.connect();

  myWebServerHandler.loop();
  myWifi.loop();

  if (abs((int32_t)(millis() - loopMillis)) > loopInterval) {
    loopMillis = millis();
    loopCounter++;

    if (!(loopCounter % 30))
      if (!myScale.isConnected(UnitIndex::UNIT_1) ||
          !myScale.isConnected(UnitIndex::UNIT_2))
        myScale.setup();  // Try to reconnect to scale

    drawScreen(UnitIndex::UNIT_1);
    drawScreen(UnitIndex::UNIT_2);
  }
}

// EOF
