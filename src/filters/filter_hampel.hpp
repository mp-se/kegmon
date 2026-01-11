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
#ifndef SRC_FILTERS_FILTER_HAMPEL_HPP_
#define SRC_FILTERS_FILTER_HAMPEL_HPP_

#include <filters/filter_base.hpp>

#include <algorithm>
#include <cmath>
#include <deque>
#include <vector>

class HampelFilter : public ScaleFilter {
 private:
  size_t _window_size;
  std::deque<float> _buffer;
  float _threshold;
  float _variance = 0.0f;
  bool _outlier_detected = false;

 public:
  explicit HampelFilter(size_t window_size = 5, float threshold = 3.0f)
      : _window_size(window_size), _threshold(threshold) {}

  float update(float raw_value) override {
    _outlier_detected = false;
    _buffer.push_back(raw_value);
    if (_buffer.size() > _window_size) {
      _buffer.pop_front();
    }

    if (_buffer.size() < 2) {
      return raw_value;
    }

    size_t center_idx = _buffer.size() / 2;
    float center = _buffer[center_idx];

    std::vector<float> sorted(_buffer.begin(), _buffer.end());
    std::sort(sorted.begin(), sorted.end());

    float median;
    if (sorted.size() % 2 == 0) {
      median = (sorted[sorted.size() / 2 - 1] + sorted[sorted.size() / 2]) / 2.0f;
    } else {
      median = sorted[sorted.size() / 2];
    }

    std::vector<float> deviations;
    for (float val : sorted) {
      deviations.push_back(std::abs(val - median));
    }
    std::sort(deviations.begin(), deviations.end());

    float mad;
    if (deviations.size() % 2 == 0) {
      mad = (deviations[deviations.size() / 2 - 1] + deviations[deviations.size() / 2]) / 2.0f;
    } else {
      mad = deviations[deviations.size() / 2];
    }

    float dev_from_median = std::abs(center - median);
    if (dev_from_median > _threshold * mad) {
      _outlier_detected = true;
      return median;
    }

    float var_sum = 0.0f;
    for (float val : sorted) {
      var_sum += (val - median) * (val - median);
    }
    _variance = var_sum / sorted.size();

    return center;
  }

  void reset() override {
    _buffer.clear();
    _variance = 0.0f;
    _outlier_detected = false;
  }

  std::string getName() const override { return "Hampel"; }
  float getVariance() const override { return _variance; }
  bool isOutlierDetected() const { return _outlier_detected; }
};

#endif  // SRC_FILTERS_FILTER_HAMPEL_HPP_
// EOF
