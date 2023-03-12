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
#include <display.hpp>
#include <kegconfig.hpp>
#include <kegpush.hpp>
#include <kegwebhandler.hpp>
#include <main.hpp>
#include <ota.hpp>
#include <perf.hpp>
#include <scale.hpp>
#include <temp.hpp>
#include <utils.hpp>
#include <wificonnection.hpp>

SerialDebug mySerial(115200L);
KegConfig myConfig(CFG_MDNSNAME, CFG_FILENAME);
WifiConnection myWifi(&myConfig, CFG_APPNAME, "password", CFG_MDNSNAME);
OtaUpdate myOta(&myConfig, CFG_APPVER);
KegWebHandler myWebHandler(&myConfig);
KegPushHandler myPush(&myConfig);
Display myDisplay;
Scale myScale;
LevelDetection myLevelDetection;
TempHumidity myTemp;

const int loopInterval = 2000;
int loopCounter = 0;
uint32_t loopMillis = 0;

void scanI2C(int sda, int scl);

void setup() {
#if defined(PERF_ENABLE)
  PerfLogging perf;
  perf.getInstance().setBaseConfig(&myConfig);
#endif

  PERF_BEGIN("setup");
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

  /*scanI2C(PIN_OLED_SDA, PIN_OLED_SCL);
  scanI2C(PIN_SCALE1_SDA, PIN_SCALE1_SCL);
  scanI2C(PIN_SCALE2_SDA, PIN_SCALE2_SCL);*/

  PERF_BEGIN("setup-display");
  myDisplay.setup(UnitIndex::U1);
  myDisplay.setup(UnitIndex::U2);
  PERF_END("setup-display");
  myConfig.checkFileSystem();

  // Temporary for migrating due to namechange of configuration file.
  // @TODO Remove this code in next version
  File f = LittleFS.open(CFG_FILENAME, "r");
  if (!f)
    LittleFS.rename("/kegscale.json", CFG_FILENAME);
  else
    f.close();
  // End

  PERF_BEGIN("setup-config");
  myConfig.loadFile();
  PERF_END("setup-config");
  PERF_BEGIN("setup-wifi");
  myWifi.init();
  PERF_END("setup-wifi");
  PERF_BEGIN("setup-scale");
  myScale.setup();
  PERF_END("setup-scale");
  PERF_BEGIN("setup-temp");
  myTemp.setup();
  PERF_END("setup-temp");

#if defined(ESP8266)
  ESP.wdtDisable();
  ESP.wdtEnable(5000);
#endif

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

  PERF_BEGIN("setup-wifi-connect");
  myWifi.connect();
  PERF_END("setup-wifi-connect");
  PERF_BEGIN("setup-timesync");
  myWifi.timeSync();
  PERF_END("setup-timesync");

  PERF_BEGIN("setup-webserver");
#if defined(USE_ASYNC_WEB)
  myWebHandler.setupAsyncWebServer();
#else
  myWebHandler.setupWebServer();
#endif
  PERF_END("setup-webserver");

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
           !isnan(myTemp.getLastTempC()) ? "Yes" : "No");
  myDisplay.printLine(UnitIndex::U1, 2, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Version: %s", CFG_APPVER);
  myDisplay.printLine(UnitIndex::U1, 3, &buf[0]);
  snprintf(&buf[0], sizeof(buf), "Push: %s",
#if defined(ENABLE_INFLUX_DEBUG)
           "Yes");
#else
           "No");
#endif
  myDisplay.printLine(UnitIndex::U1, 4, &buf[0]);
  myDisplay.show(UnitIndex::U1);

  PERF_END("main-setup");
  PERF_PUSH();
  myTemp.read();
  delay(3000);
}

void drawScreenHardwareStats(UnitIndex idx) {
  myDisplay.clear(idx);
  myDisplay.setFont(idx, FontSize::FONT_10);

  if (myScale.isConnected(idx)) {
    char buf[30];

    snprintf(&buf[0], sizeof(buf), "Last wgt: %.3f",
             myLevelDetection.getTotalWeight(idx));
    myDisplay.printLine(idx, 0, &buf[0]);
    snprintf(&buf[0], sizeof(buf), "Stab wgt: %.3f",
             myLevelDetection.getTotalStableWeight(idx));
    myDisplay.printLine(idx, 1, &buf[0]);
    snprintf(&buf[0], sizeof(buf), "Ave  wgt: %.3f",
             myLevelDetection.getStatsDetection(idx)->ave());
    myDisplay.printLine(idx, 2, &buf[0]);
    snprintf(&buf[0], sizeof(buf), "Min wgt: %.3f",
             myLevelDetection.getStatsDetection(idx)->min());
    myDisplay.printLine(idx, 3, &buf[0]);
    snprintf(&buf[0], sizeof(buf), "Max wgt: %.3f",
             myLevelDetection.getStatsDetection(idx)->max());
    myDisplay.printLine(idx, 4, &buf[0]);
    snprintf(&buf[0], sizeof(buf), "Temp: %.3f", myTemp.getLastTempC());
    myDisplay.printLine(idx, 5, &buf[0]);
  }

  myDisplay.show(idx);
}

enum ScreenDefaultIter {
  ShowWeight = 0,
  ShowGlasses = 1,
  ShowPour = 2,
  ShowTemp = 3
};

ScreenDefaultIter defaultScreenIter[2] = {ScreenDefaultIter::ShowWeight,
                                          ScreenDefaultIter::ShowWeight};

void drawScreenDefault(UnitIndex idx) {
  myDisplay.clear(idx);
  myDisplay.setFont(idx, FontSize::FONT_16);

  char buf[20];

  snprintf(&buf[0], sizeof(buf), "%s", myConfig.getBeerName(idx));
  myDisplay.printPosition(idx, -1, 0, &buf[0]);

  if (!(loopCounter % 2)) {
    switch (defaultScreenIter[idx]) {
      case ScreenDefaultIter::ShowWeight:
        defaultScreenIter[idx] = ScreenDefaultIter::ShowGlasses;
        break;
      case ScreenDefaultIter::ShowGlasses:
        defaultScreenIter[idx] = ScreenDefaultIter::ShowPour;
        break;
      case ScreenDefaultIter::ShowPour:
        defaultScreenIter[idx] = ScreenDefaultIter::ShowWeight;
        break;
      case ScreenDefaultIter::ShowTemp:
        defaultScreenIter[idx] = ScreenDefaultIter::ShowTemp;
        break;
    }
  }

  if (myScale.isConnected(idx)) {
    snprintf(&buf[0], sizeof(buf), "%.1f%%", myConfig.getBeerABV(idx));
    myDisplay.printPosition(idx, -1, 16, &buf[0]);

    switch (defaultScreenIter[idx]) {
      case ScreenDefaultIter::ShowWeight: {
        convertFloatToString(
            myLevelDetection.getBeerWeight(idx, LevelDetectionType::RAW),
            &buf[0], myConfig.getWeightPrecision());
        String s(&buf[0]);
        s += " " + String(myConfig.getWeightUnit());
        myDisplay.printPosition(idx, -1, 32, s.c_str());
      } break;

      case ScreenDefaultIter::ShowGlasses: {
        float glass =
            myLevelDetection.getNoGlasses(idx, LevelDetectionType::STATS);
        snprintf(&buf[0], sizeof(buf), "%.1f glasses", glass);
        myDisplay.printPosition(idx, -1, 32, &buf[0]);
      } break;

      case ScreenDefaultIter::ShowPour: {
        float pour =
            myLevelDetection.getPourVolume(idx, LevelDetectionType::STATS);
        // if (isnan(pour)) pour = 0.0;
        snprintf(&buf[0], sizeof(buf), "%.0f pour", pour * 100);
        myDisplay.printPosition(idx, -1, 32, &buf[0]);
      } break;

      case ScreenDefaultIter::ShowTemp: {
      } break;
    }

  } else {
    myDisplay.printPosition(idx, -1, 32, "No scale");
  }

  myDisplay.setFont(idx, FontSize::FONT_10);

  switch (defaultScreenIter[idx]) {
    case ScreenDefaultIter::ShowWeight:
      myDisplay.printPosition(
          idx, -1,
          myDisplay.getHeight(idx) - myDisplay.getCurrentFontSize(idx) - 1,
          myLevelDetection.hasStableWeight(idx, LevelDetectionType::STATS)
              ? "stable level"
              : "searching level");
      break;
    case ScreenDefaultIter::ShowGlasses:
      if (idx == UnitIndex::U1) {
        myDisplay.printPosition(
            idx, -1,
            myDisplay.getHeight(idx) - myDisplay.getCurrentFontSize(idx) - 1,
            WiFi.SSID());
      }
      break;
    case ScreenDefaultIter::ShowPour:
      if (idx == UnitIndex::U1) {
        myDisplay.printPosition(
            idx, -1,
            myDisplay.getHeight(idx) - myDisplay.getCurrentFontSize(idx) - 1,
            myWifi.getIPAddress());
      }
      break;
    case ScreenDefaultIter::ShowTemp:
      break;
  }

  myDisplay.show(idx);
}

void loop() {
  if (!myWifi.isConnected()) myWifi.connect();

  myWebHandler.loop();
  myWifi.loop();
  myScale.loop(UnitIndex::U1);
  myScale.loop(UnitIndex::U2);

  if (abs((int32_t)(millis() - loopMillis)) >
      loopInterval) {  // 2 seconds loop interval
    loopMillis = millis();
    loopCounter++;

    // Send updates to push targets at regular intervals (300 seconds / 5min)
    if (!(loopCounter % 300)) {
      myPush.pushTempInformation(myTemp.getLastTempC(), true);

      if (myLevelDetection.hasStableWeight(UnitIndex::U1))
        myPush.pushKegInformation(
            UnitIndex::U1, myLevelDetection.getBeerStableVolume(UnitIndex::U1),
            myLevelDetection.getPourVolume(UnitIndex::U1),
            myLevelDetection.getNoStableGlasses(UnitIndex::U1), true);

      if (myLevelDetection.hasStableWeight(UnitIndex::U2))
        myPush.pushKegInformation(
            UnitIndex::U2, myLevelDetection.getBeerStableVolume(UnitIndex::U2),
            myLevelDetection.getPourVolume(UnitIndex::U2),
            myLevelDetection.getNoStableGlasses(UnitIndex::U2), true);
    }

    // Try to reconnect to scales if they are missing (60 seconds)
    if (!(loopCounter % 30)) {
      if (!myScale.isConnected(UnitIndex::U1) ||
          !myScale.isConnected(UnitIndex::U2)) {
        myScale.setup();  // Try to reconnect to scale
      }
    }

    // The temp sensor should not be read too often. Reading every 10 seconds.
    if (!(loopCounter % 5)) {
      myTemp.read();
    }

    // Check if the temp sensor exist and try to reinitialize
    if (!(loopCounter % 10)) {
      if (!myTemp.hasSensor()) {
        myTemp.reset();
        myTemp.setup();
      }
    }

    // Read the scales, only once per loop
    float t = myTemp.getLastTempC();

    PERF_BEGIN("loop-scale-read1");
    myLevelDetection.update(UnitIndex::U1, myScale.read(UnitIndex::U1), t);
    PERF_END("loop-scale-read1");
    PERF_BEGIN("loop-scale-read2");
    myLevelDetection.update(UnitIndex::U2, myScale.read(UnitIndex::U2), t);
    PERF_END("loop-scale-read2");

    // Update screens
    switch (myConfig.getDisplayLayoutType()) {
      default:
      case DisplayLayoutType::Default:
        PERF_BEGIN("loop-display-default");
        drawScreenDefault(UnitIndex::U1);
        drawScreenDefault(UnitIndex::U2);
        PERF_END("loop-display-default");
        break;

      case DisplayLayoutType::HardwareStats:
        PERF_BEGIN("loop-display-hardware");
        drawScreenHardwareStats(UnitIndex::U1);
        drawScreenHardwareStats(UnitIndex::U2);
        PERF_END("loop-display-hardware");
        break;
    }

    PERF_PUSH();

    /*Log.notice(
        F("LOOP: Reading data raw1=%F,raw2=%F,kalman1=%F,kalman2=%F,stab1=%F, "
          "stab2=%F,ave1=%F,ave2=%F,min1=%F,min2=%F,max1=%F,max2=%F,pour1=%F,"
          "pour2=%F" CR),
        myScale.getTotalRawWeight(UnitIndex::U1),
        myScale.getTotalRawWeight(UnitIndex::U2),
        myScale.getTotalWeight(UnitIndex::U1),
        myScale.getTotalWeight(UnitIndex::U2),
        myScale.getTotalStableWeight(UnitIndex::U1),
        myScale.getTotalStableWeight(UnitIndex::U2),
        myScale.getStatsDetection(UnitIndex::U1)->ave(),
        myScale.getStatsDetection(UnitIndex::U2)->ave(),
        myScale.getStatsDetection(UnitIndex::U1)->min(),
        myScale.getStatsDetection(UnitIndex::U2)->min(),
        myScale.getStatsDetection(UnitIndex::U1)->max(),
        myScale.getStatsDetection(UnitIndex::U2)->max(),
        myScale.getPourWeight(UnitIndex::U1),
        myScale.getPourWeight(UnitIndex::U2));*/
    Log.notice(
        F("LOOP: Reading data raw1=%F,raw2=%F,stable1=%F, "
          "stable2=%F,pour1=%F,"
          "pour2=%F" CR),
        myLevelDetection.getRawDetection(UnitIndex::U1)->getRawValue(),
        myLevelDetection.getRawDetection(UnitIndex::U2)->getRawValue(),
        myLevelDetection.getStatsDetection(UnitIndex::U1)->getStableValue(),
        myLevelDetection.getStatsDetection(UnitIndex::U2)->getStableValue(),
        myLevelDetection.getStatsDetection(UnitIndex::U1)->getPourValue(),
        myLevelDetection.getStatsDetection(UnitIndex::U2)->getPourValue());

#if defined(ENABLE_INFLUX_DEBUG)
    // This part is used to send data to an influxdb in order to get data on
    // scale stability/drift over time.
    char buf[250];

    float raw1 = myLevelDetection.getRawDetection(UnitIndex::U1)->getRawValue();
    float raw2 = myLevelDetection.getRawDetection(UnitIndex::U2)->getRawValue();

    String s;
    snprintf(&buf[0], sizeof(buf),
             "debug,host=%s,device=%s "
             "level-raw1=%f,"
             "level-raw2=%f",
             myConfig.getMDNS(), myConfig.getID(), isnan(raw1) ? 0 : raw1,
             isnan(raw2) ? 0 : raw2);
    s = &buf[0];

    float ave1 =
        myLevelDetection.getRawDetection(UnitIndex::U1)->getAverageValue();
    float ave2 =
        myLevelDetection.getRawDetection(UnitIndex::U2)->getAverageValue();

    snprintf(&buf[0], sizeof(buf), ",level-average1=%f,level-average2=%f",
             isnan(ave1) ? 0 : ave1, isnan(ave2) ? 0 : ave2);
    s += &buf[0];

    float kal1 =
        myLevelDetection.getRawDetection(UnitIndex::U1)->getKalmanValue();
    float kal2 =
        myLevelDetection.getRawDetection(UnitIndex::U2)->getKalmanValue();

    snprintf(&buf[0], sizeof(buf), ",level-kalman1=%f,level-kalman2=%f",
             isnan(kal1) ? 0 : kal1, isnan(kal2) ? 0 : kal2);
    s += &buf[0];

    float stats1 =
        myLevelDetection.getStatsDetection(UnitIndex::U1)->getStableValue();
    float stats2 =
        myLevelDetection.getStatsDetection(UnitIndex::U2)->getStableValue();

    snprintf(&buf[0], sizeof(buf), ",level-stats1=%f,level-stats2=%f",

             isnan(stats1) ? 0 : stats1, isnan(stats2) ? 0 : stats2);
    s += &buf[0];

    if (!isnan(myTemp.getTempC())) {
      snprintf(&buf[0], sizeof(buf), ",tempC=%f,tempF=%f,humidity=%f",
               myTemp.getTempC(), myTemp.getTempF(), myTemp.getHumidity());
      s = s + &buf[0];
    }

#if LOG_LEVEL == 6
    Log.verbose(F("LOOP: %s" CR), s.c_str());
#endif
    myPush.sendInfluxDb2(s, PUSH_INFLUX_TARGET, PUSH_INFLUX_ORG,
                         PUSH_INFLUX_BUCKET, PUSH_INFLUX_TOKEN);
#endif  // ENABLE_INFLUX_DEBUG
  }
}

void scanI2C(int sda, int scl) {
#if defined(ESP8266)
  Wire.begin(sda, scl);
#else  // ESP32
  Wire.setPins(sda, scl);
  Wire.begin();
#endif

  byte error, address;
  int n = 0;

  Log.notice(F("Scanning I2C bus on pins %d:%d for devices: "), sda, scl);

  for (address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      EspSerial.print(address, HEX);
      EspSerial.print("\t");
      n++;
    }
  }
  EspSerial.print("\n");
#if defined(ESP32)
  Wire.end();
#endif
}

// EOF
