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
#include <basepush.hpp>
#include <display.hpp>
#include <kegconfig.hpp>
#include <kegwebhandler.hpp>
#include <main.hpp>
#include <ota.hpp>
#include <scale.hpp>
#include <temp.hpp>
#include <utils.hpp>
#include <wificonnection.hpp>

SerialDebug mySerial(115200L);
KegConfig myConfig(CFG_MDNSNAME, CFG_FILENAME);
KegAdvancedConfig myAdvancedConfig;
WifiConnection myWifi(&myConfig, CFG_APPNAME, "password", CFG_MDNSNAME);
OtaUpdate myOta(&myConfig, CFG_APPVER);
KegWebHandler myWebHandler(&myConfig);
BasePush myPush(&myConfig);
Display myDisplay;
Scale myScale;
TempHumidity myTemp;

const int loopInterval = 2000;
int loopCounter = 0;
uint32_t loopMillis = 0;

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

void setup() {
  Log.notice(F("Main: Reset reason %s." CR), ESP.getResetInfo().c_str());
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
  myTemp.setup();

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
  myWebHandler.setupWebServer();
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
    // float weight = myScale.getLastValue(idx);
    float weight = myScale.hasLastStableValue(idx)
                       ? myScale.getLastStableValue(idx)
                       : myScale.getLastValue(idx);
    float pint = myScale.calculateNoPints(idx, weight);

    Log.notice(F("Loop: Weight=%F kg Pints=%f [%d]." CR), weight, pint, idx);

    snprintf(&buf[0], sizeof(buf), "%.1f%%", myConfig.getBeerABV(idx));
    myDisplay.printPosition(idx, -1, 16, &buf[0]);

    if (!showPints) {
      snprintf(&buf[0], sizeof(buf), "%.1f pints", pint);
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

  myWebHandler.loop();
  myWifi.loop();

  if (abs((int32_t)(millis() - loopMillis)) >
      loopInterval) {  // 2 seconds loop interval
    loopMillis = millis();
    loopCounter++;

    // Try to reconnect to scales if they are missing
    if (!(loopCounter % 30)) {
      if (!myScale.isConnected(UnitIndex::UNIT_1) ||
          !myScale.isConnected(UnitIndex::UNIT_2)) {
        myScale.setup();  // Try to reconnect to scale
      }
    }

    // Read the scales, only once per loop
    float weight1 = myScale.getValue(UnitIndex::UNIT_1, true);
    float weight2 = myScale.getValue(UnitIndex::UNIT_2, true);

    // Update screens
    drawScreen(UnitIndex::UNIT_1);
    drawScreen(UnitIndex::UNIT_2);

    Log.notice(F("Loop: Reading data scale1=%F, scale2=%F, lastStable1=%F, "
                 "lastStable2=%F,average1=%F,average2=%F" CR),
               weight1, weight2, myScale.getLastStableValue(UnitIndex::UNIT_1),
               myScale.getLastStableValue(UnitIndex::UNIT_2),
               myScale.statsAverage(UnitIndex::UNIT_1),
               myScale.statsAverage(UnitIndex::UNIT_2));

#if defined(ENABLE_INFLUX_DEBUG)
    // This part is used to send data to an influxdb in order to get data on
    // scale stability/drift over time.
    char buf[250];

    String s;
    snprintf(
        &buf[0], sizeof(buf),
        "scale,host=%s,device=%s "
        "raw-scale1=%d,raw-scale2=%d,scale1=%f,scale2=%f,pints1=%f,pints2=%f",
        myConfig.getMDNS(), myConfig.getID(),
        myScale.getRawValue(UnitIndex::UNIT_1),
        myScale.getRawValue(UnitIndex::UNIT_2), weight1, weight2,
        myScale.calculateNoPints(UnitIndex::UNIT_1, weight1),
        myScale.calculateNoPints(UnitIndex::UNIT_2, weight2));
    s = &buf[0];

    if (myScale.hasLastStableValue(UnitIndex::UNIT_1)) {
      snprintf(&buf[0], sizeof(buf), ",stable-scale1=%f",
               myScale.getLastStableValue(UnitIndex::UNIT_1));
      s += &buf[0];
    }

    if (myScale.hasLastStableValue(UnitIndex::UNIT_2)) {
      snprintf(&buf[0], sizeof(buf), ",stable-scale2=%f",
               myScale.getLastStableValue(UnitIndex::UNIT_2));
      s += &buf[0];
    }

    if (myScale.hasPourValue(UnitIndex::UNIT_1)) {
      snprintf(&buf[0], sizeof(buf), ",pour1=%f",
               myScale.getPourValue(UnitIndex::UNIT_1));
      s += &buf[0];
    }

    if (myScale.hasPourValue(UnitIndex::UNIT_2)) {
      snprintf(&buf[0], sizeof(buf), ",pour2=%f",
               myScale.getPourValue(UnitIndex::UNIT_2));
      s += &buf[0];
    }

    snprintf(&buf[0], sizeof(buf), ",count1=%d,count2=%d",
             static_cast<int>(myScale.statsCount(UnitIndex::UNIT_1)),
             static_cast<int>(myScale.statsCount(UnitIndex::UNIT_2)));
    s = s + &buf[0];

    if (myScale.statsCount(UnitIndex::UNIT_1) > 0) {
      snprintf(&buf[0], sizeof(buf),
               ",average1=%f,min1=%f,max1=%f,stdev1=%f,variance1=%f",
               myScale.statsAverage(UnitIndex::UNIT_1),
               myScale.statsMin(UnitIndex::UNIT_1),
               myScale.statsMax(UnitIndex::UNIT_1),
               myScale.statsPopStdev(UnitIndex::UNIT_1),
               myScale.statsVariance(UnitIndex::UNIT_1));
      s = s + &buf[0];
    }

    if (myScale.statsCount(UnitIndex::UNIT_2) > 0) {
      snprintf(&buf[0], sizeof(buf),
               ",average2=%f,min2=%f,max2=%f,stdev2=%f,variance2=%f",
               myScale.statsAverage(UnitIndex::UNIT_2),
               myScale.statsMin(UnitIndex::UNIT_2),
               myScale.statsMax(UnitIndex::UNIT_2),
               myScale.statsPopStdev(UnitIndex::UNIT_2),
               myScale.statsVariance(UnitIndex::UNIT_2));
      s = s + &buf[0];
    }

    Log.verbose(F("Loop: Sending data to influx: %s" CR), s.c_str());
    myPush.sendInfluxDb2(s);
#endif
  }
}

// EOF
