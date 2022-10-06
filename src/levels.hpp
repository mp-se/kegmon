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
#ifndef SRC_LEVELS_HPP_
#define SRC_LEVELS_HPP_

#include <Arduino.h>
#include <SimpleKalmanFilter.h>
#include <Statistic.h>

#include <kegconfig.hpp>

class Stability {
 private:
  statistic::Statistic<float, uint32_t, true> _stability;

  // For viewing the stability of the scale over time. Uses raw / unfiltered
  // values.
 public:
  void clear() { _stability.clear(); }
  void add(float v) { _stability.add(v); }
  float sum() { return _stability.sum(); }
  float min() { return _stability.minimum(); }
  float max() { return _stability.maximum(); }
  float average() { return _stability.average(); }
  float variance() { return _stability.variance(); }
  float popStdev() { return _stability.pop_stdev(); }
  float unbiasedStdev() { return _stability.unbiased_stdev(); }
  uint32_t count() { return _stability.count(); }
};

class RawLevelDetection {
 private:
  static const int _cnt = 8;
  static const int _validCnt = 3;
  float _history[_cnt];
  float _last = NAN;

  // Stores the last n raw values to smooth out any faulty readings. Can be used
  // as a baseline/reference for other level detection methods.
 public:
  RawLevelDetection() { clear(); }

  bool hasValue() { return count() >= _validCnt ? true : false; }
  float getLastValue() { return _last; }
  float getValue() { return average(); }
  void clear() {
    for (int i = 0; i < _cnt; i++) _history[i] = NAN;

    _last = NAN;
  }
  void add(float v) {
    for (int i = _cnt - 1; i > 0; i--) _history[i] = _history[i - 1];
    _history[0] = v;
    _last = v;
  }
  float sum() {
    float sum = 0;
    for (int i = 0; i < _cnt; i++)
      if (!isnan(_history[i])) sum += _history[i];
    return sum;
  }
  float average() {
    int cnt = count();
    if (cnt == 0) return NAN;
    return sum() / cnt;
  }
  int count() {
    float cnt = 0;
    for (int i = 0; i < _cnt; i++)
      if (!isnan(_history[i])) cnt += 1;
    return cnt;
  }
};

#if defined ENABLE_KALMAN_LEVEL
class KalmanLevelDetection {
 private:
  UnitIndex _idx;
  SimpleKalmanFilter *_filter;
  float _baseline = NAN;
  float _raw = NAN;
  float _value = NAN;

  KalmanLevelDetection(const KalmanLevelDetection &) = delete;
  void operator=(const KalmanLevelDetection &) = delete;

  void checkForRefDeviation(float ref);

  // Kalman filter implentation, helps to smooth out sudden changes in
  // measurements, however it takes time to react to changes.
 public:
  explicit KalmanLevelDetection(UnitIndex idx) {
    _idx = idx;
    _filter = new SimpleKalmanFilter(1, 1, 0.01);
  }

  bool hasValue() { return !isnan(_value); }

  float getBaselineValue() { return _baseline; }
  float getRawValue() { return _raw; }
  float getValue() { return _value; }

  float processValue(float v, float ref);
};
#endif

class StatsLevelDetection {
 private:
  UnitIndex _idx;
  statistic::Statistic<float, uint32_t, true> _statistic;
  float _stable = NAN;
  float _pour = NAN;

  StatsLevelDetection(const StatsLevelDetection &) = delete;
  void operator=(const StatsLevelDetection &) = delete;

  void checkForMaxDeviation(float ref);
  void checkForStable(float ref);
  void checkForLevelChange(float ref);
  // void checkForRefDeviation(float ref);

  // Implementation of statistics for determine a stable level and also pour
  // detection.

 public:
  explicit StatsLevelDetection(UnitIndex idx) { _idx = idx; }

  bool hasStableValue() { return !isnan(_stable); }
  bool hasPourValue() { return !isnan(_pour); }

  float getStableValue() { return _stable; }
  float getPourValue() { return _pour; }

  void clear() { _statistic.clear(); }

  float min() { return _statistic.minimum(); }
  float max() { return _statistic.maximum(); }
  float ave() { return _statistic.average(); }
  float cnt() { return _statistic.count(); }

  float processValue(float v, float ref);
};

#endif  // SRC_LEVELS_HPP_

// EOF
