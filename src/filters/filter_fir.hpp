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
#ifndef SRC_FILTERS_FILTER_FIR_HPP_
#define SRC_FILTERS_FILTER_FIR_HPP_

#include <filters/filter_base.hpp>

#include <deque>
#include <vector>

static const std::vector<float> fir_coefficients_order5 = {
    0.1f, 0.2f, 0.4f, 0.2f, 0.1f};
static const std::vector<float> fir_coefficients_order7 = {
    0.05f, 0.1f, 0.2f, 0.3f, 0.2f, 0.1f, 0.05f};

class FIRLowPassFilter : public ScaleFilter {
 private:
  std::vector<float> _coefficients;
  std::deque<float> _buffer;
  float _variance = 0.0f;

 public:
  explicit FIRLowPassFilter(size_t order = 5) {
    if (order <= 5) {
      _coefficients = fir_coefficients_order5;
    } else {
      _coefficients = fir_coefficients_order7;
    }
  }

  float update(float raw_value) override {
    _buffer.push_back(raw_value);
    if (_buffer.size() > _coefficients.size()) {
      _buffer.pop_front();
    }

    float filtered = 0.0f;
    for (size_t i = 0; i < _buffer.size(); ++i) {
      filtered += _buffer[i] * _coefficients[i];
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

    return filtered;
  }

  void reset() override {
    _buffer.clear();
    _variance = 0.0f;
  }

  std::string getName() const override { return "FIR"; }
  float getVariance() const override { return _variance; }
};

#endif  // SRC_FILTERS_FILTER_FIR_HPP_
// EOF
