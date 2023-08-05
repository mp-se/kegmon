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
#ifndef SRC_TEMP_HPP_
#define SRC_TEMP_HPP_

#include <main.hpp>
#include <utils.hpp>
#include "sensor.hpp"

class TempHumidity {
 private:
  std::unique_ptr<Sensor> _sensor;

  tempReading _last = {NAN, NAN};


 public:
  TempHumidity();
  TempHumidity(const TempHumidity&);
  TempHumidity& operator=(const TempHumidity&);
  void setup();
  void reset();
  void read();
  bool hasSensor() { return !!_sensor; }
  float getLastTempC() { return _last.temprature; }
  float getLastTempF() {
    return isnan(_last.temprature) ? NAN : convertCtoF(_last.temprature);
  }
  float getLastHumidity() { return _last.humidity; }
};

extern TempHumidity myTemp;

#endif  // SRC_TEMP_HPP_

// EOF
