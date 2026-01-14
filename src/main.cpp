/*
MIT License

Copyright (c) 2022-2026 Magnus

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
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <changedetection.hpp>
#include <display.hpp>
#include <kegconfig.hpp>
#include <kegpush.hpp>
#include <kegwebhandler.hpp>
#include <looptimer.hpp>
#include <main.hpp>
#include <ota.hpp>
#include <scale.hpp>
#include <sdcard_mmc.hpp>
#include <sdcard_sd.hpp>
#include <serialws.hpp>
#include <temp_mgr.hpp>
#include <uptime.hpp>
#include <utils.hpp>
#include <wificonnection.hpp>
#if defined(ESP32S3)
#include <esp32s3/rom/rtc.h>
#endif
#include <esp_core_dump.h>

#include <cstdio>

void checkCoreDump();

SerialDebug mySerial(115200L);
KegConfig myConfig(CFG_MDNSNAME, CFG_FILENAME);
WifiConnection myWifi(&myConfig, CFG_APPNAME, "password", CFG_MDNSNAME);
OtaUpdate myOta(&myConfig, CFG_APPVER);
KegWebHandler myWebHandler(&myConfig);
KegPushHandler myPush(&myConfig);
Display myDisplay;
TempSensorManager myTemp;
SerialWebSocket mySerialWebSocket;
Scale myScale;
ChangeDetection myChangeDetection;
LoopTimer sdTimer(30 * 1000);
LoopTimer displayTimer(100);
LoopTimer loopTimer(2000);
RunMode runMode = RunMode::normalMode;
#if defined(ENABLE_MMC)
SdCardMMC mySdStorage;
#elif defined(ENABLE_SD)
SdCardSD mySdStorage;
#endif

/**
 * Background task for scale reading and change detection updates
 * Runs on Core 1:
 *  - Reads all scales continuously and updates change detection state machine
 *  - Runs as fast as possible (limited only by HX711 sensor readiness)
 */
void scaleDetectionTask(void *parameter) {
  Log.notice(F("ScaleTask: Started on core %d" CR), xPortGetCoreID());

  // Fire system startup event to mark the beginning of a monitoring session
  myChangeDetection.fireStartupEvent(millis());

  while (true) {
    uint32_t currentTimeMs = millis();

    ScaleReadingResult res1 = myScale.read(UnitIndex::U1);
    myChangeDetection.update(UnitIndex::U1, res1, currentTimeMs);
    ScaleReadingResult res2 = myScale.read(UnitIndex::U2);
    myChangeDetection.update(UnitIndex::U2, res2, currentTimeMs);
    ScaleReadingResult res3 = myScale.read(UnitIndex::U3);
    myChangeDetection.update(UnitIndex::U3, res3, currentTimeMs);
    ScaleReadingResult res4 = myScale.read(UnitIndex::U4);
    myChangeDetection.update(UnitIndex::U4, res4, currentTimeMs);

    // Process any scheduled tare or calibration operations
    myScale.loop();

    // Yield to other tasks (display, etc.)
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

TaskHandle_t scaleTaskHandle = nullptr;

void setup() {
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
  Log.notice(F("Main: Build options: %s (%s) LOGLEVEL %d " CR), CFG_APPVER,
             CFG_GITREV, LOG_LEVEL);

  myConfig.checkFileSystem();
  myConfig.loadFile();
  myConfig.setWifiScanAP(true);

  myDisplay.setup();
  myDisplay.setFont(FontSize::FONT_12);
  myDisplay.printLineCentered(1, "Kegmon v2");
  myDisplay.printLineCentered(3, "Starting");
  myWifi.init();
  delay(200);
  myScale.setup();

#if defined(ENABLE_MMC)
  myDisplay.printLineCentered(3, "Mounting SD (SD_MMC) card");
  Log.notice(F("Main: MMC_CLK %d." CR), MMC_CLK);
  Log.notice(F("Main: MMC_CMD %d." CR), MMC_CMD);
  Log.notice(F("Main: MMC_D0 %d." CR), MMC_D0);
  mySdStorage.begin(MMC_CLK, MMC_CMD, MMC_D0);
#endif

#if defined(ENABLE_SD)
  myDisplay.printLineCentered(3, "Mounting SD (SD) card");
#if defined(ENABLE_TFT)
  mySdStorage.begin(SD_CS, myDisplay.getSPI());
#else
  mySdStorage.begin(SD_CS, SPI);
#endif  // ENABLE_TFT
#endif  // ENABLE_SD

  // No stored config, move to portal
  if (!myWifi.hasConfig() || myWifi.isDoubleResetDetected()) {
    Log.notice(
        F("Main: Missing wifi config or double reset detected, entering wifi "
          "setup." CR));
    myDisplay.printLineCentered(3, "Entering WIFI Setup");
    myWifi.enableImprov(true);
    myWifi.startAP();
    runMode = RunMode::wifiSetupMode;
  }

  switch (runMode) {
    case RunMode::normalMode:
      myDisplay.printLineCentered(3, "Connecting to WIFI");
      myWifi.connect();
      myWifi.timeSync();
      break;

    case RunMode::wifiSetupMode:
      break;
  }

  checkCoreDump();

  myWebHandler.setupWebServer();
  mySerialWebSocket.begin(myWebHandler.getWebServer(), &EspSerial);
  mySerial.begin(&mySerialWebSocket);
  myTemp.setup();

  Log.notice(F("Main: Setup completed." CR));

  // Show the connected sensors on the display
  char buf[40];
  snprintf(&buf[0], sizeof(buf), "Temperature sensors: %d",
           myTemp.getSensorCount());
  myDisplay.printLineCentered(3, &buf[0]);

  for (int i = 0; i < MAX_SCALES; i++) {
    snprintf(&buf[0], sizeof(buf), "Scale %d: Connected=%s", i + 1,
             myScale.isConnected((UnitIndex)i) ? "yes" : "no");
    myDisplay.printLineCentered(4 + i, &buf[0]);
  }

  // Launch background task for scale reading and change detection on Core 1
  Log.notice(F("Main: Launching scale detection task on core 1" CR));
  xTaskCreatePinnedToCore(scaleDetectionTask,    // Task function
                          "ScaleDetectionTask",  // Task name
                          4096,                  // Stack size (4KB)
                          nullptr,               // Task parameter
                          1,  // Priority (0=idle, higher=more important)
                          &scaleTaskHandle,  // Task handle output
                          1);                // Core ID (0=PRO, 1=APP)

  myTemp.read();
  delay(3000);
  // Choose display layout based on number of connected scales
  myDisplay.createUI(myScale.getConnectedScaleCount() > 2 ? 0 : 1);
}

void loop() {
  if (!myWifi.isConnected() && runMode == RunMode::normalMode) myWifi.connect();

  myUptime.calculate();
  myWebHandler.loop();
  myWifi.loop();
  mySerialWebSocket.loop();

  // Consume events from change detection and queue for web status publishing
  ChangeDetectionEvent event;
  while (myChangeDetection.getNextEvent(event)) {
    myWebHandler.queueEvent(event);

    // Update the display
    myDisplay.setScaleEvent(event.unitIndex, event.type);
  }

  // Handle SD card mount retries
  if (sdTimer.hasExpired()) {
    sdTimer.reset();
#if defined(ENABLE_MMC)
    if (!mySdStorage.hasCard()) {
      Log.notice(F("Loop: SD card not mounted, retry mounting." CR));
      mySdStorage.end();
      mySdStorage.begin(MMC_CLK, MMC_CMD, MMC_D0);
    }
#endif

#if defined(ENABLE_SD)
    if (!mySdStorage.hasCard()) {
      Log.notice(F("Loop: SD card not mounted, retry mounting." CR));
      mySdStorage.end();
#if defined(ENABLE_TFT)
      mySdStorage.begin(SD_CS, myDisplay.getSPI());
#else
      mySdStorage.begin(SD_CS, SPI);
#endif  // ENABLE_TFT
    }
#endif  // ENABLE_SD
  }

  // Update data for the display
  if (displayTimer.hasExpired()) {  
    displayTimer.reset();

    // Set format and theme once (global display settings)
    myDisplay.setDisplayFormat(myConfig.getVolumeUnit(), String(myConfig.getTempUnit()).c_str());
    myDisplay.setTheme(myConfig.getDarkMode());

    for (int i = 0; i < MAX_SCALES; i++) {
      UnitIndex idx = static_cast<UnitIndex>(i);

      // TODO(mpse) : Fetch the temperature for the selected scale
      myDisplay.setScaleData(i, myChangeDetection.getStableWeight(idx),
                             myChangeDetection.getStableVolume(idx),
                             myChangeDetection.getLastPourVolume(idx),
                             myTemp.getLastTempC(0),
                             myScale.isConnected(idx), myChangeDetection.getStateString(idx));
      myDisplay.setKegInfo(idx, myConfig.getKegVolume(idx));
    }
  }

  // Handle peridoic updadates like pushing data and reading temperature sensors
  if (loopTimer.hasExpired()) {  
    loopTimer.reset();

    // Send updates to push targets at regular intervals (300 seconds / 5min)
    if (!(loopTimer.getLoopCounter() % 300)) {
      Log.info(F("LOOP: Pushing updates to configured targets." CR));

      // TODO(mpse) : Update this to push the data.
      // myPush.pushTempInformation(myTemp.getLastTempC(), true);

      // TODO(mpse) : Change this to push data based on events from background
      // thread Push ChangeDetection data for all scales for (int i = 0; i < 4;
      // i++) {
      //   UnitIndex idx = static_cast<UnitIndex>(i);
      //   if (myChangeDetection.getState(idx) == ChangeDetectionState::Stable)
      //   {
      //     float stableWeight = myChangeDetection.getStableWeight(idx);
      //     float stableVolume =
      //         WeightVolumeConverter(idx).weightToVolume(stableWeight);
      //     myPush.pushKegInformation(
      //         idx, stableVolume, myChangeDetection.getPouringVolume(idx), 0,
      //         true);
      //   }
      // }
    }

    // Try to reconnect to scales if they are missing (60 seconds)
    if (!(loopTimer.getLoopCounter() % 30)) {
      if (!myScale.isConnected(UnitIndex::U1) ||
          !myScale.isConnected(UnitIndex::U2) ||
          !myScale.isConnected(UnitIndex::U3) ||
          !myScale.isConnected(UnitIndex::U4)) {
        myScale.setup();  // Try to reconnect to scale
      }
    }

    // Try to reconnect to scales if they are missing (60 seconds)
    if (!(loopTimer.getLoopCounter() % 10)) {
      printHeap("Loop:");
    }

    // TODO(mpse) : Update this to read temperature at interval.
    // if (!(loopTimer.getLoopCounter() % 15)) {
    //   myTemp.read();
    //   Log.notice(F("LOOP: Reading temperature=%F" CR),
    //   myTemp.getLastTempC());
    // }
    // TODO(mpse) : Update this to handle temperature sensor reconnection
    // Check if the temp sensor exist and try to reinitialize
    // if (!(loopTimer.getLoopCounter() % 10)) {
    //   if (!myTemp.hasSensor()) {
    //     myTemp.setup();
    //   }
    // }

    // Read the scales, only once per loop
    // TODO(mpse) : check this
    // uint32_t currentTimeMs = millis();

    // TODO(mpse) : Do TFT updates here

    /*if (myConfig.hasTargetInfluxDb2()) {
      Log.notice(F("LOOP: Sending data to configured influxdb" CR));

      // TDOD(mpse) : copy pushing of data from hw.cpp and move this part to the
      // background thread.
    }*/
  }
}

void checkCoreDump() {
#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3)
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
