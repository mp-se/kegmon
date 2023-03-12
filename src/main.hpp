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

constexpr auto CFG_APPNAME = "KegMon";         // Name of firmware
constexpr auto CFG_MDNSNAME = "KegMon";        // Network name
constexpr auto CFG_FILENAME = "/kegmon.json";  // Name of config file

constexpr auto DISPLAY_ADR1 = 0x3c;
constexpr auto DISPLAY_ADR2 = 0x3d;

#if defined(ESP8266)
#define ESP_RESET ESP.reset
constexpr auto PIN_LED = 2;
constexpr auto PIN_OLED_SDA = D2;
constexpr auto PIN_OLED_SCL = D1;
constexpr auto PIN_SCALE1_SDA = D3;
constexpr auto PIN_SCALE1_SCL = D4;
constexpr auto PIN_SCALE2_SDA = D5;
constexpr auto PIN_SCALE2_SCL = D8;
constexpr auto PIN_DH2 = D7;
constexpr auto PIN_DH2_PWR = D6;
#elif defined(ESP32S2)
#define ESP_RESET ESP.restart
constexpr auto PIN_LED = BUILTIN_LED;
constexpr auto PIN_OLED_SDA = SDA;
constexpr auto PIN_OLED_SCL = SCL;
constexpr auto PIN_SCALE1_SDA = A17;
constexpr auto PIN_SCALE1_SCL = A15;
constexpr auto PIN_SCALE2_SDA = A6;
constexpr auto PIN_SCALE2_SCL = A11;
constexpr auto PIN_DH2 = A10;
constexpr auto PIN_DH2_PWR = A8;
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
