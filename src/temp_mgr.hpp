/*
MIT License

Copyright (c) 2021-23 Magnus

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
#ifndef SRC_TEMP_MGR_HPP_
#define SRC_TEMP_MGR_HPP_
#include <memory>
#include <temp_base.hpp>
#include <utils.hpp>

class TempSensorManager {
 private:
  std::unique_ptr<TempSensorBase> _sensor;
  TempReading _last = TEMP_READING_FAILED;

 public:
  TempSensorManager() {}
  ~TempSensorManager();
  TempSensorManager(const TempSensorManager&);
  TempSensorManager& operator=(const TempSensorManager&);
  void setup();
  void reset();
  void read();

  bool hasTemp() { return !isnan(_last.temperature); }
  bool hasHumidity() { return !isnan(_last.humidity); }
  bool hasSensor() { return !_sensor; }

  float getLastTempC() { return _last.temperature; }
  float getLastTempF() {
    return isnan(_last.temperature) ? NAN : convertCtoF(_last.temperature);
  }

  float getLastHumidity() { return _last.humidity; }
};

extern TempSensorManager myTemp;

#endif  // SRC_TEMP_MGR_HPP_

// EOF
