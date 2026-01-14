/*
MIT License

Copyright (c) 2023-2026 Magnus

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
#include <FS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <changedetection.hpp>
#include <cstdio>
#include <display.hpp>
#include <kegconfig.hpp>
#include <kegpush.hpp>
#include <looptimer.hpp>
#include <main.hpp>
#include <scale.hpp>
#include <temp_mgr.hpp>
#include <utils.hpp>
#include <wificonnection.hpp>

// #define SCALE_AUTO_TARE 1
// #define SCALE_RUN_CALIBRATION 1

// Note: MAX_SCALES is defined in main.hpp

const auto CONST_WIFI_SSID = "";  // Empty disable wifi connection
const auto CONST_WIFI_PASS = "";
const auto CONST_INFLUXDB_HOST = "";  // Empty disable the influxdb push
const auto CONST_INFLUXDB_ORG = "";
const auto CONST_INFLUXDB_TOKEN = "";
const auto CONST_INFLUXDB_BUCKET = "";

// Calibration constants for testing
const auto CONST_SCALE_OFFSET_U1 = -192039.0f;  // Set your scale offsets here.
const auto CONST_SCALE_OFFSET_U2 = 1.0f;
const auto CONST_SCALE_OFFSET_U3 = 1.0f;
const auto CONST_SCALE_OFFSET_U4 = 1.0f;

const auto CONST_SCALE_FACTOR_U1 = 22986.79f;  // Set your scale factors here.
const auto CONST_SCALE_FACTOR_U2 = 1.0f;
const auto CONST_SCALE_FACTOR_U3 = 1.0f;
const auto CONST_SCALE_FACTOR_U4 = 1.0f;

const auto CONST_CALIBRATE_WEIGHT =
    6.1f;  // Set your calibration weight here (kg)

LoopTimer myLoopTimer(1000);    // Main loop timer
LoopTimer myInfluxTimer(1000);  // InfluxDB send timer

SerialDebug mySerial(115200L);
KegConfig myConfig("KegmonHW", "./kegmon_hw.json");
WifiConnection myWifi(&myConfig, CFG_APPNAME, "password", CFG_MDNSNAME);
KegPushHandler myPush(&myConfig);
Display myDisplay;

// Scale handling
Scale myScale;
TempSensorManager myTemp;
ChangeDetection myChangeDetection;

// Dual-core task handling
TaskHandle_t scaleTaskHandle = nullptr;

/**
 * Background task for scale reading, change detection, and raw data logging
 * Runs on Core 1:
 *  - Continuously samples scales via HX711
 *  - Updates change detection state machine
 *  - Sends raw measurement data to InfluxDB every ~1 second
 * Main loop stays focused on event detection and event logging
 */
void scaleDetectionTask(void* parameter) {
  Log.notice(F("ScaleTask: Started on core %d" CR), xPortGetCoreID());

  ScaleReadingResult res[MAX_SCALES];
  uint64_t lastInfluxSend = 0;
  char buf[200];

  while (true) {
    // Process scale loop to read HX711
    myScale.loop();

    // Read from all scales and update change detection state
    uint64_t currentTimeMs = millis();
    for (int i = 0; i < MAX_SCALES; i++) {
      res[i] = myScale.read((UnitIndex)i);
      myChangeDetection.update((UnitIndex)i, res[i], currentTimeMs);
    }

    // Send raw measurement data to InfluxDB periodically (every 1 second)
    if (myInfluxTimer.hasExpired() && myConfig.hasTargetInfluxDb2()) {
      myInfluxTimer.reset();

      String s;
      float v;
      s.reserve(512);

      snprintf(&buf[0], sizeof(buf), "scale,host=%s,device=%s ",
               myConfig.getMDNS(), myConfig.getID());
      s = &buf[0];

      // Temperature data
      for (int i = 0; i < myTemp.getSensorCount(); i++) {
        v = myTemp.getLastTempC(i);
        if (!isnan(v)) {
          snprintf(&buf[0], sizeof(buf), "tempC%i=%f,", i + 1, v);
          s = s + &buf[0];
        }
      }

      // Scale filter data - all computed values
      for (int i = 0; i < MAX_SCALES; i++) {
        if (!isnan(res[i].raw)) {
          snprintf(&buf[0], sizeof(buf), "raw%i=%f,", i + 1, res[i].raw);
          s = s + &buf[0];
        }
        if (!isnan(res[i].moving_average)) {
          snprintf(&buf[0], sizeof(buf), "ma%i=%f,", i + 1,
                   res[i].moving_average);
          s = s + &buf[0];
        }
        if (!isnan(res[i].ema)) {
          snprintf(&buf[0], sizeof(buf), "ema%i=%f,", i + 1, res[i].ema);
          s = s + &buf[0];
        }
        if (!isnan(res[i].weighted_ma)) {
          snprintf(&buf[0], sizeof(buf), "wma%i=%f,", i + 1,
                   res[i].weighted_ma);
          s = s + &buf[0];
        }
        if (!isnan(res[i].median)) {
          snprintf(&buf[0], sizeof(buf), "median%i=%f,", i + 1, res[i].median);
          s = s + &buf[0];
        }
        if (!isnan(res[i].zscore)) {
          snprintf(&buf[0], sizeof(buf), "zscore%i=%f,", i + 1, res[i].zscore);
          s = s + &buf[0];
        }
        if (!isnan(res[i].hampel)) {
          snprintf(&buf[0], sizeof(buf), "hampel%i=%f,", i + 1, res[i].hampel);
          s = s + &buf[0];
        }
        if (!isnan(res[i].complementary)) {
          snprintf(&buf[0], sizeof(buf), "comp%i=%f,", i + 1,
                   res[i].complementary);
          s = s + &buf[0];
        }
        if (!isnan(res[i].alphabeta)) {
          snprintf(&buf[0], sizeof(buf), "ab%i=%f,", i + 1, res[i].alphabeta);
          s = s + &buf[0];
        }
        if (!isnan(res[i].butterworth)) {
          snprintf(&buf[0], sizeof(buf), "butter%i=%f,", i + 1,
                   res[i].butterworth);
          s = s + &buf[0];
        }
        if (!isnan(res[i].fir)) {
          snprintf(&buf[0], sizeof(buf), "fir%i=%f,", i + 1, res[i].fir);
          s = s + &buf[0];
        }
        if (!isnan(res[i].chebyshev)) {
          snprintf(&buf[0], sizeof(buf), "cheby%i=%f,", i + 1,
                   res[i].chebyshev);
          s = s + &buf[0];
        }
        if (!isnan(res[i].kalman)) {
          snprintf(&buf[0], sizeof(buf), "kalman%i=%f,", i + 1, res[i].kalman);
          s = s + &buf[0];
        }

        // Change detection state data
        float stableWeight = myChangeDetection.getStableWeight((UnitIndex)i);
        float pourVolume = myChangeDetection.getPouringVolume((UnitIndex)i);
        float confidence = myChangeDetection.getConfidence((UnitIndex)i);

        snprintf(&buf[0], sizeof(buf), "state%i=\"%s\",", i + 1,
                 myChangeDetection.getStateString((UnitIndex)i));
        s = s + &buf[0];
        snprintf(&buf[0], sizeof(buf), "stable_wgt%i=%f,", i + 1, stableWeight);
        s = s + &buf[0];
        if (!isnan(pourVolume)) {
          snprintf(&buf[0], sizeof(buf), "pour%i=%f,", i + 1, pourVolume);
          s = s + &buf[0];
        }
        snprintf(&buf[0], sizeof(buf), "conf%i=%f", i + 1, confidence);
        s = s + &buf[0];
      }

      // Remove trailing comma if present
      if (s.endsWith(",")) {
        s = s.substring(0, s.length() - 1);
      }

      myPush.sendInfluxDb2(
          s, myConfig.getTargetInfluxDB2(), myConfig.getOrgInfluxDB2(),
          myConfig.getBucketInfluxDB2(), myConfig.getTokenInfluxDB2());

      lastInfluxSend = currentTimeMs;
    }

    // Yield to allow other tasks to run, with minimal delay for responsiveness
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  delay(2000);

  char cbuf[20];
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  snprintf(&cbuf[0], sizeof(cbuf), "%6x", chipId);
  Log.notice(F("Main: Started setup for %s." CR), &cbuf[0]);

  Log.notice(F("Main: Build options: %s (%s) LOGLEVEL %d " CR), CFG_APPVER,
             CFG_GITREV, LOG_LEVEL);

#if defined(ENABLE_TFT)
  Log.notice(F("Main: TOUCH_CS %d." CR), TOUCH_CS);
  Log.notice(F("Main: TFT_BL %d." CR), TFT_BL);
  Log.notice(F("Main: TFT_DC %d." CR), TFT_DC);
  Log.notice(F("Main: TFT_MISO %d." CR), TFT_MISO);
  Log.notice(F("Main: TFT_MOSI %d." CR), TFT_MOSI);
  Log.notice(F("Main: TFT_SCLK %d." CR), TFT_SCLK);
  Log.notice(F("Main: TFT_RST %d." CR), TFT_RST);
  Log.notice(F("Main: TFT_CS %d." CR), TFT_CS);
#endif

  Log.notice(F("Main: Initialize display." CR));
  myDisplay.setup();
  myDisplay.setFont(FontSize::FONT_9);
  myDisplay.printLineCentered(1, "Kegmon Hardware Test");

  // Use hardcoded configuration
  myConfig.setWifiScanAP(true);
  myConfig.setWifiSSID(CONST_WIFI_SSID, 0);
  myConfig.setWifiPass(CONST_WIFI_PASS, 0);
  myConfig.setTargetInfluxDB2(CONST_INFLUXDB_HOST);
  myConfig.setOrgInfluxDB2(CONST_INFLUXDB_ORG);
  myConfig.setTokenInfluxDB2(CONST_INFLUXDB_TOKEN);
  myConfig.setBucketInfluxDB2(CONST_INFLUXDB_BUCKET);

  // Set a scale factor so we get values
  myConfig.setScaleFactor(UnitIndex::U1, CONST_SCALE_FACTOR_U1);
  myConfig.setScaleFactor(UnitIndex::U2, CONST_SCALE_FACTOR_U2);
  myConfig.setScaleFactor(UnitIndex::U3, CONST_SCALE_FACTOR_U3);
  myConfig.setScaleFactor(UnitIndex::U4, CONST_SCALE_FACTOR_U4);

  myConfig.setScaleOffset(UnitIndex::U1, CONST_SCALE_OFFSET_U1);
  myConfig.setScaleOffset(UnitIndex::U2, CONST_SCALE_OFFSET_U2);
  myConfig.setScaleOffset(UnitIndex::U3, CONST_SCALE_OFFSET_U3);
  myConfig.setScaleOffset(UnitIndex::U4, CONST_SCALE_OFFSET_U4);

  if (strlen(CONST_WIFI_SSID)) {
    Log.notice(F("Main: Connecting to WifF." CR));
    myDisplay.printLineCentered(2, "Connecting to WiFi");
    myWifi.connect();
  }

  myScale.setup();
  myTemp.setup();
  myTemp.read();

  // Show what HX711 boards and temp sensors are connected
  //
  //
  myDisplay.printLineCentered(2, "Showing connected sensors");

  char buf[40];
  snprintf(&buf[0], sizeof(buf), "temp: tmp=%s cnt=%d",
           myTemp.hasSensor() ? "yes" : "no", myTemp.getSensorCount());
  myDisplay.printLineCentered(7, &buf[0]);

  for (int i = 0; i < MAX_SCALES; i++) {
    snprintf(&buf[0], sizeof(buf), "%d: adc=%s rdy=%s", i + 1,
             myScale.isConnected((UnitIndex)i) ? "yes" : "no",
             myScale.isReady((UnitIndex)i) ? "yes" : "no");
    myDisplay.printLineCentered(3 + i, &buf[0]);
    Log.notice(F("Main: %s" CR), &buf[0]);
  }

  delay(5000);

  // Do a tare on the scales and show the offset.
  //
  //
#if defined(SCALE_AUTO_TARE)
  myDisplay.printLineCentered(2, "Tare connected scales");
  for (int i = 3; i < 8; i++) {
    myDisplay.printLineCentered(i, "");  // Clear lines
  }

  for (int i = 0; i < MAX_SCALES; i++) {
    myScale.scheduleTare((UnitIndex)i);
    myScale.loop();

    snprintf(&buf[0], sizeof(buf), "%d: off=%d fct=%.2F", i + 1,
             myConfig.getScaleOffset((UnitIndex)i),
             myConfig.getScaleFactor((UnitIndex)i));
    myDisplay.printLineCentered(3 + i, &buf[0]);
    Log.notice(F("Main: %s" CR), &buf[0]);
    yield();
  }

  delay(5000);
#endif

  // Run a calibration procedure on scale 1
  //
  //
#if defined(SCALE_RUN_CALIBRATION)
  myDisplay.printLineCentered(2, "Add weights for calibration");
  for (int i = 3; i < 8; i++) {
    myDisplay.printLineCentered(i, "");  // Clear lines
  }

  delay(10000);  // Time to add weights
  myDisplay.printLineCentered(2, "Doing calibration");

  for (int i = 0; i < MAX_SCALES; i++) {
    myScale.scheduleFindFactor((UnitIndex)i,
                               CONST_CALIBRATE_WEIGHT);  // Calibrate with 5 kg
    myScale.loop();

    snprintf(&buf[0], sizeof(buf), "%d: off=%d fct=%.2F", i + 1,
             myConfig.getScaleOffset((UnitIndex)i),
             myConfig.getScaleFactor((UnitIndex)i));
    myDisplay.printLineCentered(3 + i, &buf[0]);
    Log.notice(F("Main: %s" CR), &buf[0]);
    yield();
  }
#endif

  // Launch background task for scale reading and change detection on Core 1
  Log.notice(F("Main: Launching scale detection task on core 1" CR));
  xTaskCreatePinnedToCore(scaleDetectionTask,    // Task function
                          "ScaleDetectionTask",  // Task name
                          4096,                  // Stack size (4KB)
                          nullptr,               // Task parameter
                          1,  // Priority (0=idle, higher=more important)
                          &scaleTaskHandle,  // Task handle output
                          1);                // Core ID (0=PRO, 1=APP)

  Log.notice(F("Main: Setup completed." CR));
}

void loop() {
#if defined(SCALE_RUN_CALIBRATION)
  return;  // Skip the measurement part
#endif

  // NOTE: Scale reading and change detection now runs in background task on
  // Core 1 Main loop (Core 0) handles: display updates, event processing, and
  // InfluxDB sending

  if (myLoopTimer.hasExpired()) {
    myLoopTimer.reset();

    // Show the values on the display
    //
    //
    myDisplay.printLineCentered(2, "Measuring");

    for (int i = 0; i < MAX_SCALES; i++) {
      float t = myTemp.getLastTempC(i);
      char buf[40];

      if (!myScale.isConnected((UnitIndex)i)) {
        snprintf(&buf[0], sizeof(buf), "%d: wgt=missing, tmp=%.1F", i + 1, t);
      } else {
        uint32_t timer = millis();
        float w = myScale.read((UnitIndex)i)
                      .raw;  // Read raw value from filter result

        timer = millis() - timer;
        snprintf(&buf[0], sizeof(buf), "%d: wgt=%.2F, tmp=%.1F, %s", i + 1, w,
                 t, myChangeDetection.getStateString((UnitIndex)i));
      }

      myDisplay.printLineCentered(3 + i, &buf[0]);
    }

    // NOTE: Raw scale data is now sent to InfluxDB from background task every
    // ~1 second Main loop focuses on event detection and logging instead

    // Process any queued events and send them to InfluxDB
    if (myConfig.hasTargetInfluxDb2()) {
      ChangeDetectionEvent event;
      while (myChangeDetection.getNextEvent(event)) {
        String eventStr;
        eventStr.reserve(256);

        char buf[200];
        const char* eventType = "unknown";

        snprintf(&buf[0], sizeof(buf),
                 "keg_event,host=%s,device=%s,unit=%d,type=",
                 myConfig.getMDNS(), myConfig.getID(), event.unitIndex);
        eventStr = &buf[0];

        // Add event type tag
        switch (event.type) {
          case ChangeDetectionEventType::STABLE_DETECTED:
            eventType = "stable_detected";
            break;
          case ChangeDetectionEventType::POUR_STARTED:
            eventType = "pour_started";
            break;
          case ChangeDetectionEventType::POUR_COMPLETED:
            eventType = "pour_completed";
            break;
          case ChangeDetectionEventType::KEG_REMOVED:
            eventType = "keg_removed";
            break;
          case ChangeDetectionEventType::KEG_REPLACED:
            eventType = "keg_replaced";
            break;
        }

        eventStr = eventStr + eventType + " ";

        // Add event-specific fields
        if (event.type == ChangeDetectionEventType::STABLE_DETECTED) {
          snprintf(&buf[0], sizeof(buf),
                   "weight_kg=%f,volume_l=%f,duration_ms=%llu",
                   event.stable.stableWeightKg, event.stable.stableVolumeL,
                   event.stable.durationMs);
          eventStr = eventStr + &buf[0];
        } else if (event.type == ChangeDetectionEventType::POUR_STARTED) {
          snprintf(&buf[0], sizeof(buf), "weight_kg=%f",
                   event.weight.currentWeightKg);
          eventStr = eventStr + &buf[0];
        } else if (event.type == ChangeDetectionEventType::POUR_COMPLETED) {
          snprintf(&buf[0], sizeof(buf),
                   "pre_pour_kg=%f,post_pour_kg=%f,pour_kg=%f,pour_l=%f,"
                   "duration_ms=%llu,"
                   "slope_kg_per_sec=%f",
                   event.pour.prePourWeightKg, event.pour.postPourWeightKg,
                   event.pour.pourWeightKg, event.pour.pourVolumeL,
                   event.pour.durationMs, event.pour.averageSlopeKgSec);
          eventStr = eventStr + &buf[0];
        } else if (event.type == ChangeDetectionEventType::KEG_REMOVED ||
                   event.type == ChangeDetectionEventType::KEG_REPLACED) {
          snprintf(&buf[0], sizeof(buf), "previous_kg=%f",
                   event.weight.previousWeightKg);
          eventStr = eventStr + &buf[0];
        }

        snprintf(&buf[0], sizeof(buf), " %llu", event.timestampMs);
        eventStr = eventStr + &buf[0];

        Log.verbose(F("LOOP: Event: %s" CR), eventStr.c_str());

        myPush.sendInfluxDb2(
            eventStr, myConfig.getTargetInfluxDB2(), myConfig.getOrgInfluxDB2(),
            myConfig.getBucketInfluxDB2(), myConfig.getTokenInfluxDB2());
      }
    }
  }
}

// EOF
