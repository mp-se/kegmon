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
#ifndef SRC_LEVELRAW_HPP_
#define SRC_LEVELRAW_HPP_

#include <Arduino.h>
#include <SimpleKalmanFilter.h>
#include <tinyexpr.h>

#include <kegconfig.hpp>
#include <main.hpp>
#include <utils.hpp>

class RawLevelDetection {
 private:
  UnitIndex _idx;

  // Raw values
  static const int _cnt = 10;
  static const int _validCnt = 5;
  float _history[_cnt];
  float _last = NAN;

  // Kalman filter
  float _kalman = NAN;
  SimpleKalmanFilter *_kalmanFilter = 0;

  // Temperature correction filter
  float _tempCorr = NAN;

  // Slope filter
  float _slope = NAN;

  RawLevelDetection(const RawLevelDetection &) = delete;
  void operator=(const RawLevelDetection &) = delete;

  // Stores the last n raw values to smooth out any faulty readings. Can be used
  // as a baseline/reference for other level detection methods.
 public:
  RawLevelDetection(UnitIndex idx, float kalmanMea, float _kalmanEst,
                    float kalmanNoise) {
    clear();
    _idx = idx;
    _kalmanFilter = new SimpleKalmanFilter(kalmanMea, _kalmanEst, kalmanNoise);
  }

  bool hasRawValue() { return isnan(_last) ? false : true; }
  bool hasAverageValue() { return count() >= _validCnt ? true : false; }
  float getRawValue() { return _last; }
  float getAverageValue() { return average(); }

  bool hasKalmanValue() { return isnan(_kalman) ? false : true; }
  float getKalmanValue() { return _kalman; }

  bool hasTempCorrValue() { return isnan(_tempCorr) ? false : true; }
  float getTempCorrValue() { return _tempCorr; }

  bool hasSlopeValue() { return isnan(_slope) ? false : true; }
  float getSlopeValue() { return _slope; }
  bool slopeRising() { return _slope > 0.0 ? true : false; }
  bool slopeSinking() { return _slope < 0.0 ? true : false; }

  void clear() {
    for (int i = 0; i < _cnt; i++) _history[i] = NAN;
    _last = NAN;
    _kalman = NAN;
    _tempCorr = NAN;
  }
  void add(float v, float temp) {
    // Raw values
    for (int i = _cnt - 1; i > 0; i--) _history[i] = _history[i - 1];
    _history[0] = v;
    _last = v;

    // Temperature correction
    _tempCorr = NAN;
    const char *formula = myConfig.getScaleTempCompensationFormula(_idx);

    if (strlen(formula) > 0) {
      double weight = v;
      double tempC = temp;
      double tempF = convertCtoF(tempC);
      int err;
      te_variable vars[] = {
          {"weight", &weight}, {"tempC", &tempC}, {"tempF", &tempF}};
      te_expr *expr = te_compile(formula, vars, 3, &err);

      if (expr) {
        _tempCorr = te_eval(expr);
        te_free(expr);
      }

      Log.notice(F("LVL : %F -> %F" CR), v, _tempCorr);
    }

    // Slope calculation
    if (count() == _cnt) {
      _slope = _history[0] - _history[_cnt - 1];
    }

    // Kalman filter
    float k = _kalmanFilter->updateEstimate(isnan(_tempCorr) ? v : _tempCorr);
    if (hasAverageValue()) {  // Only present value when we have enough sensor
                              // reads
      _kalman = k;
      // Log.notice(F("LVL : Kalman value %F, esterr=%F, gain=%F" CR), k,
      // _kalmanFilter->getEstimateError(), _kalmanFilter->getKalmanGain());
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
