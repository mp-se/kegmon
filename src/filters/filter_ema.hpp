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
#ifndef SRC_FILTERS_FILTER_EMA_HPP_
#define SRC_FILTERS_FILTER_EMA_HPP_

#include <filters/filter_base.hpp>

class ExponentialMovingAverageFilter : public ScaleFilter {
 private:
  float _alpha;
  float _last_value = 0.0f;
  bool _initialized = false;
  float _variance = 0.0f;

 public:
  explicit ExponentialMovingAverageFilter(float alpha = 0.3f) : _alpha(alpha) {}

  float update(float raw_value) override {
    if (!_initialized) {
      _last_value = raw_value;
      _initialized = true;
      _variance = 0.0f;
      return raw_value;
    }

    float filtered = _alpha * raw_value + (1.0f - _alpha) * _last_value;
    _variance = _alpha * (raw_value - filtered) * (raw_value - filtered) +
                (1.0f - _alpha) * _variance;
    _last_value = filtered;
    return filtered;
  }

  void reset() override {
    _initialized = false;
    _last_value = 0.0f;
    _variance = 0.0f;
  }

  std::string getName() const override { return "EMA"; }
  float getVariance() const override { return _variance; }
};

#endif  // SRC_FILTERS_FILTER_EMA_HPP_
// EOF
