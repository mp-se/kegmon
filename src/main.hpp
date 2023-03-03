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

#include <LittleFS.h>
#include <stdlib.h>

#include <log.hpp>

#define CFG_APPNAME "KegMon"         // Name of firmware
#define CFG_MDNSNAME "KegMon"        // Network name
#define CFG_FILENAME "/kegmon.json"  // Name of config file

#if defined(ESP8266)
#define ESP_RESET ESP.reset
#define PIN_LED 2
#define PIN_OLED_SDA D2
#define PIN_OLED_SCL D1
#define PIN_SCALE1_SDA D3
#define PIN_SCALE1_SCL D4
#define PIN_SCALE2_SDA D5
#define PIN_SCALE2_SCL D8
#define PIN_DH2 D7
#define PIN_DH2_PWR D6
#elif defined(ESP32S2)
#define ESP_RESET ESP.restart
#define PIN_LED BUILTIN_LED
#define PIN_OLED_SDA SDA
#define PIN_OLED_SCL SCL
#define PIN_SCALE1_SDA A17
#define PIN_SCALE1_SCL A15
#define PIN_SCALE2_SDA A6
#define PIN_SCALE2_SCL A11
#define PIN_DH2 A10
#define PIN_DH2_PWR A8
#else
#error "Undefined target platform"
#endif

enum UnitIndex { U1 = 0, U2 = 1 };
/*
 * RAW: Last value read
 * STATS: Statistics applied and average value used over the last 20 seconds
 */
enum LevelDetectionType { RAW = 0, STATS = 1 };

#endif  // SRC_MAIN_HPP_
