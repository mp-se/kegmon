/*
MIT License

Copyright (c) 2021-2024 Magnus

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
#include <kegpush.hpp>
#include <perf.hpp>
#include <scale.hpp>

void Scale::setupHX711(bool force) {
  if (!_hxScale[0] || force) {
    if (_hxScale[0]) delete _hxScale[0];

#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: HX711 initializing scale [0], using offset %l." CR),
                myConfig.getScaleOffset(UnitIndex::U1));
#endif
    _hxScale[0] = new HX711();
    Log.notice(F("SCAL: Initializing HX711 bus #1 on pins Data=%d,Clock=%d" CR),
               myConfig.getPinScale1Data(), myConfig.getPinScale1Clock());
    _hxScale[0]->begin(myConfig.getPinScale1Data(),
                       myConfig.getPinScale1Clock());
    _hxScale[0]->set_offset(myConfig.getScaleOffset(UnitIndex::U1));

    if (_hxScale[0]->wait_ready_timeout(500)) {
      Log.notice(F("SCAL: HX711 scale [0] found." CR));
      _hxScale[0]->get_units(1);
    } else {
      Log.error(
          F("SCAL: HX711 scale [0] not responding, disabling interface." CR));
      delete _hxScale[0];
      _hxScale[0] = 0;
    }
  }

  if (!_hxScale[1] || force) {
    if (_hxScale[1]) delete _hxScale[1];

#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: HX711 initializing [1], using offset %l." CR),
                myConfig.getScaleOffset(UnitIndex::U2));
#endif
    _hxScale[1] = new HX711();
    Log.notice(F("SCAL: Initializing HX711 bus #2 on pins Data=%d,Clock=%d" CR),
               myConfig.getPinScale2Data(), myConfig.getPinScale2Clock());
    _hxScale[1]->begin(myConfig.getPinScale2Data(),
                       myConfig.getPinScale2Clock());
    _hxScale[1]->set_offset(myConfig.getScaleOffset(UnitIndex::U2));

    if (_hxScale[1]->wait_ready_timeout(500)) {
      Log.notice(F("SCAL: HX711 scale [1] found." CR));
      _hxScale[1]->get_units(1);
    } else {
      Log.error(
          F("SCAL: HX711 scale [1] not responding, disabling interface." CR));
      delete _hxScale[1];
      _hxScale[1] = 0;
    }
  }

  setScaleFactorHX711(UnitIndex::U1);
  setScaleFactorHX711(UnitIndex::U2);
}

void Scale::setScaleFactorHX711(UnitIndex idx) {
  if (!_hxScale[idx]) return;

  float fs = myConfig.getScaleFactor(idx);

  if (fs == 0.0) fs = 1.0;

  _hxScale[idx]->set_scale(fs);
}

float Scale::readHX711(UnitIndex idx, bool skipValidation) {
#if defined(DEBUG_LINK_SCALES)
  idx = UnitIndex::U1;
#endif

#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: HX711 reading scale for [%d]." CR), idx);
#endif

  if (myConfig.getScaleFactor(idx) == 0 ||
      myConfig.getScaleOffset(idx) == 0) {  // Not initialized, just return zero
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
    // If the value is higher/lower than 100 kb/lbs then the reading is proably
    // wrong, just ignore the reading
    if (raw > 100) {
      Log.error(F("SCAL: HX711 Ignoring value since it's higher than 100kg, %F "
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

void Scale::tareHX711(UnitIndex idx) {
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

int32_t Scale::readRawHX711(UnitIndex idx) {
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

void Scale::findFactorHX711(UnitIndex idx, float weight) {
  if (!_hxScale[idx]) return;

  float l = _hxScale[idx]->get_units(myConfig.getScaleReadCountCalibration());
  float f = l / weight;
  Log.notice(
      F("SCAL: HX711 Detecting factor for weight %F, raw %l %F [%d]." CR),
      weight, l, f, idx);

  if (isinf(f) || isnan(f)) f = 0.0;

  myConfig.setScaleFactor(idx, f);
  myConfig.saveFile();  // save the factor to file

  setScaleFactorHX711(idx);  // apply the factor after it has been saved
  readHX711(idx, true);
}

// EOF
