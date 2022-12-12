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

  // void checkForRefDeviation(float ref);
  void checkForMaxDeviation(float ref);
  void checkForStable(float ref);
  void checkForLevelChange(float ref);

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

  float processValue(float v, float ref);
};

#endif  // SRC_LEVELSTATISTIC_HPP_

// EOF
