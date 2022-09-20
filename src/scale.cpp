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
#include <scale.hpp>

Scale::Scale() {
  _detection[UnitIndex::U1]._statistic.clear();
  _detection[UnitIndex::U2]._statistic.clear();
  _detection[UnitIndex::U1]._stability.clear();
  _detection[UnitIndex::U2]._stability.clear();
}

void Scale::setup(bool force) {
  if (!_scale[0] || force) {
    if (_scale[0]) delete _scale[0];

#if LOG_LEVEL == 6
      // Log.verbose(F("SCAL: Initializing scale [0], using offset %l." CR),
      //            myConfig.getScaleOffset(0));
#endif
    _scale[0] = new HX711();
    _scale[0]->begin(PIN_SCALE1_SDA, PIN_SCALE1_SCL);
    _scale[0]->set_offset(myConfig.getScaleOffset(UnitIndex::U1));

    if (_scale[0]->wait_ready_timeout(500)) {
      int32_t l = _scale[0]->get_units(1);
      // Log.notice(F("SCAL: Verified connection to scale [0] got %l." CR), l);
    } else {
      Log.error(F("SCAL: Scale [0] not responding, disabling interface." CR));
      delete _scale[0];
      _scale[0] = 0;
    }
  }

  if (!_scale[1] || force) {
    if (_scale[1]) delete _scale[1];

#if LOG_LEVEL == 6
      // Log.verbose(F("SCAL: Initializing scale [1], using offset %l." CR),
      //            myConfig.getScaleOffset(1));
#endif
    _scale[1] = new HX711();
    _scale[1]->begin(PIN_SCALE2_SDA, PIN_SCALE2_SCL);
    _scale[1]->set_offset(myConfig.getScaleOffset(UnitIndex::U2));

    if (_scale[1]->wait_ready_timeout(500)) {
      int32_t l = _scale[1]->get_units(1);
      // Log.notice(F("SCAL: Verified connection to scale [1] got %l." CR), l);
    } else {
      Log.error(F("SCAL: Scale [1] not responding, disabling interface." CR));
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

float LevelDetection::applyKalmanFilter(float weight) {
  if (isnan(_filterBaseline)) {
    _filterBaseline = weight;
    return weight;
  }

  _lastFilterOutput = _filter->updateEstimate(weight);
  return _lastFilterOutput;
}

void Scale::checkMaxDeviation(UnitIndex idx) {
  // Check if the min/max are too far apart, then we have a to wide spread of
  // values and level has changed to much
  if (statsCount(idx) > 0) {
    if ((statsMax(idx) - myConfig.getScaleMaxDeviationValue()) >
            statsAverage(idx) &&
        (statsMin(idx) + myConfig.getScaleMaxDeviationValue()) <
            statsAverage(idx)) {
      Log.notice(
          F("SCAL: Min/Max values deviates to much from average, restarting "
            "statistics %F-%F-%F [%d]." CR),
          statsMin(idx), statsAverage(idx), statsMax(idx), idx);
      /*if (statsCount(idx) > myConfig.getScaleStableCount()) {
        Log.notice(
            F("SCAL: Value was stable, storing as stable level %F [%d]" CR),
            statsAverage(idx), idx);
        _detection[idx]._lastStableWeight = statsAverage(idx);

        myPush.pushKegInformation(idx);
        logLevels(
            isConnected(UnitIndex::U1) ? getLastBeerVolumeLiters(UnitIndex::U1)
                                       : NAN,
            isConnected(UnitIndex::U2) ? getLastBeerVolumeLiters(UnitIndex::U2)
                                       : NAN,
            NAN, NAN);
      }*/
      statsClear(idx);
    }
  }
}

void Scale::checkForLevelChange(UnitIndex idx) {
  // Check if the level has changed up or down. If its down we record the delta
  // as the latest pour.
  if (statsCount(idx) > myConfig.getScaleStableCount() &&
      hasLastStableWeight(idx)) {
    if ((_detection[idx]._lastStableWeight +
         myConfig.getScaleMaxDeviationValue()) < statsAverage(idx)) {
      Log.notice(
          F("SCAL: Level has increased, adjusting from %F to %F [%d]." CR),
          _detection[idx]._lastStableWeight, statsAverage(idx), idx);
      _detection[idx]._lastStableWeight = statsAverage(idx);

      myPush.pushKegInformation(idx);
      logLevels(
          isConnected(UnitIndex::U1) ? getLastBeerVolumeLiters(UnitIndex::U1)
                                     : NAN,
          isConnected(UnitIndex::U2) ? getLastBeerVolumeLiters(UnitIndex::U2)
                                     : NAN,
          NAN, NAN);

    } else if ((_detection[idx]._lastStableWeight -
                myConfig.getScaleMaxDeviationValue()) > statsAverage(idx)) {
      Log.notice(
          F("SCAL: Level has decreased, adjusting from %F to %F [%d]." CR),
          _detection[idx]._lastStableWeight, statsAverage(idx), idx);

      float lastPour = 
          _detection[idx]._lastStableWeight - statsAverage(idx);
      _detection[idx]._lastStableWeight = statsAverage(idx);

      // Check if the keg was removed so we dont register a too large pour
      if ((_detection[idx]._lastStableWeight - myConfig.getKegWeight(idx)) < 0) {
          lastPour -= myConfig.getKegWeight(idx);
          if (lastPour > 0.0) {
            _lastPourWeight[idx] = lastPour;
            Log.notice(F("SCAL: Keg removed and beer has been poured volume %F [%d]." CR),
                      _lastPourWeight[idx], idx);
          }
      } else {
        _lastPourWeight[idx] = lastPour;
        Log.notice(F("SCAL: Beer has been poured volume %F [%d]." CR),
                  _lastPourWeight[idx], idx);
      }


      // Notify registered endpoints and save to log
      myPush.pushPourInformation(idx);
      logLevels(
          isConnected(UnitIndex::U1) ? getLastBeerVolumeLiters(UnitIndex::U1)
                                     : NAN,
          isConnected(UnitIndex::U2) ? getLastBeerVolumeLiters(UnitIndex::U2)
                                     : NAN,
          idx == UnitIndex::U1 ? getPourVolumeLiters(idx) : NAN,
          idx == UnitIndex::U2 ? getPourVolumeLiters(idx) : NAN);

#if defined(ENABLE_INFLUX_DEBUG)
      // This part is used to send data to an influxdb in order to get data on
      // scale stability/drift over time.
      char buf[250];
      float _prevStableWeight = _detection[idx]._lastStableWeight;

      String s;
      snprintf(&buf[0], sizeof(buf),
               "pour,host=%s,device=%s "
               "last-pour-weight%d=%f,last-pour-volume%d=%f,last-stable-weight%"
               "d=%f,stable-weight%d=%f",
               myConfig.getMDNS(), myConfig.getID(), idx + 1,
               _lastPourWeight[idx], idx + 1, getPourVolumeLiters(idx), idx + 1,
               _prevStableWeight, idx + 1, _detection[idx]._lastStableWeight);
      s = &buf[0];

      // Log.verbose(F("Scale: Sending pour data to influx: %s" CR), s.c_str());
      myPush.sendInfluxDb2(s, PUSH_INFLUX_TARGET, PUSH_INFLUX_ORG,
                           PUSH_INFLUX_BUCKET, PUSH_INFLUX_TOKEN);
#endif
    }
  }
}

float Scale::readWeightKg(UnitIndex idx, bool updateStats) {
  if (myConfig.getScaleFactor(idx) == 0 ||
      myConfig.getScaleOffset(idx) == 0)  // Not initialized, just return zero
    return 0;

  if (!_scale[idx]) return 0;

  float f = _scale[idx]->get_units(myConfig.getScaleReadCount());
#if LOG_LEVEL == 6
  // Log.verbose(F("SCAL: Reading weight=%F, updating stats %s [%d]" CR), f,
  //            updateStats ? "true" : "false", idx);
#endif

  f = _detection[idx].applyKalmanFilter(f);

  // If the value is higher/lower than 100 kb/lbs then the reading is proably
  // wrong, just ignore the reading
  if (f > 100) {
    Log.error(
        F("SCAL: Ignoring value since it's higher than 100kg, %F [%d]." CR), f,
        idx);
    return _lastWeight[idx];
  }

  if (f < -100) {
    Log.error(F("SCAL: Ignoring value since it's less than 100kg %F [%d]." CR),
              f, idx);
    return _lastWeight[idx];
  }

#if LOG_LEVEL == 6
  /*Log.verbose(F("SCAL: Stats #%d of #%d, LastLevel %F(delta %F) [%d]." CR),
              statsCount(idx), myConfig.getScaleStableCount(),
              _detection[idx]._lastStableWeight,
              myConfig.getScaleMaxDeviationValue(), idx);*/
#endif

  // If we have enough values and last stable level if its NAN, then update the
  // lastStable value
  if (statsCount(idx) > myConfig.getScaleStableCount() &&
      !hasLastStableWeight(idx)) {
    _detection[idx]._lastStableWeight = statsAverage(idx);
    Log.notice(F("SCAL: Found a new stable value %F [%d]." CR),
               _detection[idx]._lastStableWeight, idx);

    myPush.pushKegInformation(idx);
    logLevels(
        isConnected(UnitIndex::U1) ? getLastBeerVolumeLiters(UnitIndex::U1)
                                   : NAN,
        isConnected(UnitIndex::U2) ? getLastBeerVolumeLiters(UnitIndex::U2)
                                   : NAN,
        NAN, NAN);
  }

  // Update the statistics with the current value
  if (updateStats) {
    _detection[idx]._lastAverageWeight = getAverageWeightKg(idx);
    statsAdd(idx, f);
#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: Adding stats %F (#%d) [%d]." CR), f, statsCount(idx),
                idx);
#endif
  }

  // cache the last read value, will be used by API's and updated in loop()
  _lastWeight[idx] = f;

  // Check for pour and validate if the value is correct (filter out invalid
  // values)
  checkMaxDeviation(idx);
  checkForLevelChange(idx);

  return f;
}

void Scale::tare(UnitIndex idx) {
  if (!_scale[idx]) return;

#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: Set scale to zero, prepare for calibration [%d]." CR),
              idx);
#endif

  _scale[idx]->set_scale();  // set scale factor to 1
  _scale[idx]->tare(myConfig.getScaleReadCountCalibration());  // zero weight
  int32_t l = _scale[idx]->get_offset();
  Log.verbose(F("SCAL: New scale offset found %l [%d]." CR), l, idx);

  myConfig.setScaleOffset(idx, l);  // save the offset
  myConfig.saveFile();
}

int32_t Scale::readRawWeightKg(UnitIndex idx) {
  if (!_scale[idx]) return 0;

  // statsClearAll();

  int32_t l = _scale[idx]->read_average(
      myConfig.getScaleReadCountCalibration());  // get the raw value without
                                                 // applying scaling factor
  return l;
}

void Scale::findFactor(UnitIndex idx, float weight) {
  if (!_scale[idx]) return;

  // statsClearAll();

  float l = _scale[idx]->get_units(myConfig.getScaleReadCountCalibration());
  float f = l / weight;
#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: Detecting factor for weight %F, raw %l %F [%d]." CR),
              weight, l, f, idx);
#endif

  myConfig.setScaleFactor(idx, f);
  myConfig.saveFile();  // save the factor to file

  setScaleFactor(idx);  // apply the factor after it has been saved
  readWeightKg(idx);    // read the value again to update the cached value based
                        // on new factor
}

float Scale::calculateNoGlasses(UnitIndex idx) {
  if (!isConnected(idx)) return 0;

  float weight = getLastBeerWeightKg(idx);
  float glassVol = myConfig.getGlassVolume(idx);
  float fg = myConfig.getBeerFG(idx);

  if (fg == 0.0) fg = 1.0;

  float glassWeight = glassVol * fg;
  float glass = weight / glassWeight;

#if LOG_LEVEL == 6
  /*Log.verbose(F("SCAL: Weight=%F kg, Glass Volume=%F liter, Glass Weight=%F kg
     " "FG=%F, Glasses=%F [%d]." CR),
              weight, glassVol, glassWeight, fg, glass, idx);*/
#endif

  return glass < 0 ? 0 : glass;
}

/*float Scale::getAverageWeightDirectionCoefficient(UnitIndex idx) {
  if (isnan(_detection[idx]._lastAverageWeight) || !statsCount(idx)) return NAN;

  // Loop interval is 2 seconds
  float coeff =
      (statsAverage(idx) - _detection[idx]._lastAverageWeight) / 2 * 100;

#if LOG_LEVEL == 6
  char s[100];
  snprintf(&s[0], sizeof(s), "LastAve=%.6f Ave=%6f, Coeff=%6f",
           _detection[idx]._lastAverageWeight, statsAverage(idx), coeff);
  Log.verbose("SCAL: %s [%d]." CR, &s[0], idx);
#endif

  return coeff;
}*/

void Scale::logLevels(float kegVolume1, float kegVolume2, float pourVolume1,
                      float pourVolume2) {
  struct tm timeinfo;
  time_t now = time(nullptr);
  char s[100];
  gmtime_r(&now, &timeinfo);
  snprintf(&s[0], sizeof(s), "%04d-%02d-%02d %02d:%02d:%02d;%f;%f;%f;%f\n",
           1900 + timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
           kegVolume1 < 0 ? 0 : kegVolume1, kegVolume2 < 0 ? 0 : kegVolume2,
           pourVolume1 < 0 ? 0 : pourVolume1,
           pourVolume2 < 0 ? 0 : pourVolume2);

  Log.notice(F("SCAL: Logging level change %s"), &s[0]);

  File f = LittleFS.open(LEVELS_FILENAME, "a");

  if (f && f.size() > LEVELS_FILEMAXSIZE) {
    f.close();
    bool b1 = LittleFS.remove(LEVELS_FILENAME2);
    bool b2 = LittleFS.rename(LEVELS_FILENAME, LEVELS_FILENAME2);
    f = LittleFS.open(LEVELS_FILENAME, "a");
  }

  if (f) {
    f.write(&s[0], strlen(&s[0]));
    f.close();
  }
}

// EOF
