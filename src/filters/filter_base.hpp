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
#ifndef SRC_FILTERS_FILTER_BASE_HPP_
#define SRC_FILTERS_FILTER_BASE_HPP_

#include <string>

// Enum for all available filter types
enum class FilterType {
  FILTER_RAW = 0,
  FILTER_MOVING_AVERAGE,
  FILTER_EMA,
  FILTER_WEIGHTED_MA,
  FILTER_MEDIAN,
  FILTER_ZSCORE,
  FILTER_HAMPEL,
  FILTER_COMPLEMENTARY,
  FILTER_ALPHABETA,
  FILTER_BUTTERWORTH,
  FILTER_FIR,
  FILTER_CHEBYSHEV,
  FILTER_KALMAN
};

class ScaleFilter {
 public:
  virtual ~ScaleFilter() = default;

  // Process a new raw value and return filtered result
  virtual float update(float raw_value) = 0;

  // Reset filter to initial state
  virtual void reset() = 0;

  // Get human-readable filter name
  virtual std::string getName() const = 0;

  // Get internal variance (for diagnostics)
  virtual float getVariance() const { return 0.0f; }

  // Get slope or rate of change (for diagnostics)
  virtual float getSlope() const { return 0.0f; }
};

#endif  // SRC_FILTERS_FILTER_BASE_HPP_
// EOF
