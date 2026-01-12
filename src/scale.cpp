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
#include <filters/filter_alphabeta.hpp>
#include <filters/filter_butterworth.hpp>
#include <filters/filter_chebyshev.hpp>
#include <filters/filter_complementary.hpp>
#include <filters/filter_ema.hpp>
#include <filters/filter_fir.hpp>
#include <filters/filter_hampel.hpp>
#include <filters/filter_kalman.hpp>
#include <filters/filter_median.hpp>
#include <filters/filter_movingaverage.hpp>
#include <filters/filter_weightedma.hpp>
#include <filters/filter_zscore.hpp>
#include <kegpush.hpp>
#include <memory>
#include <perf.hpp>
#include <scale.hpp>

bool Scale::isReady(UnitIndex idx) const {
  if (!_hxScale[idx]) {
    Log.verbose(F("SCAL: HX711 scale not ready [%d]." CR), idx);
    return false;
  }
  return _hxScale[idx]->wait_ready_retry(3, 1000);
}

void Scale::tare(UnitIndex idx) {
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

void Scale::findFactor(UnitIndex idx, float weight) {
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
  read(idx);
}

float Scale::readRaw(UnitIndex idx) {
#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: HX711 Reading raw scale for [%d]." CR), idx);
#endif
  if (!_hxScale[idx]) return 0;
  PERF_BEGIN("scale-readraw");
  float l = _hxScale[idx]->read_average(
      myConfig.getScaleReadCountCalibration());  // get the raw value without
                                                 // applying scaling factor
  _lastRaw[idx] = l;
#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: HX711 Reading scale raw weight=%d [%d]" CR), l, idx);
#endif
  PERF_END("scale-readraw");
  return l;
}

ScaleReadingResult Scale::read(UnitIndex idx) {
  if (!_hxScale[idx]) {
    return _lastResult[idx];
  }

  if (myConfig.getScaleFactor(idx) == 0 ||
      myConfig.getScaleOffset(idx) == 0) {  // Not initialized
    // Log.verbose(F("SCAL: HX711 has no configuration [%d]." CR), idx);
    return _lastResult[idx];
  }

#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: HX711 reading scale for [%d]." CR), idx);
#endif

  PERF_BEGIN("scale-read");
  _hxScale[idx]->set_medavg_mode();
  float raw = _hxScale[idx]->get_units(myConfig.getScaleReadCount());

#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: HX711 Reading weight=%F [%d]" CR), raw, idx);
#endif

  // Validate raw reading
  bool isValid = true;
  if (raw > 100) {
    Log.error(F("SCAL: HX711 Ignoring value since it's higher than 100kg, %F "
                "[%d]." CR),
              raw, idx);
    PERF_END("scale-read");
    isValid = false;
    _stats.recordReading(static_cast<int>(idx), raw, isValid, millis());
    return _lastResult[idx];
  }

  if (raw < -100) {
    Log.error(
        F("SCAL: HX711 Ignoring value since it's less than -100kg %F [%d]." CR),
        raw, idx);
    PERF_END("scale-read");
    isValid = false;
    _stats.recordReading(static_cast<int>(idx), raw, isValid, millis());
    return _lastResult[idx];
  }

  // Update all filters with new raw reading
  ScaleReadingResult result;
  result.raw = raw;
  result.moving_average = _filter_ma[idx]->update(raw);
  result.ema = _filter_ema[idx]->update(raw);
  result.weighted_ma = _filter_wma[idx]->update(raw);
  result.median = _filter_median[idx]->update(raw);
  result.zscore = _filter_zscore[idx]->update(raw);
  result.hampel = _filter_hampel[idx]->update(raw);
  result.complementary = _filter_complementary[idx]->update(raw);
  result.alphabeta = _filter_alphabeta[idx]->update(raw);
  result.butterworth = _filter_butterworth[idx]->update(raw);
  result.fir = _filter_fir[idx]->update(raw);
  result.chebyshev = _filter_chebyshev[idx]->update(raw);
  result.kalman = _filter_kalman[idx]->update(raw);

  _lastResult[idx] = result;

  // Record statistics
  _stats.recordReading(static_cast<int>(idx), raw, isValid, millis());

  PERF_END("scale-read");
  return result;
}

void Scale::setupScale(UnitIndex idx, bool force, int pinData, int pinClock) {
  if (!_hxScale[idx] || force) {
#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: HX711 initializing scale, using offset %l [%d]." CR),
                myConfig.getScaleOffset(idx), idx);
#endif
    _hxScale[idx] = std::make_unique<HX711>();
    _hxScale[idx]->begin(pinData, pinClock, true, false);
    _hxScale[idx]->set_offset(myConfig.getScaleOffset(idx));
    Log.notice(
        F("SCAL: Initializing HX711 on pins Data=%d,Clock=%d,rate=%d [%d]." CR),
        pinData, pinClock, _hxScale[idx]->get_rate(), idx);
    setScaleFactor(idx);

    if (_hxScale[idx]->is_ready()) {
      Log.notice(F("SCAL: HX711 scale found [%d]." CR), idx);
    } else {
      Log.error(
          F("SCAL: HX711 scale not responding, disabling interface [%d]." CR),
          idx);
      _hxScale[idx].reset();
    }
  }
}

void Scale::setScaleFactor(UnitIndex idx) {
  if (!_hxScale[idx]) return;

  float fs = myConfig.getScaleFactor(idx);

  if (fs == 0.0) fs = 1.0;

  _hxScale[idx]->set_scale(fs);
}

void Scale::setup(bool force) {
  if (MAX_SCALES > 0)
    setupScale(UnitIndex::U1, force, PIN_SCALE_SDA1, PIN_SCALE_SCK1);

  if (MAX_SCALES > 1)
    setupScale(UnitIndex::U2, force, PIN_SCALE_SDA2, PIN_SCALE_SCK2);

  if (MAX_SCALES > 2)
    setupScale(UnitIndex::U3, force, PIN_SCALE_SDA3, PIN_SCALE_SCK3);

  if (MAX_SCALES > 3)
    setupScale(UnitIndex::U4, force, PIN_SCALE_SDA4, PIN_SCALE_SCK4);

  // Initialize all filters for each scale
  for (int i = 0; i < MAX_SCALES; ++i) {
    _filter_ma[i] = std::make_unique<MovingAverageFilter>(5);
    _filter_ema[i] = std::make_unique<ExponentialMovingAverageFilter>(0.3f);
    _filter_wma[i] = std::make_unique<WeightedMovingAverageFilter>(5);
    _filter_median[i] = std::make_unique<MedianFilter>(5);
    _filter_zscore[i] = std::make_unique<ModifiedZScoreFilter>(5, 3.5f);
    _filter_hampel[i] = std::make_unique<HampelFilter>(5, 3.0f);
    _filter_complementary[i] = std::make_unique<ComplementaryFilter>(0.7f);
    _filter_alphabeta[i] = std::make_unique<AlphaBetaFilter>(0.9f, 0.5f);
    _filter_butterworth[i] =
        std::make_unique<ButterworthLowPassFilter>(2.0f, 10.0f);
    _filter_fir[i] = std::make_unique<FIRLowPassFilter>(5);
    _filter_chebyshev[i] =
        std::make_unique<ChebyshevLowPassFilter>(2.0f, 10.0f);
    _filter_kalman[i] = std::make_unique<KalmanLowPassFilter>(0.001f, 0.1f);
  }

  Log.notice(F("SCAL: All filters initialized for all 4 scales."));
}

void Scale::loop() {
  for (int i = 0; i < MAX_SCALES; i++) loopScale((UnitIndex)i);
}

void Scale::loopScale(UnitIndex idx) {
  if (_sched[idx].tare) {
#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: Tare triggered [%d]." CR), idx);
#endif
    tare(idx);
    readRaw(idx);
    _sched[idx].tare = false;
  }

  if (_sched[idx].findFactor) {
#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: Find factor triggered [%d]." CR), idx);
#endif
    findFactor(idx, _sched[idx].factorWeight);
    readRaw(idx);
    _sched[idx].findFactor = false;
  }

  // Update all filters with latest reading
  read(idx);
}

// EOF
