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

  myDisplay.setup(UnitIndex::U1);
  myDisplay.setup(UnitIndex::U2);
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

    myDisplay.clear(UnitIndex::U1);
    myDisplay.setFont(UnitIndex::U1, FontSize::FONT_16);
    myDisplay.printLineCentered(UnitIndex::U1, 0, "Entering");
    myDisplay.printLineCentered(UnitIndex::U1, 1, "WIFI Portal");
    myDisplay.printLineCentered(UnitIndex::U1, 2, "192.168.4.1");
    myDisplay.show(UnitIndex::U1);
    myWifi.startPortal();
  }

  myWifi.connect();
  myWebHandler.setupWebServer();
  Log.notice(F("Main: Setup completed." CR));

  // Show what sensors has been detected on display 1
  char buf[30];

  myDisplay.clear(UnitIndex::U1);
  myDisplay.setFont(UnitIndex::U1, FontSize::FONT_10);
  snprintf(&buf[0], sizeof(buf), "Scale 1: %s",
           myScale.isConnected(UnitIndex::U1) ? "Yes" : "No");
  myDisplay.printLine(UnitIndex::U1, 0, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Scale 2: %s",
           myScale.isConnected(UnitIndex::U2) ? "Yes" : "No");
  myDisplay.printLine(UnitIndex::U1, 1, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Temp : %s",
           !isnan(myTemp.getTempC()) ? "Yes" : "No");
  myDisplay.printLine(UnitIndex::U1, 2, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Version: %s", CFG_APPVER);
  myDisplay.printLine(UnitIndex::U1, 3, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Push: %s",
           strlen(myConfig.getTargetInfluxDB2()) > 0 ? "Yes" : "No");
  myDisplay.printLine(UnitIndex::U1, 4, &buf[0]);
  myDisplay.show(UnitIndex::U1);
  delay(3000);
}

bool showPints = false;

void drawScreenHardwareStats(UnitIndex idx) {
  myDisplay.clear(idx);
  myDisplay.setFont(idx, FontSize::FONT_10);

  if (myScale.isConnected(idx)) {
    char buf[30];

    snprintf(&buf[0], sizeof(buf), "Last wgt: %.3f",
             myScale.getLastWeight(idx));
    myDisplay.printLine(idx, 0, &buf[0]);
    snprintf(&buf[0], sizeof(buf), "Stab wgt: %.3f",
             myScale.getLastStableWeight(idx));
    myDisplay.printLine(idx, 1, &buf[0]);
    snprintf(&buf[0], sizeof(buf), "Ave  wgt: %.3f",
             myScale.getAverageWeight(idx));
    myDisplay.printLine(idx, 2, &buf[0]);
    snprintf(&buf[0], sizeof(buf), "Stat min: %.3f", myScale.statsMin(idx));
    myDisplay.printLine(idx, 3, &buf[0]);
    snprintf(&buf[0], sizeof(buf), "Stat max: %.3f", myScale.statsMax(idx));
    myDisplay.printLine(idx, 4, &buf[0]);
    snprintf(&buf[0], sizeof(buf), "Stat dev: %.3f",
             myScale.statsPopStdev(idx));
    myDisplay.printLine(idx, 5, &buf[0]);
  }
  myDisplay.show(idx);
}

void drawScreenDefault(UnitIndex idx) {
  myDisplay.clear(idx);
  myDisplay.setFont(idx, FontSize::FONT_16);

  char buf[20];

  snprintf(&buf[0], sizeof(buf), "%s", myConfig.getBeerName(idx));
  myDisplay.printPosition(idx, -1, 0, &buf[0]);

  if (!(loopCounter % 10)) showPints = !showPints;

  if (myScale.isConnected(idx)) {
    // float weight = myScale.getLastValue(idx);
    float weight = myScale.hasLastStableWeight(idx)
                       ? myScale.getLastStableWeight(idx)
                       : myScale.getLastWeight(idx);
    float glass = myScale.calculateNoGlasses(idx);

    Log.notice(F("Loop: Weight=%F Glasses=%F [%d]." CR), weight, glass, idx);

    snprintf(&buf[0], sizeof(buf), "%.1f%%", myConfig.getBeerABV(idx));
    myDisplay.printPosition(idx, -1, 16, &buf[0]);

    if (!showPints) {
      snprintf(&buf[0], sizeof(buf), "%.1f glasses", glass);
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
  if (idx == UnitIndex::U1) {
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
      if (!myScale.isConnected(UnitIndex::U1) ||
          !myScale.isConnected(UnitIndex::U2)) {
        myScale.setup();  // Try to reconnect to scale
      }
    }

    // Read the scales, only once per loop
    float weight1 = myScale.readWeight(UnitIndex::U1, true);
    float weight2 = myScale.readWeight(UnitIndex::U2, true);

    // Update screens
    switch (myConfig.getDisplayLayout()) {
      default:
      case DisplayLayout::Default:
        drawScreenDefault(UnitIndex::U1);
        drawScreenDefault(UnitIndex::U2);
        break;

      case DisplayLayout::HardwareStats:
        drawScreenHardwareStats(UnitIndex::U1);
        drawScreenHardwareStats(UnitIndex::U2);
        break;
    }

    Log.notice(F("Loop: Reading data scale1=%F, scale2=%F, lastStable1=%F, "
                 "lastStable2=%F,average1=%F,average2=%F" CR),
               weight1, weight2, myScale.getLastStableWeight(UnitIndex::U1),
               myScale.getLastStableWeight(UnitIndex::U2),
               myScale.statsAverage(UnitIndex::U1),
               myScale.statsAverage(UnitIndex::U2));

#if defined(ENABLE_INFLUX_DEBUG)
    // This part is used to send data to an influxdb in order to get data on
    // scale stability/drift over time.
    char buf[250];

    String s;
    snprintf(&buf[0], sizeof(buf),
             "scale,host=%s,device=%s "
             "scale1=%f,scale2=%f,glass1=%f,glass2=%f",
             myConfig.getMDNS(), myConfig.getID(), weight1, weight2,
             myScale.calculateNoGlasses(UnitIndex::U1),
             myScale.calculateNoGlasses(UnitIndex::U2));
    s = &buf[0];

    if (myScale.hasLastStableWeight(UnitIndex::U1)) {
      snprintf(&buf[0], sizeof(buf), ",stable-scale1=%f",
               myScale.getLastStableWeight(UnitIndex::U1));
      s += &buf[0];
    }

    if (myScale.hasLastStableWeight(UnitIndex::U2)) {
      snprintf(&buf[0], sizeof(buf), ",stable-scale2=%f",
               myScale.getLastStableWeight(UnitIndex::U2));
      s += &buf[0];
    }

    if (myScale.hasPourWeight(UnitIndex::U1)) {
      snprintf(&buf[0], sizeof(buf), ",pour1=%f",
               myScale.getPourWeight(UnitIndex::U1));
      s += &buf[0];
    }

    if (myScale.hasPourWeight(UnitIndex::U2)) {
      snprintf(&buf[0], sizeof(buf), ",pour2=%f",
               myScale.getPourWeight(UnitIndex::U2));
      s += &buf[0];
    }

    snprintf(&buf[0], sizeof(buf), ",count1=%d,count2=%d",
             static_cast<int>(myScale.statsCount(UnitIndex::U1)),
             static_cast<int>(myScale.statsCount(UnitIndex::U2)));
    s = s + &buf[0];

    if (myScale.statsCount(UnitIndex::U1) > 0) {
      snprintf(&buf[0], sizeof(buf),
               ",average1=%f,min1=%f,max1=%f,stdev1=%f,variance1=%f",
               myScale.statsAverage(UnitIndex::U1),
               myScale.statsMin(UnitIndex::U1), myScale.statsMax(UnitIndex::U1),
               myScale.statsPopStdev(UnitIndex::U1),
               myScale.statsVariance(UnitIndex::U1));
      s = s + &buf[0];
    }

    if (myScale.statsCount(UnitIndex::U2) > 0) {
      snprintf(&buf[0], sizeof(buf),
               ",average2=%f,min2=%f,max2=%f,stdev2=%f,variance2=%f",
               myScale.statsAverage(UnitIndex::U2),
               myScale.statsMin(UnitIndex::U2), myScale.statsMax(UnitIndex::U2),
               myScale.statsPopStdev(UnitIndex::U2),
               myScale.statsVariance(UnitIndex::U2));
      s = s + &buf[0];
    }

    if (!isnan(myTemp.getTempC())) {
      snprintf(&buf[0], sizeof(buf), ",tempC=%f,tempF=%f,humidity=%f",
               myTemp.getTempC(), myTemp.getTempF(), myTemp.getHumidity());
      s = s + &buf[0];
    }

    Log.verbose(F("Loop: Sending data to influx: %s" CR), s.c_str());
    myPush.sendInfluxDb2(s);
#endif
  }
}

// EOF
