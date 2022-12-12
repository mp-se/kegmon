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
#include <levelkalman.hpp>

void KalmanLevelDetection::checkForMaxDeviation(float ref) {
  // Not really valid for the kalman filter
  /*
  if (cnt() > 0) {
    float delta = abs(ave() - ref);

    if (delta > myConfig.getScaleMaxDeviationValue()) {
      Log.notice(F("LKAL: Average statistics deviates too much from raw values "
                   "%F, restarting stable level detection [%d]." CR),
                 delta, _idx);
      clear();
    }
  }*/
}

void KalmanLevelDetection::checkForLevelChange(float ref) {
  // Check if the level has changed up or down. If its down we record the delta
  // as the latest pour.
  /*
  if (cnt() > myConfig.getScaleStableCount() && hasStableValue()) {
    if ((_stable + myConfig.getScaleMaxDeviationValue()) < ave()) {
      Log.notice(
          F("LKAL: Level has increased, adjusting from %F to %F [%d]." CR),
          _stable, ave(), _idx);
      _stable = ave();

      _newStable = true;
    } else if ((_stable - myConfig.getScaleMaxDeviationValue()) > ave()) {
      Log.notice(
          F("LKAL: Level has decreased, adjusting from %F to %F [%d]." CR),
          _stable, ave(), _idx);

      float p = _stable - ave();
      _stable = ave();

      // Check if the keg was removed so we dont register a too large pour
      if ((_stable - myConfig.getKegWeight(_idx)) < 0) {
        _pour -= myConfig.getKegWeight(_idx);
        if (p > 0.0) {
          _pour = p;
          Log.notice(F("LKAL: Keg removed and beer has been poured volume %F "
                       "[%d]." CR),
                     _pour, _idx);
        }
      } else {
        _pour = p;
        Log.notice(F("LKAL: Beer has been poured volume %F [%d]." CR), _pour,
                   _idx);
      }

      // Notify registered endpoints and save to log
      _newPour = true;
    }
  }*/
}

void KalmanLevelDetection::checkForStable(float ref) {
  /*
  if (cnt() > myConfig.getScaleStableCount() && !hasStableValue()) {
    _stable = ave();
    Log.notice(F("LKAL: Found a new stable value %F [%d]." CR),
               getStableValue(), _idx);
    _newStable = true;
  }*/
}

/*
void KalmanLevelDetection::checkForRefDeviation(float ref) {
  // for future reference
}
*/

float KalmanLevelDetection::processValue(float v, float ref) {
  _raw = v;
  if (isnan(_baseline)) {
    _baseline = v;
    return v;
  }
  _value = _filter->updateEstimate(v);

  checkForMaxDeviation(ref);
  checkForStable(ref);
  checkForLevelChange(ref);

#if LOG_DEBUG == 6
  Log.verbose(F("LKAL: Update kalman filter base %F value %F [%d]." CR),
              _baseline, _value, _idx);
#endif
  return _value;
}

// EOF
