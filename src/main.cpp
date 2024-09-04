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
#include <displayout.hpp>
#include <kegconfig.hpp>
#include <kegpush.hpp>
#include <kegwebhandler.hpp>
#include <main.hpp>
#include <ota.hpp>
#include <perf.hpp>
#include <scale.hpp>
#include <serialws.hpp>
#include <temp_mgr.hpp>
#include <uptime.hpp>
#include <utils.hpp>
#include <wificonnection.hpp>
#if CONFIG_IDF_TARGET_ESP32S2
#include <esp32s2/rom/rtc.h>
#endif
#if CONFIG_IDF_TARGET_ESP32S3

#include <esp32s3/rom/rtc.h>
#endif
#if defined(ESP32)
#include <esp_core_dump.h>
#endif

SerialDebug mySerial(115200L);
KegConfig myConfig(CFG_MDNSNAME, CFG_FILENAME);
WifiConnection myWifi(&myConfig, CFG_APPNAME, "password", CFG_MDNSNAME);
OtaUpdate myOta(&myConfig, CFG_APPVER);
KegWebHandler myWebHandler(&myConfig);
KegPushHandler myPush(&myConfig);
Display myDisplay;
Scale myScale;
LevelDetection myLevelDetection;
TempSensorManager myTemp;
SerialWebSocket mySerialWebSocket;
DisplayLayout myDisplayLayout;

const int loopInterval = 2000;
int loopCounter = 0;
uint32_t loopMillis = 0;

RunMode runMode = RunMode::normalMode;

void scanI2C(int sda, int scl);
void logStartup();
void checkCoreDump();

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
  // see: rtc.h for reset reasons
  Log.notice(F("Main: Reset reason %d." CR), rtc_get_reset_reason(0));
  Log.notice(F("Core dump check %d." CR), esp_core_dump_image_check());

  char cbuf[30];
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  snprintf(&cbuf[0], sizeof(cbuf), "%6x", chipId);
  Log.notice(F("Main: Started setup for %s." CR), &cbuf[0]);
#endif
  Log.notice(F("Main: Build options: %s (%s) LOGLEVEL %d " CR), CFG_APPVER,
             CFG_GITREV, LOG_LEVEL);

  myConfig.checkFileSystem();
  myConfig.migrateSettings();

  PERF_BEGIN("setup-config");
  myConfig.loadFile();
  PERF_END("setup-config");

  delay(4000);

  Log.notice(F("Main: Initializing I2C bus #1 on pins SDA=%d, SCL=%d" CR),
             myConfig.getPinDisplayData(), myConfig.getPinDisplayClock());
#if defined(ESP8266)
  Wire.begin(myConfig.getPinDisplayData(), myConfig.getPinDisplayClock());
#else  // ESP32
  Wire.setPins(myConfig.getPinDisplayData(), myConfig.getPinDisplayClock());
  Wire.begin();
#endif
  Wire.setClock(400000);
#if !defined(ESP8266)
  Log.notice(F("Main: I2C bus clock=%d" CR), Wire.getClock());
#endif
  scanI2C(myConfig.getPinDisplayData(), myConfig.getPinDisplayClock());

  PERF_BEGIN("setup-display");
  myDisplay.setup();
  PERF_END("setup-display");
  PERF_BEGIN("setup-wifi");
  myWifi.init();
  PERF_END("setup-wifi");

  delay(200);

  PERF_BEGIN("setup-scale");
  myScale.setup();
  PERF_END("setup-scale");

#if defined(ESP8266)
  ESP.wdtDisable();
  ESP.wdtEnable(5000);
#endif

  // No stored config, move to portal
  if (!myWifi.hasConfig() || myWifi.isDoubleResetDetected()) {
    Log.notice(
        F("Main: Missing wifi config or double reset detected, entering wifi "
          "setup." CR));
    myDisplayLayout.showWifiPortal();
    myWifi.startAP();
    runMode = RunMode::wifiSetupMode;
  }

  switch (runMode) {
    case RunMode::normalMode:
      PERF_BEGIN("setup-wifi-connect");
      myWifi.connect();
      PERF_END("setup-wifi-connect");
      PERF_BEGIN("setup-timesync");
      myWifi.timeSync();
      PERF_END("setup-timesync");
      break;

    case RunMode::wifiSetupMode:
      break;
  }

  checkCoreDump();

  PERF_BEGIN("setup-webserver");
  myWebHandler.setupWebServer();
  mySerialWebSocket.begin(myWebHandler.getWebServer(), &EspSerial);
  mySerial.begin(&mySerialWebSocket);
  PERF_END("setup-webserver");

  PERF_BEGIN("setup-temp");
  myTemp.setup();
  PERF_END("setup-temp");

  Log.notice(F("Main: Setup completed." CR));
  myDisplayLayout.showStartupDevices(myScale.isConnected(UnitIndex::U1),
                                     myScale.isConnected(UnitIndex::U2),
                                     !isnan(myTemp.getLastTempC()));

  PERF_END("main-setup");
  PERF_PUSH();
  myTemp.read();
  delay(3000);
}

void loop() {
  if (!myWifi.isConnected() && runMode == RunMode::normalMode) myWifi.connect();

  myUptime.calculate();
  myWebHandler.loop();
  myWifi.loop();
  mySerialWebSocket.loop();
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

    // Try to reconnect to scales if they are missing (60 seconds)
    if (!(loopCounter % 10)) {
      printHeap("Loop:");
    }

    // The temp sensor should not be read too often.
    if (!(loopCounter % 15)) {
      myTemp.read();
      Log.notice(F("LOOP: Reading temperature=%F,humidity=%F,pressure=%F" CR),
                 myTemp.getLastTempC(), myTemp.getLastPressure(),
                 myTemp.getLastPressure());
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
    PERF_BEGIN("loop-display-default");
    myDisplayLayout.loop();
    myDisplayLayout.showCurrent(
        UnitIndex::U1, myScale.isConnected(UnitIndex::U1),
        myLevelDetection.getBeerWeight(UnitIndex::U1, LevelDetectionType::RAW),
        myLevelDetection.getBeerVolume(UnitIndex::U1, LevelDetectionType::RAW),
        myLevelDetection.getNoGlasses(UnitIndex::U1, LevelDetectionType::STATS),
        myLevelDetection.getPourVolume(UnitIndex::U1,
                                       LevelDetectionType::STATS),
        myTemp.getLastTempC(),
        myLevelDetection.hasStableWeight(UnitIndex::U1,
                                         LevelDetectionType::STATS));
    myDisplayLayout.showCurrent(
        UnitIndex::U2, myScale.isConnected(UnitIndex::U2),
        myLevelDetection.getBeerWeight(UnitIndex::U2, LevelDetectionType::RAW),
        myLevelDetection.getBeerVolume(UnitIndex::U2, LevelDetectionType::RAW),
        myLevelDetection.getNoGlasses(UnitIndex::U2, LevelDetectionType::STATS),
        myLevelDetection.getPourVolume(UnitIndex::U2,
                                       LevelDetectionType::STATS),
        myTemp.getLastTempC(),
        myLevelDetection.hasStableWeight(UnitIndex::U2,
                                         LevelDetectionType::STATS));
    PERF_END("loop-display-default");
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

    if (myConfig.hasTargetInfluxDb2()) {
      Log.notice(F("LOOP: Sending data to configured influxdb" CR));

      // This part is used to send data to an influxdb in order to get data on
      // scale stability/drift over time.
      char buf[250];

      float raw1 =
          myLevelDetection.getRawDetection(UnitIndex::U1)->getRawValue();
      float raw2 =
          myLevelDetection.getRawDetection(UnitIndex::U2)->getRawValue();
      float stb1 =
          myLevelDetection.getStatsDetection(UnitIndex::U1)->getStableValue();
      float stb2 =
          myLevelDetection.getStatsDetection(UnitIndex::U2)->getStableValue();

      String s;
      snprintf(&buf[0], sizeof(buf),
               "scale,host=%s,device=%s "
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

      if (!isnan(myTemp.getLastTempC())) {
        snprintf(&buf[0], sizeof(buf), ",tempC=%f,tempF=%f",
                 myTemp.getLastTempC(), myTemp.getLastTempF());
        s = s + &buf[0];
      }

      if (!isnan(myTemp.getLastHumidity())) {
        snprintf(&buf[0], sizeof(buf), ",humidity=%f",
                 myTemp.getLastHumidity());
        s = s + &buf[0];
      }

      if (!isnan(stb1)) {
        snprintf(&buf[0], sizeof(buf), ",stable1=%f", stb1);
        s = s + &buf[0];
      }

      if (!isnan(stb2)) {
        snprintf(&buf[0], sizeof(buf), ",stable2=%f", stb2);
        s = s + &buf[0];
      }

#if LOG_LEVEL == 6
      Log.verbose(F("LOOP: %s" CR), s.c_str());
#endif
      myPush.sendInfluxDb2(
          s, myConfig.getTargetInfluxDB2(), myConfig.getOrgInfluxDB2(),
          myConfig.getBucketInfluxDB2(), myConfig.getTokenInfluxDB2());
    }
  }
}

void scanI2C(int sda, int scl) {
  byte error, address;
  int n = 0;

  Log.notice(F("Main: Scanning I2C bus on pins %d:%d for devices: "), sda, scl);

  for (address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      EspSerial.print(address, HEX);
      EspSerial.print(",");
      n++;
    }
  }
  EspSerial.print("\n");
}

void logStartup() {
  struct tm timeinfo;
  time_t now = time(nullptr);
  char s[100];
  gmtime_r(&now, &timeinfo);

#if defined(ESP8266)
  snprintf(&s[0], sizeof(s),
           "%04d-%02d-%02d %02d:%02d:%02d;Starting up kegmon;%s\n",
           1900 + timeinfo.tm_year, 1 + timeinfo.tm_mon, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
           ESP.getResetInfo().c_str());
#else
  snprintf(&s[0], sizeof(s),
           "%04d-%02d-%02d %02d:%02d:%02d;Starting up kegmon;%d\n",
           1900 + timeinfo.tm_year, 1 + timeinfo.tm_mon, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
           rtc_get_reset_reason(0));
#endif

  File f = LittleFS.open(STARTUP_FILENAME, "a");

  if (f && f.size() > 2000) {
    f.close();
    LittleFS.remove(STARTUP_FILENAME);
    f = LittleFS.open(STARTUP_FILENAME, "a");
  }

  if (f) {
#if defined(ESP8266)
    f.write(&s[0], strlen(&s[0]));
#else
    f.write((unsigned char *)&s[0], strlen(&s[0]));
#endif
    f.close();
  }
}

void checkCoreDump() {
#if CONFIG_IDF_TARGET_ESP32S2
  esp_core_dump_summary_t *summary = static_cast<esp_core_dump_summary_t *>(
      malloc(sizeof(esp_core_dump_summary_t)));

  if (summary) {
    esp_log_level_set("esp_core_dump_elf", ESP_LOG_VERBOSE);

    if (esp_core_dump_get_summary(summary) == ESP_OK) {
      Log.notice(F("Exception cause %d." CR), summary->ex_info.exc_cause);
      Log.notice(F("PC 0x%x." CR), summary->exc_pc);

      for (int i = 0; i < summary->exc_bt_info.depth; i++) {
        Log.notice(F("PC(%d) 0x%x." CR), i, summary->exc_bt_info.bt[i]);
      }
    }
  }
#endif
}

// EOF
