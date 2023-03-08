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
#include <kegpush.hpp>
#include <perf.hpp>
#include <scale.hpp>

void Scale::loop(UnitIndex idx) {
  if (_sched[idx].tare) {
#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: Tare triggered [%d]." CR), idx);
#endif
    tare(idx);
    readRaw(idx);
    _sched[idx].tare = false;
  }

  if (_sched[idx].findFactor) {
#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: Find factor triggered [%d]." CR), idx);
#endif
    findFactor(idx, _sched[idx].factorWeight);
    readRaw(idx);
    _sched[idx].findFactor = false;
  }
}

// EOF
