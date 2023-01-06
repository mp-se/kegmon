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
#ifndef SRC_LEVELRAW_HPP_
#define SRC_LEVELRAW_HPP_

#include <Arduino.h>
#include <SimpleKalmanFilter.h>

class RawLevelDetection {
 private:
  static const int _cnt = 10;
  static const int _validCnt = 5;
  float _history[_cnt];
  float _last = NAN;
  float _kalman = NAN;
  SimpleKalmanFilter *_kalmanFilter = 0;

  // Stores the last n raw values to smooth out any faulty readings. Can be used
  // as a baseline/reference for other level detection methods.
 public:
  RawLevelDetection() { 
    clear(); 
    _kalmanFilter = new SimpleKalmanFilter(0.001, 0.001, 0.001);
    // _kalmanFilter = new SimpleKalmanFilter(myConfig.getKalmanMeasurement(), myConfig.getKalmanEstimation(), myConfig.getKalmanNoise());
  }

  bool hasRawValue() { return isnan(_last) ? false : true; }
  bool hasAverageValue() { return count() >= _validCnt ? true : false; }
  float getRawValue() { return _last; }
  float getAverageValue() { return average(); }

  bool hasKalmanValue() { return isnan(_kalman) ? false : true; }
  float getKalmanValue() { return _kalman; }

  void clear() {
    for (int i = 0; i < _cnt; i++) _history[i] = NAN;
    _last = NAN;
    _kalman = NAN;
  }
  void add(float v) {
    for (int i = _cnt - 1; i > 0; i--) _history[i] = _history[i - 1];
    _history[0] = v;
    _last = v;
    float k = _kalmanFilter->updateEstimate(v);

    if (hasAverageValue()) { // Only present value when we have enough sensor reads
      _kalman = k;
      // Log.notice(F("LVL : Kalman value %F, esterr=%F, gain=%F" CR), k, _kalmanFilter->getEstimateError(), _kalmanFilter->getKalmanGain());
    }
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

#endif  // SRC_LEVELRAW_HPP_

// EOF
