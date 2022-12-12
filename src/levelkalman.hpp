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
#ifndef SRC_LEVELKALMAN_HPP_
#define SRC_LEVELKALMAN_HPP_

#include <SimpleKalmanFilter.h>

#include <main.hpp>

class KalmanLevelDetection {
 private:
  UnitIndex _idx;
  SimpleKalmanFilter *_filter;
  int _cnt = 0;
  float _baseline = NAN;
  float _raw = NAN;
  float _value = NAN;

  KalmanLevelDetection(const KalmanLevelDetection &) = delete;
  void operator=(const KalmanLevelDetection &) = delete;

  // void checkForRefDeviation(float ref);
  void checkForMaxDeviation(float ref);
  void checkForStable(float ref);
  void checkForLevelChange(float ref);

  // Kalman filter implentation, helps to smooth out sudden changes in
  // measurements, however it takes time to react to changes.
 public:
  explicit KalmanLevelDetection(UnitIndex idx) {
    _idx = idx;
    _filter = new SimpleKalmanFilter(5, 3, 0.3);
  }

  bool hasValue() { return !isnan(_value); }
  bool hasPourValue() { return false; }
  bool hasStableValue() { return false; }

  float getBaselineValue() { return _baseline; }
  float getRawValue() { return _raw; }
  float getValue() { return _value; }
  float getStableValue() { return NAN; }
  float getPourValue() { return NAN; }

  float processValue(float v, float ref);
};

#endif  // SRC_LEVELKALMAN_HPP_

// EOF
