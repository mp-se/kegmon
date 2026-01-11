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
#ifndef SRC_FILTERS_FILTER_ALPHABETA_HPP_
#define SRC_FILTERS_FILTER_ALPHABETA_HPP_

#include <filters/filter_base.hpp>

class AlphaBetaFilter : public ScaleFilter {
 private:
  float _alpha;
  float _beta;
  float _x = 0.0f;
  float _v = 0.0f;
  bool _initialized = false;
  float _variance = 0.0f;
  float _slope = 0.0f;

 public:
  explicit AlphaBetaFilter(float alpha = 0.9f, float beta = 0.5f)
      : _alpha(alpha), _beta(beta) {}

  float update(float raw_value) override {
    if (!_initialized) {
      _x = raw_value;
      _v = 0.0f;
      _initialized = true;
      _variance = 0.0f;
      _slope = 0.0f;
      return raw_value;
    }

    float x_pred = _x + _v;
    float v_pred = _v;
    float residual = raw_value - x_pred;

    _x = x_pred + _alpha * residual;
    _v = v_pred + (_beta / 1.0f) * residual;
    _slope = _v;
    _variance = _alpha * residual * residual + (1.0f - _alpha) * _variance;

    return _x;
  }

  void reset() override {
    _initialized = false;
    _x = 0.0f;
    _v = 0.0f;
    _variance = 0.0f;
    _slope = 0.0f;
  }

  std::string getName() const override { return "AlphaBeta"; }
  float getVariance() const override { return _variance; }
  float getSlope() const override { return _slope; }
};

#endif  // SRC_FILTERS_FILTER_ALPHABETA_HPP_
// EOF
