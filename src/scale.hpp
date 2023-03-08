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
#ifndef SRC_SCALE_HPP_
#define SRC_SCALE_HPP_

#include <HX711.h>
#include <SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h>

#include <kegconfig.hpp>
#include <levels.hpp>
#include <main.hpp>

// #define DEBUG_LINK_SCALES  // For test rig to use one scale for both...

class Scale {
 private:
  class Schedule {
   public:
    bool tare = false;
    bool findFactor = false;
    float factorWeight = 0;
  };

  HX711* _hxScale[2] = {0, 0};
  NAU7802* _nauScale[2] = {0, 0};

  Schedule _sched[2];
  int32_t _lastRaw[2] = {0, 0};

  Scale(const Scale&) = delete;
  void operator=(const Scale&) = delete;

  void tare(UnitIndex idx) {
    switch (myConfig.getScaleSensorType()) {
      case ScaleSensorType::ScaleHX711:
        tareHX711(idx);
        break;
      case ScaleNAU7802:
        tareNAU7802(idx);
        break;
    }
  }
  void findFactor(UnitIndex idx, float weight) {
    switch (myConfig.getScaleSensorType()) {
      case ScaleHX711:
        findFactorHX711(idx, weight);
        break;
      case ScaleSensorType::ScaleNAU7802:
        findFactorNAU7802(idx, weight);
        break;
    }
  }
  int32_t readRaw(UnitIndex idx) {
    if (myConfig.getScaleSensorType() == ScaleSensorType::ScaleHX711)
      return readRawHX711(idx);
    else
      return readRawNAU7802(idx);
  }

  void setupHX711(bool force);
  void setupNAU7802(bool force);
  void setScaleFactorHX711(UnitIndex idx);
  void setScaleFactorNAU7802(UnitIndex idx);
  void tareHX711(UnitIndex idx);
  void tareNAU7802(UnitIndex idx);
  void findFactorHX711(UnitIndex idx, float weight);
  void findFactorNAU7802(UnitIndex idx, float weight);
  float readHX711(UnitIndex idx);
  float readNAU7802(UnitIndex idx);
  int32_t readRawHX711(UnitIndex idx);
  int32_t readRawNAU7802(UnitIndex idx);

 public:
  Scale() {}

  void test(UnitIndex idx) { readRawNAU7802(idx); }

  void setup(bool force = false) {
    switch (myConfig.getScaleSensorType()) {
      case ScaleSensorType::ScaleHX711:
        setupHX711(force);
        break;
      case ScaleSensorType::ScaleNAU7802:
        setupNAU7802(force);
        break;
    }
  }
  void loop(UnitIndex idx);
  void scheduleTare(UnitIndex idx) { _sched[idx].tare = true; }
  void scheduleFindFactor(UnitIndex idx, float weight) {
    _sched[idx].findFactor = true;
    _sched[idx].factorWeight = weight;
  }
  int32_t readLastRaw(UnitIndex idx) { return _lastRaw[idx]; }

#if defined(DEBUG_LINK_SCALES)
  bool isConnected(UnitIndex idx) { return true; }
#else
  bool isConnected(UnitIndex idx) {
    return _hxScale[idx] != 0 || _nauScale[idx] != 0 ? true : false;
  }
#endif
  float read(UnitIndex idx) {
    if (myConfig.getScaleSensorType() == ScaleSensorType::ScaleHX711)
      return readHX711(idx);
    else
      return readNAU7802(idx);
  }
};

extern Scale myScale;

#endif  // SRC_SCALE_HPP_

// EOF
