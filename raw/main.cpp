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
#include <temp_mgr.hpp>
#include <utils.hpp>
#include <wificonnection.hpp>
#include "simulated.hpp"

SerialDebug mySerial(115200L);
KegConfig myConfig(CFG_MDNSNAME, CFG_FILENAME);
WifiConnection myWifi(&myConfig, CFG_APPNAME, "password", CFG_MDNSNAME);
OtaUpdate myOta(&myConfig, CFG_APPVER);
KegPushHandler myPush(&myConfig);
Display myDisplay;
TempSensorManager myTemp;
LevelDetection myLevelDetection;

void setup() {
  Log.notice(F("Level detection simulator" CR));
  myDisplay.setup();
  myConfig.checkFileSystem();

  myConfig.loadFile();
  myWifi.init();

  // Disable BrewSpy and HA integration for the tests.
  myConfig.setTargetMqtt("");
  myConfig.setPassMqtt("");
  myConfig.setUserMqtt("");
  myConfig.setBrewspyToken(UnitIndex::U1, "");
  myConfig.setBrewspyToken(UnitIndex::U1, "");

  String formula = "";
  // String formula = "weight*(1.0-0.025*(tempC-3.0))";
  // String formula = "weight*(1+(0.004*(tempC-5)))";
  // String formula = "weight-(1+0.004*(tempC-5))";
  myConfig.setScaleTempCompensationFormula(UnitIndex::U1, formula);
  myConfig.setScaleTempCompensationFormula(UnitIndex::U2, formula);

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
    myWifi.startAP();
  }

  myWifi.connect();
  Log.notice(F("Main: Setup completed." CR));
  delay(3000);
  myDisplay.clear(UnitIndex::U1);
  myDisplay.setFont(UnitIndex::U1, FontSize::FONT_10);
  myDisplay.printLineCentered(UnitIndex::U1, 0, "Running");
  myDisplay.show(UnitIndex::U1);

  // Change setting for the simulation
  Log.notice(F("SETUP: Max deviation increase %F" CR), myConfig.getScaleDeviationIncreaseValue());
  Log.notice(F("SETUP: Max deviation decrease %F" CR), myConfig.getScaleDeviationDecreaseValue());
}

int simulatedIndex = 0;
// int simulatedDelay = 1000;
// int simulatedDelay = 500;
// int simulatedDelay = 200;
int simulatedDelay = 100;
// int simulatedDelay = 50;

void loop() {
  if (!myWifi.isConnected()) myWifi.connect();

  myWifi.loop();

  if (simulatedData[simulatedIndex].scale1 > 0.0) {
    float t = simulatedData[simulatedIndex].temp; 
    // float v = simulatedData[simulatedIndex].scale1; 
    float v = simulatedData[simulatedIndex].scale2; 

    myLevelDetection.update(UnitIndex::U1, v, t);

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
             "level-raw1=%f,temp=%f",
             myConfig.getMDNS(), myConfig.getID(), isnan(raw1) ? 0 : raw1, t);
    s = &buf[0];

    float ave1 =
        myLevelDetection.getRawDetection(UnitIndex::U1)->getAverageValue();

    if (!isnan(ave1)) {
      snprintf(&buf[0], sizeof(buf), ",level-average1=%f",
              isnan(ave1) ? 0 : ave1);
      s += &buf[0];
    }

    float kal1 = myLevelDetection.getRawDetection(UnitIndex::U1)->getKalmanValue();

    if (kal1 > 0.1  && !isnan(kal1)) {
      snprintf(&buf[0], sizeof(buf), ",level-kalman1=%f",
              isnan(kal1) ? 0 : kal1);
      s += &buf[0];
    }

    float stats1 =
        myLevelDetection.getStatsDetection(UnitIndex::U1)->getStableValue();

    if( stats1 > 0  && !isnan(stats1)) { // Skip the 0 values to make focus the scale in influx.
      snprintf(&buf[0], sizeof(buf), ",level-stats1=%f",
              isnan(stats1) ? 0 : stats1);
      s += &buf[0];
    }

    float temp1 =
        myLevelDetection.getRawDetection(UnitIndex::U1)->getTempCorrValue();

    if( temp1 > 0 && !isnan(temp1)) { // Skip the 0 values to make focus the scale in influx.
      snprintf(&buf[0], sizeof(buf), ",level-temp1=%f",
              isnan(temp1) ? 0 : temp1);
      s += &buf[0];
    }

    float slope1 =
        myLevelDetection.getRawDetection(UnitIndex::U1)->getSlopeValue();

    if(!isnan(slope1)) { 
      snprintf(&buf[0], sizeof(buf), ",level-slope1=%f",
              isnan(slope1) ? 0 : slope1);
      s += &buf[0];
    }

    float tempCorr1 =
        myLevelDetection.getRawDetection(UnitIndex::U1)->getTempCorrValue();

    if(!isnan(tempCorr1)) { 
      snprintf(&buf[0], sizeof(buf), ",level-temp=%f",
              isnan(tempCorr1) ? 0 : tempCorr1);
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
