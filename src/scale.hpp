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
#ifndef SRC_SCALE_HPP_
#define SRC_SCALE_HPP_

#include <HX711.h>

#include <kegconfig.hpp>
#include <levels.hpp>
#include <main.hpp>
#include <perf.hpp>

// #define DEBUG_LINK_SCALES  // For test rig to use one scale for both...
// #define USE_HX711_MP // Use alternative HX library with different interface
#define USE_HX711 // Use alternative HX library with different interface

class Scale {
 private:
  class Schedule {
   public:
    bool tare = false;
    bool findFactor = false;
    float factorWeight = 0;
  };

  HX711* _hxScale[4] = {nullptr, nullptr, nullptr, nullptr};

  Schedule _sched[4];
  int32_t _lastRaw[4] = {0, 0, 0, 0};

  Scale(const Scale&) = delete;
  void operator=(const Scale&) = delete;

  void tare(UnitIndex idx);
  void findFactor(UnitIndex idx, float weight);
  int32_t readRaw(UnitIndex idx);
  void setupScale(UnitIndex idx, bool force, int pinData, int pinClock);
  void loopScale(UnitIndex idx);
  void setScaleFactor(UnitIndex idx);

 public:
  Scale() {}

  void setup(bool force = false);
  void loop();
  bool isReady(UnitIndex idx) const;

  void scheduleTare(UnitIndex idx) { _sched[idx].tare = true; }
  void scheduleFindFactor(UnitIndex idx, float weight) {
    _sched[idx].findFactor = true;
    _sched[idx].factorWeight = weight;
  }
  bool isScheduleRunning() const {
    return _sched[UnitIndex::U1].findFactor ||
           _sched[UnitIndex::U2].findFactor ||
           _sched[UnitIndex::U3].findFactor ||
           _sched[UnitIndex::U4].findFactor || _sched[UnitIndex::U1].tare ||
           _sched[UnitIndex::U2].tare || _sched[UnitIndex::U3].tare ||
           _sched[UnitIndex::U4].tare;
  }
  int32_t readLastRaw(UnitIndex idx) { return _lastRaw[idx]; }

#if defined(DEBUG_LINK_SCALES)
  bool isConnected(UnitIndex idx) { return true; }
#else
  bool isConnected(UnitIndex idx) const { return _hxScale[idx] ? true : false; }
#endif

  float read(UnitIndex idx, bool skipValidation = false);
};

extern Scale myScale;

#endif  // SRC_SCALE_HPP_

// EOF
