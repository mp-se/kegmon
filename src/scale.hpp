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
#include <Statistic.h>
#include <SimpleKalmanFilter.h>

#include <main.hpp>

// Note! Internally we assume that all weights are in KG and all volumes are in Liters.

class LevelDetection {
 private:
  float _filterBaseline = NAN;
  SimpleKalmanFilter *_filter;

 public:
  LevelDetection() {
    _filter = new SimpleKalmanFilter(1, 1, 1);
  }
  float _lastStableWeight = NAN;
  float _lastAverageWeight = NAN;
  float _lastFilterOutput = NAN;

  statistic::Statistic<float, uint32_t, true> _statistic;
  statistic::Statistic<float, uint32_t, true> _stability;

  float applyKalmanFilter(float value);
};

class Scale {
 private:
  HX711 *_scale[2] = {0, 0};
  float _lastWeight[2] = {0, 0};
  float _lastPourWeight[2] = {NAN, NAN};
  LevelDetection _detection[2];

  void setScaleFactor(UnitIndex idx);
  void statsAdd(UnitIndex idx, float val) {
    _detection[idx]._statistic.add(val);
    _detection[idx]._stability.add(val);
  }

  void validateScaleLevel(UnitIndex idx);
  void checkForPour(UnitIndex idx);

 public:
  Scale();

  // Setup and status
  void setup(bool force = false);
  void tare(UnitIndex idx);
  void findFactor(UnitIndex idx, float weight);
  bool isConnected(UnitIndex idx) { return _scale[idx] != 0 ? true : false; }
  bool hasLastStableWeight(UnitIndex idx) {
    return !isnan(_detection[idx]._lastStableWeight);
  }
  bool hasPourWeight(UnitIndex idx) { return !isnan(_lastPourWeight[idx]); }

  // Read from scale
  int32_t readRawWeight(UnitIndex idx);
  float readWeight(UnitIndex idx, bool updateStats = false);

  float getLastWeight(UnitIndex idx) { return _lastWeight[idx]; }
  float getLastFilterWeight(UnitIndex idx) { return _detection[idx]._lastFilterOutput; }
  float getLastVolume(UnitIndex idx);
  float getAverageWeight(UnitIndex idx) {
    return statsCount(idx) > 0 ? statsAverage(idx) : _lastWeight[idx];
  }

  float getLastStableWeight(UnitIndex idx) { return _detection[idx]._lastStableWeight; }
  float getPourWeight(UnitIndex idx) { return _lastPourWeight[idx]; }
  float getPourVolume(UnitIndex idx);

  // Helper methods
  float calculateNoGlasses(UnitIndex idx);

  // Statistics
  float getAverageWeightDirectionCoefficient(UnitIndex idx);
  void statsClearAll() {
    statsClear(UnitIndex::U1);
    statsClear(UnitIndex::U2);
  }
  void statsClear(UnitIndex idx) { _detection[idx]._statistic.clear(); }
  uint32_t statsCount(UnitIndex idx) { return _detection[idx]._statistic.count(); }
  float statsSum(UnitIndex idx) { return _detection[idx]._statistic.sum(); }
  float statsMin(UnitIndex idx) { return _detection[idx]._statistic.minimum(); }
  float statsMax(UnitIndex idx) { return _detection[idx]._statistic.maximum(); }
  float statsAverage(UnitIndex idx) { return _detection[idx]._statistic.average(); }
  float statsVariance(UnitIndex idx) { return _detection[idx]._statistic.variance(); }
  float statsPopStdev(UnitIndex idx) { return _detection[idx]._statistic.pop_stdev(); }
  float statsUnbiasedStdev(UnitIndex idx) {
    return _detection[idx]._statistic.unbiased_stdev();
  }

  // Stability since start
  void stabilityClearAll() {
    stabilityClear(UnitIndex::U1);
    stabilityClear(UnitIndex::U2);
  }
  void stabilityClear(UnitIndex idx) { _detection[idx]._stability.clear(); }
  uint32_t stabilityCount(UnitIndex idx) { return _detection[idx]._stability.count(); }
  float stabilitySum(UnitIndex idx) { return _detection[idx]._stability.sum(); }
  float stabilityMin(UnitIndex idx) { return _detection[idx]._stability.minimum(); }
  float stabilityMax(UnitIndex idx) { return _detection[idx]._stability.maximum(); }
  float stabilityAverage(UnitIndex idx) { return _detection[idx]._stability.average(); }
  float stabilityVariance(UnitIndex idx) { return _detection[idx]._stability.variance(); }
  float stabilityPopStdev(UnitIndex idx) { return _detection[idx]._stability.pop_stdev(); }
  float stabilityUnbiasedStdev(UnitIndex idx) {
    return _detection[idx]._stability.unbiased_stdev();
  }
};

extern Scale myScale;

#endif  // SRC_SCALE_HPP_

// EOF
