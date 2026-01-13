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
#ifndef SRC_SCALE_HPP_
#define SRC_SCALE_HPP_

#include <HX711.h>

#include <atomic>
#include <filters/filter_base.hpp>
#include <filters/filter_kalman.hpp>
#include <kegconfig.hpp>
#include <main.hpp>
#include <memory>
#include <perf.hpp>
#include <scale_statistics.hpp>

// Result structure containing raw reading and all filter outputs
struct ScaleReadingResult {
  float raw = 0.0f;
  float moving_average = 0.0f;
  float ema = 0.0f;
  float weighted_ma = 0.0f;
  float median = 0.0f;
  float zscore = 0.0f;
  float hampel = 0.0f;
  float complementary = 0.0f;
  float alphabeta = 0.0f;
  float butterworth = 0.0f;
  float fir = 0.0f;
  float chebyshev = 0.0f;
  float kalman = 0.0f;

  // Get filter value by FilterType enum
  inline float getFilterValue(FilterType filter) const {
    switch (filter) {
      case FilterType::FILTER_RAW:
        return raw;
      case FilterType::FILTER_MOVING_AVERAGE:
        return moving_average;
      case FilterType::FILTER_EMA:
        return ema;
      case FilterType::FILTER_WEIGHTED_MA:
        return weighted_ma;
      case FilterType::FILTER_MEDIAN:
        return median;
      case FilterType::FILTER_ZSCORE:
        return zscore;
      case FilterType::FILTER_HAMPEL:
        return hampel;
      case FilterType::FILTER_COMPLEMENTARY:
        return complementary;
      case FilterType::FILTER_ALPHABETA:
        return alphabeta;
      case FilterType::FILTER_BUTTERWORTH:
        return butterworth;
      case FilterType::FILTER_FIR:
        return fir;
      case FilterType::FILTER_CHEBYSHEV:
        return chebyshev;
      case FilterType::FILTER_KALMAN:
        return kalman;
      default:
        return NAN;
    }
  }

  // Get filter name by FilterType enum
  inline const char* getFilterName(FilterType filter) const {
    switch (filter) {
      case FilterType::FILTER_RAW:
        return "Raw";
      case FilterType::FILTER_MOVING_AVERAGE:
        return "Moving Average";
      case FilterType::FILTER_EMA:
        return "EMA";
      case FilterType::FILTER_WEIGHTED_MA:
        return "Weighted MA";
      case FilterType::FILTER_MEDIAN:
        return "Median";
      case FilterType::FILTER_ZSCORE:
        return "Z-Score";
      case FilterType::FILTER_HAMPEL:
        return "Hampel";
      case FilterType::FILTER_COMPLEMENTARY:
        return "Complementary";
      case FilterType::FILTER_ALPHABETA:
        return "Alpha-Beta";
      case FilterType::FILTER_BUTTERWORTH:
        return "Butterworth";
      case FilterType::FILTER_FIR:
        return "FIR";
      case FilterType::FILTER_CHEBYSHEV:
        return "Chebyshev";
      case FilterType::FILTER_KALMAN:
        return "Kalman";
      default:
        return "Unknown";
    }
  }
};

class Scale {
 private:
  class Schedule {
   public:
    std::atomic<bool> tare = false;
    std::atomic<bool> findFactor = false;
    float factorWeight = 0.0;  // Protected by factorWeight atomic read/write
  };

  std::unique_ptr<HX711> _hxScale[MAX_SCALES] = {nullptr, nullptr, nullptr, nullptr};

  Schedule _sched[MAX_SCALES];
  float _lastRaw[MAX_SCALES] = {0.0, 0.0, 0.0, 0.0};

  // Filter instances
  std::unique_ptr<ScaleFilter> _filter_ma[MAX_SCALES];
  std::unique_ptr<ScaleFilter> _filter_ema[MAX_SCALES];
  std::unique_ptr<ScaleFilter> _filter_wma[MAX_SCALES];
  std::unique_ptr<ScaleFilter> _filter_median[MAX_SCALES];
  std::unique_ptr<ScaleFilter> _filter_zscore[MAX_SCALES];
  std::unique_ptr<ScaleFilter> _filter_hampel[MAX_SCALES];
  std::unique_ptr<ScaleFilter> _filter_complementary[MAX_SCALES];
  std::unique_ptr<ScaleFilter> _filter_alphabeta[MAX_SCALES];
  std::unique_ptr<ScaleFilter> _filter_butterworth[MAX_SCALES];
  std::unique_ptr<ScaleFilter> _filter_fir[MAX_SCALES];
  std::unique_ptr<ScaleFilter> _filter_chebyshev[MAX_SCALES];
  std::unique_ptr<ScaleFilter> _filter_kalman[MAX_SCALES];

  // Latest reading results for each scale
  ScaleReadingResult _lastResult[MAX_SCALES];

  // Statistics manager
  ScaleStatisticsManager _stats;

  Scale(const Scale&) = delete;
  void operator=(const Scale&) = delete;

  void tare(UnitIndex idx);
  void findFactor(UnitIndex idx, float weight);
  void setupScale(UnitIndex idx, bool force, int pinData, int pinClock);
  void loopScale(UnitIndex idx);
  void setScaleFactor(UnitIndex idx);
  float readRaw(UnitIndex idx);

 public:
  Scale() = default;
  ~Scale() = default;

  void setup(bool force = false);
  void loop();
  bool isReady(UnitIndex idx) const;

  void scheduleTare(UnitIndex idx) { _sched[idx].tare = true; }
  void scheduleFindFactor(UnitIndex idx, float weight) {
    _sched[idx].findFactor = true;
    _sched[idx].factorWeight = weight;
  }
  bool isScheduleRunning() const {
    for (int i = 0; i < MAX_SCALES; i++) {
      if (_sched[i].findFactor || _sched[i].tare) {
        return true;
      }
    }
    return false;
  }
  int32_t readLastRaw(UnitIndex idx) { return _lastRaw[idx]; }
  bool isConnected(UnitIndex idx) const { return _hxScale[idx] ? true : false; }
  ScaleReadingResult read(UnitIndex idx);

  // Get the last reading result (for thread-safe access from main loop)
  inline ScaleReadingResult getLastResult(UnitIndex idx) const {
    return _lastResult[static_cast<int>(idx)];
  }

  // Statistics access
  const ScaleStatistics& getStatistics(UnitIndex idx) const {
    return _stats.getStatistics(static_cast<int>(idx));
  }

  void resetStatistics(UnitIndex idx) {
    _stats.resetStatistics(static_cast<int>(idx));
  }

  void resetAllStatistics() { _stats.resetAllStatistics(); }
};

extern Scale myScale;

#endif  // SRC_SCALE_HPP_

// EOF
