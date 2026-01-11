/*
MIT License

Copyright (c) 2026 Magnus

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
#ifndef SRC_FILTERS_FILTER_CHEBYSHEV_HPP_
#define SRC_FILTERS_FILTER_CHEBYSHEV_HPP_

#include <filters/filter_base.hpp>

#include <cmath>
#include <deque>

class ChebyshevLowPassFilter : public ScaleFilter {
 private:
  float _ripple_db = 0.5f;  // dB
  float _cutoff_frequency = 2.0f;
  float _sampling_rate = 10.0f;
  float _y1 = 0.0f;
  float _y2 = 0.0f;
  float _x1 = 0.0f;
  float _x2 = 0.0f;
  float _variance = 0.0f;
  std::deque<float> _buffer;
  bool _initialized = false;

  float calculateAlpha() {
    float wc = 2.0f * 3.14159265359f * _cutoff_frequency / _sampling_rate;
    float epsilon = std::sqrt(std::pow(10.0f, _ripple_db / 10.0f) - 1.0f);
    float cosh_inv =
        std::log(1.0f / epsilon + std::sqrt(1.0f / (epsilon * epsilon) + 1.0f));
    float sinh_inv = std::sinh(cosh_inv / 1.0f);

    float q = 2.0f * std::sin(3.14159265359f / 4.0f) * std::sinh(sinh_inv);
    float alpha = std::sin(wc) / (2.0f * q);

    return alpha;
  }

 public:
  explicit ChebyshevLowPassFilter(float cutoff_frequency = 2.0f,
                                  float sampling_rate = 10.0f,
                                  float ripple_db = 0.5f)
      : _ripple_db(ripple_db),
        _cutoff_frequency(cutoff_frequency),
        _sampling_rate(sampling_rate) {}

  float update(float raw_value) override {
    if (!_initialized) {
      _y1 = raw_value;
      _y2 = raw_value;
      _x1 = raw_value;
      _x2 = raw_value;
      _initialized = true;
    }

    float alpha = calculateAlpha();
    float filtered = (1.0f - alpha) * _y1 + alpha * raw_value;

    _buffer.push_back(filtered);
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

    _x2 = _x1;
    _x1 = raw_value;
    _y2 = _y1;
    _y1 = filtered;

    return filtered;
  }

  void reset() override {
    _y1 = 0.0f;
    _y2 = 0.0f;
    _x1 = 0.0f;
    _x2 = 0.0f;
    _initialized = false;
    _buffer.clear();
    _variance = 0.0f;
  }

  std::string getName() const override { return "Chebyshev"; }
  float getVariance() const override { return _variance; }
};

#endif  // SRC_FILTERS_FILTER_CHEBYSHEV_HPP_
// EOF
