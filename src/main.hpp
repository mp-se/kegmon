/*
MIT License

Copyright (c) 2021-2026 Magnus

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

constexpr auto CFG_MDNSNAME = "Kegmon";
constexpr auto CFG_FILENAME = "/kegmon2.json";
constexpr int MAX_SCALES = 4;

enum RunMode {
  normalMode = 0,
  wifiSetupMode = 2,
};
extern RunMode runMode;

enum UnitIndex { U1 = 0, U2 = 1, U3 = 2, U4 = 3 };

#if defined(LOLIN_S3_PRO)
// Hardware config for Lolin S3 PRO
// --------------------------------
constexpr auto PIN_LED = BUILTIN_LED;
#define PIN_SCALE_SDA1 41
#define PIN_SCALE_SCK1 42
#define PIN_SCALE_SDA2 39
#define PIN_SCALE_SCK2 40
#define PIN_SCALE_SDA3 18
#define PIN_SCALE_SCK3 38
#define PIN_SCALE_SDA4 16
#define PIN_SCALE_SCK4 17
#define PIN_DS 1

#define CFG_FILENAMEBIN "firmware_32s3pro.bin"
#endif

#endif  // SRC_MAIN_HPP_
