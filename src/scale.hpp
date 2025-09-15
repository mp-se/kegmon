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

  void tare(UnitIndex idx) {
    if (!_hxScale[idx]) return;

    Log.notice(
        F("SCAL: HX711 set scale to zero, prepare for calibration %d [%d]." CR),
        myConfig.getScaleReadCountCalibration(), idx);

    _hxScale[idx]->set_scale(1.0);
    _hxScale[idx]->tare(myConfig.getScaleReadCountCalibration());
    int32_t l = _hxScale[idx]->get_offset();
    Log.verbose(F("SCAL: HX711 New scale offset found %l [%d]." CR), l, idx);
    myConfig.setScaleOffset(idx, l);
    myConfig.saveFile();
  }

  void findFactor(UnitIndex idx, float weight) {
    if (!_hxScale[idx]) return;

    float l = _hxScale[idx]->get_units(myConfig.getScaleReadCountCalibration());
    float f = l / weight;
    Log.notice(
        F("SCAL: HX711 Detecting factor for weight %F, raw %l %F [%d]." CR),
        weight, l, f, idx);

    if (isinf(f) || isnan(f)) f = 0.0;

    myConfig.setScaleFactor(idx, f);
    myConfig.saveFile();  // save the factor to file

    setScaleFactor(idx);  // apply the factor after it has been saved
    read(idx, true);
  }

  int32_t readRaw(UnitIndex idx) {
#if defined(DEBUG_LINK_SCALES)
    idx = UnitIndex::U1;
#endif
#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: HX711 Reading raw scale for [%d]." CR), idx);
#endif
    if (!_hxScale[idx]) return 0;
    PERF_BEGIN("scale-readraw");
    int32_t l = _hxScale[idx]->read_average(
        myConfig.getScaleReadCountCalibration());  // get the raw value without
                                                   // applying scaling factor
    _lastRaw[idx] = l;
#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: HX711 Reading scale raw weight=%d [%d]" CR), l, idx);
#endif
    PERF_END("scale-readraw");
    return l;
  }

  void setupScale(UnitIndex idx, bool force, int pinData, int pinClock) {
    if (!_hxScale[idx] || force) {
      if (_hxScale[idx]) delete _hxScale[idx];

#if LOG_LEVEL == 6
      Log.verbose(F("SCAL: HX711 initializing scale [0], using offset %l." CR),
                  myConfig.getScaleOffset(idx));
#endif
      _hxScale[idx] = new HX711();
      Log.notice(
          F("SCAL: Initializing HX711 bus #%d on pins Data=%d,Clock=%d" CR),
          idx + 1, pinData, pinClock);
      _hxScale[idx]->begin(pinData, pinClock);
      _hxScale[idx]->set_offset(myConfig.getScaleOffset(idx));

      if (_hxScale[idx]->wait_ready_timeout(500)) {
        Log.notice(F("SCAL: HX711 scale [%d] found." CR), idx);
        _hxScale[idx]->get_units(1);
      } else {
        Log.error(
            F("SCAL: HX711 scale [%d] not responding, disabling interface." CR),
            idx);
        delete _hxScale[idx];
        _hxScale[idx] = nullptr;
      }
    }

    setScaleFactor(idx);
  }

  void setScaleFactor(UnitIndex idx) {
    if (!_hxScale[idx]) return;

    float fs = myConfig.getScaleFactor(idx);

    if (fs == 0.0) fs = 1.0;

    _hxScale[idx]->set_scale(fs);
  }

 public:
  Scale() {}

  void setup(bool force = false) {
    setupScale(UnitIndex::U1, force, PIN_SCALE_SDA1, PIN_SCALE_SCK1);
    setupScale(UnitIndex::U2, force, PIN_SCALE_SDA2, PIN_SCALE_SCK2);
    setupScale(UnitIndex::U3, force, PIN_SCALE_SDA3, PIN_SCALE_SCK3);
    setupScale(UnitIndex::U4, force, PIN_SCALE_SDA4, PIN_SCALE_SCK4);
  }

  void loop(UnitIndex idx);

  void scheduleTare(UnitIndex idx) { _sched[idx].tare = true; }
  void scheduleFindFactor(UnitIndex idx, float weight) {
    _sched[idx].findFactor = true;
    _sched[idx].factorWeight = weight;
  }
  bool isScheduleRunning() {
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
  bool isConnected(UnitIndex idx) { return _hxScale[idx] != 0 ? true : false; }
#endif

  float read(UnitIndex idx, bool skipValidation = false) {
#if defined(DEBUG_LINK_SCALES)
    idx = UnitIndex::U1;
#endif

#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: HX711 reading scale for [%d]." CR), idx);
#endif

    if (myConfig.getScaleFactor(idx) == 0 ||
        myConfig.getScaleOffset(idx) ==
            0) {  // Not initialized, just return zero
      Log.verbose(F("SCAL: HX711 scale not initialized [%d]." CR), idx);
      return 0;
    }

    if (!_hxScale[idx]) return 0;

    PERF_BEGIN("scale-read");
    float raw = _hxScale[idx]->get_units(myConfig.getScaleReadCount());
#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: HX711 Reading weight=%F [%d]" CR), raw, idx);
#endif

    if (!skipValidation) {
      // If the value is higher/lower than 100 kb/lbs then the reading is
      // proably wrong, just ignore the reading
      if (raw > 100) {
        Log.error(
            F("SCAL: HX711 Ignoring value since it's higher than 100kg, %F "
              "[%d]." CR),
            raw, idx);
        PERF_END("scale-read");
        return NAN;
      }

      if (raw < -100) {
        Log.error(F("SCAL: HX711 Ignoring value since it's less than -100kg %F "
                    "[%d]." CR),
                  raw, idx);
        PERF_END("scale-read");
        return NAN;
      }
    }

    PERF_END("scale-read");
    return raw;
  }
};

extern Scale myScale;

#endif  // SRC_SCALE_HPP_

// EOF
