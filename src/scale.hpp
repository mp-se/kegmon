/*
MIT License

Copyright (c) 2021-22 Magnus

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
#ifndef SRC_SCALE_HPP_
#define SRC_SCALE_HPP_

#include <main.hpp>
#include <HX711.h>

class Scale {
 private:
  HX711 *_scale[2] = { 0, 0 };
  int _readCount = 10;

 public:
  Scale();
  void setup(bool force = false);
  void tare(UnitIndex idx);
  float getValue(UnitIndex idx);
  long getRawValue(UnitIndex idx);
  void findFactor(UnitIndex idx, float weight);
  int calculateNoPints(UnitIndex idx);
  bool isConnected(UnitIndex idx) { return _scale[idx] != 0 ? true : false; };
};

extern Scale myScale;

#endif  // SRC_SCALE_HPP_

// EOF
