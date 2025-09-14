/*
MIT License

Copyright (c) 2021-2025 Magnus

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

constexpr auto CFG_MDNSNAME = "KegMon";
constexpr auto CFG_FILENAME = "/kegmon1.json";
constexpr auto STARTUP_FILENAME = "/startup.log";
constexpr auto LEVELS_FILENAME = "/levels.log";
constexpr auto LEVELS_FILENAME2 = "/levels2.log";

constexpr auto DISPLAY_ADR1 = 0x3c;
constexpr auto DISPLAY_ADR2 = 0x3d;

constexpr auto JSON_BUFFER = 3000;

enum RunMode {
  normalMode = 0,
  wifiSetupMode = 2,
};
extern RunMode runMode;

constexpr auto PIN_LED = BUILTIN_LED;

enum UnitIndex { U1 = 0, U2 = 1 };
/*
 * RAW: Last value read
 * STATS: Statistics applied and average value used over the last 20 seconds
 */
enum LevelDetectionType { RAW = 0, STATS = 1 };

#endif  // SRC_MAIN_HPP_
