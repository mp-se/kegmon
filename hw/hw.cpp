/*
MIT License

Copyright (c) 2023-2025 Magnus

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
#include <main.hpp>
#include <scale.hpp>
#include <temp_mgr.hpp>
#include <utils.hpp>

SerialDebug mySerial(115200L);
KegConfig myConfig("KegmonHW", "./kegmon_hw.json");
Scale myScale;
Display myDisplay;
LevelDetection myLevelDetection;
TempSensorManager myTemp;

const int loopInterval = 1000;
int loopCounter = 0;
uint32_t loopMillis = 0;

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
  myDisplay.setFont(FontSize::FONT_12);
  myDisplay.printLineCentered(1, "Kegmon Hardware Test");

  myConfig.checkFileSystem();
  myConfig.loadFile();

  // Set a scale factor so we get values
  myConfig.setScaleFactor(UnitIndex::U1, 1); 
  myConfig.setScaleFactor(UnitIndex::U2, 1);
  myConfig.setScaleFactor(UnitIndex::U3, 1);
  myConfig.setScaleFactor(UnitIndex::U4, 1);

  myConfig.setScaleOffset(UnitIndex::U1, 1); 
  myConfig.setScaleOffset(UnitIndex::U2, 1); 
  myConfig.setScaleOffset(UnitIndex::U3, 1); 
  myConfig.setScaleOffset(UnitIndex::U4, 1); 

  myScale.setup();
  myTemp.setup();
  myTemp.read();

  // Show what HX711 boards and temp sensors are connected
  myDisplay.printLineCentered(2, "Showing connected sensors");

  char buf[40];
  snprintf(&buf[0], sizeof(buf), "temp: con=%s cnt=%d", myTemp.hasSensor() ? "yes" : "no", myTemp.getSensorCount());
  myDisplay.printLineCentered(3, &buf[0]);

  for(int i = 0; i < 4; i++) {
    snprintf(&buf[0], sizeof(buf), "%d: con=%s rdy=%s", i+1, myScale.isConnected((UnitIndex)i) ? "yes" : "no", myScale.isReady((UnitIndex)i) ? "yes" : "no");
    myDisplay.printLineCentered(4+i, &buf[0]);
    Log.notice(F("Main: %s" CR), &buf[0]);
  }

  delay(5000);

  // Do a tare on the scales and show the offset.
  myDisplay.printLineCentered(2, "Tare connected scales");
  for(int i = 3; i < 8; i++) {
    myDisplay.printLineCentered(i, ""); // Clear lines
  }

  for(int i = 0; i < 4; i++) {
    myScale.scheduleTare((UnitIndex)i);
    myScale.loop();

    snprintf(&buf[0], sizeof(buf), "%d: offset=%d factor=%.2F", i+1, myConfig.getScaleOffset((UnitIndex)i), myConfig.getScaleFactor((UnitIndex)i));
    myDisplay.printLineCentered(4+i, &buf[0]);
    Log.notice(F("Main: %s" CR), &buf[0]);
    yield();
  }

  delay(5000);
  Log.notice(F("Main: Setup completed." CR));
}

void loop() {
  myScale.loop();

  if (abs((int32_t)(millis() - loopMillis)) >
      loopInterval) {  // 2 seconds loop interval
    loopMillis = millis();
    loopCounter++;

    // Show the values
    myDisplay.printLineCentered(2, "Measuring");

    char buf[40];

    for(int i = 0; i < 4; i++) {
      float t = myTemp.getLastTempC(i);

      if(!myScale.isConnected((UnitIndex)i)) {
        snprintf(&buf[0], sizeof(buf), "%d: w=missing, t=%.1F", i+1, t);
      } else {
        snprintf(&buf[0], sizeof(buf), "%d: w=%.2F, t=%.1F", i+1, myScale.read((UnitIndex)i, true), t);
      }

      myDisplay.printLineCentered(4+i, &buf[0]);
      Log.notice(F("Loop: %s" CR), &buf[0]);
    }
  }
}

// EOF
