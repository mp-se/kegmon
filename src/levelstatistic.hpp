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
#ifndef SRC_LEVELSTATISTIC_HPP_
#define SRC_LEVELSTATISTIC_HPP_

#include <Arduino.h>
#include <Statistic.h>

#include <kegconfig.hpp>
#include <main.hpp>

class StatsLevelDetection {
 private:
  UnitIndex _idx;
  statistic::Statistic<float, uint32_t, true> _statistic;
  float _stable = NAN;
  float _pour = NAN;
  bool _newPour = false;
  bool _newStable = false;

  StatsLevelDetection(const StatsLevelDetection &) = delete;
  void operator=(const StatsLevelDetection &) = delete;

  bool checkForValidValue(float raw, float kalman) {
    float delta = abs(kalman - raw);

    // Log.notice(
    //    F("LVL : Valid delta %F [%d]." CR),
    //    delta, _idx);

    if (delta < myConfig.getKalmanMaxDeviationValue()) {
      return true;
    }

    Log.notice(F("LVL : Raw and Kalman values differ to much %F, not yet "
                 "stable value [%d]." CR),
               delta, _idx);
    return false;
  }

  void checkForMaxDeviation(float v) {
    if (cnt() > 0) {
      float delta = abs(ave() - v);

      if (delta > myConfig.getScaleMaxDeviationDecreaseValue()) {
        Log.notice(
            F("LVL : Average statistics deviates too much from raw values "
              "%F, restarting stable level detection, ave=%F, cnt=%F [%d]." CR),
            delta, ave(), cnt(), _idx);
        clear();
      }
    }
  }

  void checkForStable() {
    if (cnt() > myConfig.getScaleStableCount() && isnan(_stable)) {
      _stable = ave();
      _newStable = true;
      Log.notice(
          F("LVL : Found a new stable value %F, ave=%F, cnt=%F [%d]." CR),
          getStableValue(), ave(), cnt(), _idx);
    }
  }

  void checkForLevelChange() {
    // Check if the level has changed up or down. If its down we record the
    // delta as the latest pour.
    if (cnt() > myConfig.getScaleStableCount() && !isnan(_stable)) {
      if ((_stable + myConfig.getScaleMaxDeviationIncreaseValue()) < ave()) {
        Log.notice(F("LVL : Level has increased, adjusting from %F to %F, "
                     "cnt=%F [%d]." CR),
                   _stable, ave(), cnt(), _idx);
        _stable = ave();
        _newStable = true;
      } else if ((_stable - myConfig.getScaleMaxDeviationDecreaseValue()) > ave()) {
        Log.notice(F("LVL : Level has decreased, adjusting from %F to %F, "
                     "cnt=%F [%d]." CR),
                   _stable, ave(), cnt(), _idx);

        float p = _stable - ave();
        _stable = ave();
        _newStable = true;

        // Check if the keg was removed so we dont register a too large pour
        if ((_stable - myConfig.getKegWeight(_idx)) < 0) {
          _pour -= myConfig.getKegWeight(_idx);
          if (p > 0.0) {
            _pour = p;
            Log.notice(F("LVL : Keg removed and beer has been poured volume %F "
                         "[%d]." CR),
                       _pour, _idx);
          }
        } else {
          _pour = p;
          Log.notice(F("LVL : Beer has been poured volume %F [%d]." CR), _pour,
                     _idx);
        }

        // Notify registered endpoints and save to log
        _newPour = true;
      }
    }
  }

  // Implementation of statistics for determine a stable level and also pour
  // detection.
 public:
  explicit StatsLevelDetection(UnitIndex idx) { _idx = idx; }

  bool hasStableValue() { return !isnan(_stable); }
  bool hasPourValue() { return !isnan(_pour); }

  float getValue() { return ave(); }
  float getStableValue() { return _stable; }
  float getPourValue() { return _pour; }

  bool newPourValue() { return _newPour; }
  bool newStableValue() { return _newStable; }

  void clear() { _statistic.clear(); }
  float min() { return _statistic.minimum(); }
  float max() { return _statistic.maximum(); }
  float ave() { return _statistic.average(); }
  float cnt() { return _statistic.count(); }

  float processValue(float raw, float kalman) {
    _newPour = false;
    _newStable = false;

    if (isnan(raw) || isnan(kalman)) return NAN;

    if (checkForValidValue(raw, kalman)) {
      _statistic.add(kalman);
      checkForMaxDeviation(kalman);
      checkForStable();
      checkForLevelChange();
#if LOG_DEBUG == 6
      Log.verbose(
          F("LVL : Update statistics raw=%F kalman=%F ave=%F min=%F max=%F "
            "[%d]." CR),
          raw, kalman, ave(), min(), max(), _idx);
#endif
      return ave();
    }

    return NAN;
  }
};

#endif  // SRC_LEVELSTATISTIC_HPP_

// EOF
