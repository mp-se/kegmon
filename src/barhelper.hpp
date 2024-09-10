/*
MIT License

Copyright (c) 2024 Magnus

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
#ifndef SRC_BARHELPER_HPP_
#define SRC_BARHELPER_HPP_

#include <basepush.hpp>
#include <main.hpp>

class Barhelper {
 protected:
  BasePush *_push;

  bool _hasRun = false;
  uint32_t _lastTimestamp = 0;
  bool _lastStatus = false;
  int _lastHttpError = 0;
  String _lastResponse;

  void updateStatus(String &response);

 public:
  explicit Barhelper(BasePush *push) { _push = push; }

  void sendPourInformation(UnitIndex idx, float pourVol);
  void sendKegInformation(UnitIndex idx, float kegVol);

  bool hasRun() { return _hasRun; }
  uint32_t getLastTimeStamp() { return _lastTimestamp; }
  bool getLastStatus() { return _lastStatus; }
  int getLastError() { return _lastHttpError; }
  String getLastResponse() { return _lastResponse; }
};

#endif  // SRC_BARHELPER_HPP_

// EOF
