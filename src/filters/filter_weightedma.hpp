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
#ifndef SRC_FILTERS_FILTER_WEIGHTEDMA_HPP_
#define SRC_FILTERS_FILTER_WEIGHTEDMA_HPP_

#include <filters/filter_base.hpp>

#include <deque>

class WeightedMovingAverageFilter : public ScaleFilter {
 private:
  size_t _window_size;
  std::deque<float> _buffer;
  float _variance = 0.0f;

 public:
  explicit WeightedMovingAverageFilter(size_t window_size = 5)
      : _window_size(window_size) {}

  float update(float raw_value) override {
    _buffer.push_back(raw_value);
    if (_buffer.size() > _window_size) {
      _buffer.pop_front();
    }

    float weighted_sum = 0.0f;
    float weight_sum = 0.0f;
    for (size_t i = 0; i < _buffer.size(); ++i) {
      float weight = (i + 1.0f) / _buffer.size();
      weighted_sum += _buffer[i] * weight;
      weight_sum += weight;
    }
    float weighted_avg = weighted_sum / weight_sum;

    float var_sum = 0.0f;
    for (float val : _buffer) {
      var_sum += (val - weighted_avg) * (val - weighted_avg);
    }
    _variance = var_sum / _buffer.size();

    return weighted_avg;
  }

  void reset() override {
    _buffer.clear();
    _variance = 0.0f;
  }

  std::string getName() const override { return "WeightedMA"; }
  float getVariance() const override { return _variance; }
};

#endif  // SRC_FILTERS_FILTER_WEIGHTEDMA_HPP_
// EOF
