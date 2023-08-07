/*
MIT License

Copyright (c) 2023 Magnus

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
#include <display.hpp>
#include <kegconfig.hpp>
#include <kegpush.hpp>
#include <kegwebhandler.hpp>
#include <main.hpp>
#include <ota.hpp>
#include <perf.hpp>
#include <scale.hpp>
#include <serialws.hpp>
#include <temp_mgr.hpp>
#include <utils.hpp>
#include <wificonnection.hpp>

SerialDebug mySerial(115200L);
KegConfig myConfig(CFG_MDNSNAME, CFG_FILENAME);
WifiConnection myWifi(&myConfig, CFG_APPNAME, "password", CFG_MDNSNAME);
Display myDisplay;
Scale myScale;
LevelDetection myLevelDetection;
TempSensorManager myTemp;

const int loopInterval = 2000;
int loopCounter = 0;
uint32_t loopMillis = 0;

void setup() {
#if defined(ESP8266)
  Log.notice(F("Main: Reset reason %s." CR), ESP.getResetInfo().c_str());
  Log.notice(F("Main: Started setup for %s." CR),
             String(ESP.getChipId(), HEX).c_str());
#else
  char cbuf[20];
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  snprintf(&cbuf[0], sizeof(cbuf), "%6x", chipId);
  Log.notice(F("Main: Started setup for %s." CR), &cbuf[0]);
#endif
  Log.notice(F("Main: Build options: %s (%s) LOGLEVEL %d " CR), CFG_APPVER,
             CFG_GITREV, LOG_LEVEL);

  PERF_BEGIN("setup-display");
  myDisplay.setup();
  PERF_END("setup-display");
  myConfig.checkFileSystem();

  myConfig.loadFile();

  // Set a scale factor so we get values
  myConfig.setScaleFactor(UnitIndex::U1, 1); 
  myConfig.setScaleFactor(UnitIndex::U2, 1);
  myConfig.setScaleOffset(UnitIndex::U1, 1); 
  myConfig.setScaleOffset(UnitIndex::U2, 1); 

  myScale.setup();

  myTemp.setup();

#if defined(ESP8266)
  ESP.wdtDisable();
  ESP.wdtEnable(5000);
#endif

  Log.notice(F("Main: Setup completed." CR));
  myTemp.read();
}

void draw(UnitIndex idx, float temp, float scale1, float scale2, int32_t raw1, int32_t raw2) {
  myDisplay.clear(idx);
  myDisplay.setFont(idx, FontSize::FONT_10);

  char buf[30];

  snprintf(&buf[0], sizeof(buf), "Temp   : %.3f", temp);
  myDisplay.printLine(idx, 0, &buf[0]);

  snprintf(&buf[0], sizeof(buf), "Scale 1: %.3f", scale1);
  myDisplay.printLine(idx, 1, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Scale 2: %.3f", scale2);
  myDisplay.printLine(idx, 2, &buf[0]);

  snprintf(&buf[0], sizeof(buf), "Raw   1: %d", raw1);
  myDisplay.printLine(idx, 3, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Raw   2: %d", raw2);
  myDisplay.printLine(idx, 4, &buf[0]);

  myDisplay.show(idx);
}


void loop() {
  myScale.loop(UnitIndex::U1);
  myScale.loop(UnitIndex::U2);

  if (abs((int32_t)(millis() - loopMillis)) >
      loopInterval) {  // 2 seconds loop interval
    loopMillis = millis();
    loopCounter++;

    // Try to reconnect to scales if they are missing (6 seconds)
    if (!(loopCounter % 3)) {
      if (!myScale.isConnected(UnitIndex::U1) ||
          !myScale.isConnected(UnitIndex::U2)) {
        myScale.setup();  // Try to reconnect to scale
      }
    }

    // The temp sensor should not be read too often. Reading every 4 seconds.
    if (!(loopCounter % 1)) {
      myTemp.read();
    }

    // Check if the temp sensor exist and try to reinitialize
    if (!(loopCounter % 3)) {
      if (!myTemp.hasSensor()) {
        myTemp.reset();
        myTemp.setup();
      }
    }

    float t = myTemp.getLastTempC();
    float s1 = myScale.read(UnitIndex::U1, true);
    float s2 = myScale.read(UnitIndex::U2, true);
    int32_t l1 = myScale.readLastRaw(UnitIndex::U1);
    int32_t l2 = myScale.readLastRaw(UnitIndex::U2);

    draw(UnitIndex::U1, t, s1, s2, l1, l2);
    draw(UnitIndex::U2, t, s1, s2, l1, l2);
  }
}

// EOF
