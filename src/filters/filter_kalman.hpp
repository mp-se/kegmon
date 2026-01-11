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
#ifndef SRC_FILTERS_FILTER_KALMAN_HPP_
#define SRC_FILTERS_FILTER_KALMAN_HPP_

#include <filters/filter_base.hpp>

#include <cmath>
#include <deque>

class KalmanLowPassFilter : public ScaleFilter {
 private:
  float _process_noise = 0.001f;  // Q - how much we expect state to change
  float _measurement_noise = 0.1f;  // R - measurement uncertainty
  float _estimate_error = 1.0f;   // P - estimate error covariance
  float _state = 0.0f;            // x - current state estimate
  float _variance = 0.0f;
  std::deque<float> _buffer;
  bool _initialized = false;

 public:
  explicit KalmanLowPassFilter(float process_noise = 0.001f,
                               float measurement_noise = 0.1f)
      : _process_noise(process_noise),
        _measurement_noise(measurement_noise) {}

  float update(float raw_value) override {
    if (!_initialized) {
      _state = raw_value;
      _initialized = true;
    }

    // Predict step
    float predicted_error = _estimate_error + _process_noise;

    // Update step: Calculate Kalman gain
    float kalman_gain =
        predicted_error / (predicted_error + _measurement_noise);

    // Update state estimate with measurement
    float innovation = raw_value - _state;
    _state = _state + kalman_gain * innovation;

    // Update estimate error
    _estimate_error = (1.0f - kalman_gain) * predicted_error;

    // Track variance for diagnostics
    _buffer.push_back(_state);
    if (_buffer.size() > 5) {
      _buffer.pop_front();
    }

    float mean = 0.0f;
    for (float val : _buffer) {
      mean += val;
    }
    mean /= _buffer.size();

    float var_sum = 0.0f;
    for (float val : _buffer) {
      var_sum += (val - mean) * (val - mean);
    }
    _variance = var_sum / _buffer.size();

    return _state;
  }

  void reset() override {
    _state = 0.0f;
    _estimate_error = 1.0f;
    _initialized = false;
    _buffer.clear();
    _variance = 0.0f;
  }

  std::string getName() const override { return "Kalman"; }
  float getVariance() const override { return _variance; }

  // Getter for state estimate (useful for diagnostics)
  float getState() const { return _state; }

  // Getter for estimate error (uncertainty)
  float getEstimateError() const { return _estimate_error; }
};

#endif  // SRC_FILTERS_FILTER_KALMAN_HPP_
// EOF
