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
#include <basepush.hpp>
#include <kegconfig.hpp>
#include <scale.hpp>

extern BasePush myPush;

Scale::Scale() {
  _statistic[UnitIndex::U1].clear();
  _statistic[UnitIndex::U2].clear();
  _stability[UnitIndex::U1].clear();
  _stability[UnitIndex::U2].clear();
}

void Scale::setup(bool force) {
  if (!_scale[0] || force) {
    if (_scale[0]) delete _scale[0];

#if LOG_LEVEL == 6
    Log.verbose(F("Scal: Initializing scale [0], using offset %l." CR),
                myConfig.getScaleOffset(0));
#endif
    _scale[0] = new HX711();
    _scale[0]->begin(PIN_SCALE1_SDA, PIN_SCALE1_SCL);
    _scale[0]->set_offset(myConfig.getScaleOffset(UnitIndex::U1));

    if (_scale[0]->wait_ready_timeout(500)) {
      int32_t l = _scale[0]->get_units(1);
      Log.notice(F("Scal: Verified connection to scale [0] got %l." CR), l);
    } else {
      Log.error(F("Scal: Scale [0] not responding, disabling interface." CR));
      delete _scale[0];
      _scale[0] = 0;
    }
  }

  if (!_scale[1] || force) {
    if (_scale[1]) delete _scale[1];

#if LOG_LEVEL == 6
    Log.verbose(F("Scal: Initializing scale [1], using offset %l." CR),
                myConfig.getScaleOffset(1));
#endif
    _scale[1] = new HX711();
    _scale[1]->begin(PIN_SCALE2_SDA, PIN_SCALE2_SCL);
    _scale[1]->set_offset(myConfig.getScaleOffset(UnitIndex::U2));

    if (_scale[1]->wait_ready_timeout(500)) {
      int32_t l = _scale[1]->get_units(1);
      Log.notice(F("Scal: Verified connection to scale [1] got %l." CR), l);
    } else {
      Log.error(F("Scal: Scale [1] not responding, disabling interface." CR));
      delete _scale[1];
      _scale[1] = 0;
    }
  }

  // Set the scale factor
  setScaleFactor(UnitIndex::U1);
  setScaleFactor(UnitIndex::U2);
}

void Scale::setScaleFactor(UnitIndex idx) {
  if (!_scale[idx]) return;

  float fs = myConfig.getScaleFactor(idx);

  if (fs == 0.0) fs = 1.0;

  _scale[idx]->set_scale(
      fs);  // apply the saved scale factor so we get valid results
}

float Scale::readWeight(UnitIndex idx, bool updateStats) {
  if (myConfig.getScaleFactor(idx) == 0 ||
      myConfig.getScaleOffset(idx) == 0)  // Not initialized, just return zero
    return 0;

#if defined(ENABLE_SCALE_SIMULATION)
  if (idx == 1) return 0;  // Simulation mode only supports one scale attached

  float f = getNextSimulated();
  Log.verbose(
      F("Scal: Reading simulated weight=%F [%d], updating stats=%s." CR), f,
      idx, updateStats ? "yes" : "no");
#else
  if (!_scale[idx]) return 0;

  float f = _scale[idx]->get_units(myConfig.getScaleReadCount());
#if LOG_LEVEL == 6
  Log.verbose(F("Scal: Reading weight=%F [%d], updating stats %s." CR), f, idx,
              updateStats ? "true" : "false");
#endif

  if (f > 100)  // If the value is more than 100 then the reading is proably
                // wrong, cap the value
    f = 100;

#endif
  // If we have enough values and last stable level if its NAN, then update the
  // lastStable value
  if (statsCount(idx) > myConfig.getScaleStableCount() &&
      !hasLastStableWeight(idx)) {
    _lastStableWeight[idx] = statsAverage(idx);
    Log.notice(F("Scal: Found a new stable value %F [%d]." CR),
               _lastStableWeight[idx], idx);
  }

  // Check if the min/max are too far apart, then we have a to wide spread of
  // values and level has changed to much
  if (statsCount(idx) > 0) {
    if ((statsMax(idx) - statsMin(idx)) >
        myConfig.getScaleMaxDeviationValue()) {
      Log.notice(F("Scal: Min/Max values deviates to much, restarting "
                   "statistics [%d]." CR),
                 idx);
      // Before clearing statistics we record the last average as the stable to
      // get better accuracu for pour detection.
      _lastStableWeight[idx] = statsAverage(idx);
      statsClear(idx);
    }
  }

  // Check if the level has changed up or down. If its down we record the delta
  // as the latest pour.
  if (statsCount(idx) > myConfig.getScaleStableCount() &&
      hasLastStableWeight(idx)) {
    if ((_lastStableWeight[idx] + myConfig.getScaleMaxDeviationValue()) <
        statsAverage(idx)) {
      Log.notice(
          F("Scal: Level has increased, adjusting from %F to %F [%d]." CR),
          _lastStableWeight[idx], statsAverage(idx), idx);
      _lastStableWeight[idx] = statsAverage(idx);
    }

    if ((_lastStableWeight[idx] - myConfig.getScaleMaxDeviationValue()) >
        statsAverage(idx)) {
      Log.notice(
          F("Scal: Level has decreased, adjusting from %F to %F [%d]." CR),
          _lastStableWeight[idx], statsAverage(idx), idx);
      _lastPourWeight[idx] = _lastStableWeight[idx] - statsAverage(idx);
      Log.notice(F("Scal: Beer has been poured volume %F [%d]." CR),
                 _lastPourWeight[idx], idx);
      float _prevStableWeight = _lastStableWeight[idx];
      _lastStableWeight[idx] = statsAverage(idx);

#if defined(ENABLE_INFLUX_DEBUG)
      // This part is used to send data to an influxdb in order to get data on
      // scale stability/drift over time.
      char buf[250];

      String s;
      snprintf(
          &buf[0], sizeof(buf),
          "pour,host=%s,device=%s "
          "last-pour-weight%d=%f,last-stable-weight%d=%f,stable-weight%d=%f",
          myConfig.getMDNS(), myConfig.getID(), idx + 1, _lastPourWeight[idx],
          idx + 1, _prevStableWeight, idx + 1, _lastStableWeight[idx]);
      s = &buf[0];

      Log.verbose(F("Scale: Sending pour data to influx: %s" CR), s.c_str());
      myPush.sendInfluxDb2(s);
#endif
    }
  }

  if (f < 0) {  // We ignore negative values since this can be an faulty reading
    Log.error(F("Scal: Ignoring value since it's less than zero %F [%d]." CR),
              f, idx);
    return _lastWeight[idx];
  }

  // Update the statistics with the current value
  if (updateStats) {
    _lastAverageWeight[idx] = getAverageWeight(idx);
    statsAdd(idx, f);
  }

  _lastWeight[idx] = f;  // cache the last read value, will be used by API's and
                         // updated in loop()
  return f;
}

void Scale::tare(UnitIndex idx) {
  if (!_scale[idx]) return;

#if LOG_LEVEL == 6
  Log.verbose(F("Scal: Set scale to zero, prepare for calibration [%d]." CR),
              idx);
#endif

  _scale[idx]->set_scale();  // set scale factor to 1
  _scale[idx]->tare(myConfig.getScaleReadCountCalibration());  // zero weight
  int32_t l = _scale[idx]->get_offset();
  Log.verbose(F("Scal: New scale offset found %l [%d]." CR), l, idx);

  myConfig.setScaleOffset(idx, l);  // save the offset
  myConfig.saveFile();
}

int32_t Scale::readRawWeight(UnitIndex idx) {
  if (!_scale[idx]) return 0;

  statsClearAll();

  int32_t l = _scale[idx]->read_average(
      myConfig.getScaleReadCountCalibration());  // get the raw value without
                                                 // applying scaling factor
  return l;
}

void Scale::findFactor(UnitIndex idx, float weight) {
  if (!_scale[idx]) return;

  statsClearAll();

  float l = _scale[idx]->get_units(myConfig.getScaleReadCountCalibration());
  float f = l / weight;
#if LOG_LEVEL == 6
  Log.verbose(F("Scal: Detecting factor for weight %F, raw %l %F [%d]." CR),
              weight, l, f, idx);
#endif

  myConfig.setScaleFactor(idx, f);
  myConfig.saveFile();  // save the factor to file

  setScaleFactor(idx);  // apply the factor after it has been saved
  readWeight(idx);  // read the value again to update the cached value based on
                    // new factor
}

float Scale::calculateNoGlasses(UnitIndex idx) {
  if (!isConnected(idx)) return 0;

  float weight = getAverageWeight(idx);
  float glassVol = myConfig.getGlassVolume(idx);
  float fg = myConfig.getBeerFG(idx);

  if (fg == 0.0) fg = 1.0;

  float glassWeight = glassVol * fg;
  float glass = (weight - myConfig.getKegWeight(idx)) / glassWeight;

  Log.notice(F("Scal: Weight=%F kg, Glass Volume=%F liter, Glass Weight=%F kg "
               "FG=%F, Glasses=%F [%d]." CR),
             weight, glassVol, glassWeight, fg, glass, idx);

  return glass < 0 ? 0 : glass;
}

float Scale::getAverageWeightDirectionCoefficient(UnitIndex idx) {
  if (isnan(_lastAverageWeight[idx]) || !statsCount(idx)) return NAN;

  // Loop interval is 2 seconds
  float coeff = (statsAverage(idx) - _lastAverageWeight[idx]) / 2 * 100;

#if LOG_LEVEL == 6
  char s[100];
  snprintf(&s[0], sizeof(s), "LastAve=%.6f Ave=%6f, Coeff=%6f",
           _lastAverageWeight[idx], statsAverage(idx), coeff);
  Log.verbose("Scal: %s [%d]." CR, &s[0], idx);
#endif

  return coeff;
}

void Scale::statsDump(UnitIndex idx) {
  Log.notice(F("Scal: %d: Count=%d, Sum=%F, Min=%F, Max=%F" CR), idx,
             statsCount(idx), statsSum(idx), statsMin(idx), statsMax(idx));
  Log.notice(
      F("Scal: %d: Average=%F, Variance=%F, PopStdev=%F, UnbiasedStdev=%F" CR),
      idx, statsAverage(idx), statsVariance(idx), statsPopStdev(idx),
      statsUnbiasedStdev(idx));
}

// EOF
