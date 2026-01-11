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
#ifndef SRC_FILTERS_FILTER_ZSCORE_HPP_
#define SRC_FILTERS_FILTER_ZSCORE_HPP_

#include <filters/filter_base.hpp>

#include <algorithm>
#include <cmath>
#include <deque>
#include <vector>

class ModifiedZScoreFilter : public ScaleFilter {
 private:
  size_t _window_size;
  std::deque<float> _buffer;
  float _threshold;
  float _variance = 0.0f;
  bool _outlier_detected = false;

 public:
  explicit ModifiedZScoreFilter(size_t window_size = 5, float threshold = 3.5f)
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

    float modified_zscore = 0.6745f * (raw_value - median) / (mad + 1e-6f);

    if (std::abs(modified_zscore) > _threshold) {
      _outlier_detected = true;
      return median;
    }

    float var_sum = 0.0f;
    for (float val : sorted) {
      var_sum += (val - median) * (val - median);
    }
    _variance = var_sum / sorted.size();

    return raw_value;
  }

  void reset() override {
    _buffer.clear();
    _variance = 0.0f;
    _outlier_detected = false;
  }

  std::string getName() const override { return "ZScore"; }
  float getVariance() const override { return _variance; }
  bool isOutlierDetected() const { return _outlier_detected; }
};

#endif  // SRC_FILTERS_FILTER_ZSCORE_HPP_
// EOF
