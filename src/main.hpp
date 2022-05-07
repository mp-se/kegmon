/*
MIT License

Copyright (c) 2021-22 Magnus

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
#ifndef SRC_MAIN_HPP_
#define SRC_MAIN_HPP_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <stdlib.h>
#include <LittleFS.h>
#define ESP_RESET ESP.reset
#define PIN_LED 2
#define PIN_OLED_SDA D2
#define PIN_OLED_SCL D1
#define PIN_SCALE1_SDA D5
#define PIN_SCALE1_SCL D6
#define PIN_SCALE2_SDA D3
#define PIN_SCALE2_SCL D4

enum UnitIndex {
  UNIT_1 = 0, 
  UNIT_2 = 1
};

class SerialDebug {
 public:
  explicit SerialDebug(const uint32_t serialSpeed = 115200L);
  static Logging* getLog() { return &Log; }
};

char* convertFloatToString(float f, char* buffer, int dec);
float reduceFloatPrecision(float f, int dec);

#endif  // SRC_MAIN_HPP_
