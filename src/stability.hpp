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
#ifndef SRC_STABILITY_HPP_
#define SRC_STABILITY_HPP_

#include <Statistic.h>

class Stability {
 private:
  statistic::Statistic<float, uint32_t, true> _stability;

  // For viewing the stability of the scale over time. Uses raw / unfiltered
  // values.
 public:
  void clear() { _stability.clear(); }
  void add(float v) { _stability.add(v); }
  float sum() { return _stability.sum(); }
  float min() { return _stability.minimum(); }
  float max() { return _stability.maximum(); }
  float average() { return _stability.average(); }
  float variance() { return _stability.variance(); }
  float popStdev() { return _stability.pop_stdev(); }
  float unbiasedStdev() { return _stability.unbiased_stdev(); }
  uint32_t count() { return _stability.count(); }
};

#endif  // SRC_STABILITY_HPP_

// EOF
