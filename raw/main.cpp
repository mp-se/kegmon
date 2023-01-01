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
KegPushHandler myPush(&myConfig);
Display myDisplay;
LevelDetection myLevelDetection;

void setup() {
  Log.notice(F("Level detection simulator" CR));
  myDisplay.setup(UnitIndex::U1);
  myDisplay.setup(UnitIndex::U2);
  myConfig.checkFileSystem();

  myConfig.loadFile();
  myWifi.init();

  // Disable BrewSpy and HA integration for the tests.
  myConfig.setTargetMqtt("");
  myConfig.setPassMqtt("");
  myConfig.setUserMqtt("");
  myConfig.setBrewspyToken(0, "");
  myConfig.setBrewspyToken(1, "");

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
  Log.notice(F("Main: Setup completed." CR));
  delay(3000);
  myDisplay.clear(UnitIndex::U1);
  myDisplay.setFont(UnitIndex::U1, FontSize::FONT_10);
  myDisplay.printLineCentered(UnitIndex::U1, 0, "Running");
  myDisplay.show(UnitIndex::U1);

  // Change setting for the simulation
  myConfig.setKalmanActive(true);

  Log.notice(F("SETUP: Kalman %s, mea=%F, est=%F, noise=%F" CR), myConfig.isKalmanActive() ? "true" : "false",
    myConfig.getKalmanMeasurement(), myConfig.getKalmanEstimation(), myConfig.getKalmanNoise());
  Log.notice(F("SETUP: Max deviation %F" CR), myConfig.getScaleMaxDeviationValue());
}

extern float simulatedData [];
int simulatedIndex = 0;
// int simulatedDelay = 1000;
// int simulatedDelay = 500;
int simulatedDelay = 50;

void loop() {
  if (!myWifi.isConnected()) myWifi.connect();

  myWifi.loop();

  if (simulatedData[simulatedIndex] > 0.0) {
    float v = simulatedData[simulatedIndex];

    myLevelDetection.update(UnitIndex::U1, v);

    Log.verbose(
        F("LOOP: Input: %F, output: raw1=%F,stable1=%F"
          ",pour1=%F [%d]" CR), v,
        myLevelDetection.getRawDetection(UnitIndex::U1)->getRawValue(),
        myLevelDetection.getStatsDetection(UnitIndex::U1)->getStableValue(),
        myLevelDetection.getStatsDetection(UnitIndex::U1)->getPourValue(), simulatedIndex);
    Serial.print(".");

#if defined(ENABLE_INFLUX_DEBUG)
    // This part is used to send data to an influxdb in order to get data on
    // scale stability/drift over time.
    char buf[250];

    float raw1 = myLevelDetection.getRawDetection(UnitIndex::U1)->getRawValue();

    String s;
    snprintf(&buf[0], sizeof(buf),
             "simulate,host=%s,device=%s "
             "level-raw1=%f",
             myConfig.getMDNS(), myConfig.getID(), isnan(raw1) ? 0 : raw1);
    s = &buf[0];

    float ave1 =
        myLevelDetection.getRawDetection(UnitIndex::U1)->getAverageValue();

    snprintf(&buf[0], sizeof(buf), ",level-average1=%f",
             isnan(ave1) ? 0 : ave1);
    s += &buf[0];

    float kal1 = myLevelDetection.getKalmanDetection(UnitIndex::U1)->getValue();

    if (kal1>0.1) {
      snprintf(&buf[0], sizeof(buf), ",level-kalman1=%f",
              isnan(kal1) ? 0 : kal1);
      s += &buf[0];
    }

    float stats1 =
        myLevelDetection.getStatsDetection(UnitIndex::U1)->getStableValue();

    if( stats1 > 0) { // Skip the 0 values to make focus the scale in influx.
      snprintf(&buf[0], sizeof(buf), ",level-stats1=%f",
              isnan(stats1) ? 0 : stats1);
      s += &buf[0];
    }

    Log.verbose(F("%s" CR), s.c_str());

    // Change loglevel to avoid filling console with trace information
    Log.setLevel(LOG_LEVEL_WARNING);
    myPush.sendInfluxDb2(s, PUSH_INFLUX_TARGET, PUSH_INFLUX_ORG,
                         PUSH_INFLUX_BUCKET, PUSH_INFLUX_TOKEN);
    Log.setLevel(LOG_LEVEL);
#endif  // ENABLE_INFLUX_DEBUG

    simulatedIndex++;
    delay(simulatedDelay);
  } else {
    myDisplay.clear(UnitIndex::U1);
    myDisplay.setFont(UnitIndex::U1, FontSize::FONT_10);
    myDisplay.printLineCentered(UnitIndex::U1, 0, "Done");
    myDisplay.show(UnitIndex::U1);
    delay(1000);
  }
}

// EOF
