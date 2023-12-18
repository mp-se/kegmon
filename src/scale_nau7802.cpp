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

// NOTE! Since the esp8266 only suppors one I2C bus we need a different hardware
// design with a multiplexer to support multiple NAU on that platform. ESP32
// however supports two I2C busses so that is the prefered platform is NAU is to
// be used.

void Scale::setupNAU7802(bool force) {
  if (!_nauScale[0] || force) {
    if (_nauScale[0]) delete _nauScale[0];

#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: NAU7802 initializing scale [0]." CR));
#endif
    _nauScale[0] = new NAU7802();
    _nauScale[0]->begin(Wire);

    if (_nauScale[0]->isConnected()) {
      Log.notice(F("SCAL: NAU7802 scale [0] found." CR));
      _nauScale[0]->setZeroOffset(myConfig.getScaleOffset(UnitIndex::U1));
      _nauScale[0]->setSampleRate(NAU7802_SPS_320);
      // _nauScale[0]->setLDO(NAU7802_LDO_3V3);
      // _nauScale[0]->setGain(NAU7802_GAIN_128);
      // _nauScale[0]->setChannel(NAU7802_CHANNEL_1);
      _nauScale[0]->calibrateAFE();
    } else {
      Log.error(
          F("SCAL: NAU7802 scale [0] not responding, disabling interface." CR));
      delete _nauScale[0];
      _nauScale[0] = 0;
    }
  }

  if (!_nauScale[1] || force) {
    if (_nauScale[1]) delete _nauScale[1];

#if defined(ESP8266)
    Log.error(
        F("SCAL: NAU7802 scale [1] cannot be used on ESP8266 since only one "
          "I2C bus is supported in Arduino. Use an ESP32S2/S3 instead." CR));
#else
#if LOG_LEVEL == 6
    Log.verbose(F("SCAL: NAU7802 initializing [1], using offset %l." CR),
                myConfig.getScaleOffset(UnitIndex::U2));
#endif
    _nauScale[1] = new NAU7802();

    Log.notice(F("SCAL: Initializing I2C bus #2 on pins SDA=%d,SCL=%d" CR),
               myConfig.getPinScale2Data(), myConfig.getPinScale2Clock());
    Wire1.setPins(myConfig.getPinScale2Data(), myConfig.getPinScale2Clock());
    Wire1.begin();
    _nauScale[1]->begin(Wire1);

    if (_nauScale[1]->isConnected()) {
      Log.notice(F("SCAL: NAU7802 scale [1] found." CR));
      _nauScale[1]->setZeroOffset(myConfig.getScaleOffset(UnitIndex::U2));
      _nauScale[1]->setSampleRate(NAU7802_SPS_320);
      // _nauScale[1]->setLDO(NAU7802_LDO_3V3);
      // _nauScale[1]->setGain(NAU7802_GAIN_128);
      // _nauScale[1]->setChannel(NAU7802_CHANNEL_1);
      _nauScale[1]->calibrateAFE();
    } else {
      Log.error(
          F("SCAL: NAU7802 scale [1] not responding, disabling interface." CR));
      delete _nauScale[1];
      _nauScale[1] = 0;
      Wire1.end();
    }
#endif
  }

  setScaleFactorNAU7802(UnitIndex::U1);
  setScaleFactorNAU7802(UnitIndex::U2);
}

void Scale::setScaleFactorNAU7802(UnitIndex idx) {
  if (!_nauScale[idx]) return;

  float fs = myConfig.getScaleFactor(idx);

  if (fs == 0.0) fs = 1.0;

  _nauScale[idx]->setCalibrationFactor(
      fs);  // apply the saved scale factor so we get valid results
}

float Scale::readNAU7802(UnitIndex idx, bool skipValidation) {
#if defined(DEBUG_LINK_SCALES)
  idx = UnitIndex::U1;
#endif

#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: NAU7802 reading scale for [%d]." CR), idx);
#endif

  if (myConfig.getScaleFactor(idx) == 0 ||
      myConfig.getScaleOffset(idx) == 0) {  // Not initialized, just return zero
    Log.verbose(F("SCAL: NAU7802 scale not initialized [%d]." CR), idx);
    return 0;
  }

  if (!_nauScale[idx]) return 0;

  PERF_BEGIN("scale-read");
  float raw = _nauScale[idx]->getWeight(true);  // default is 8 reads
#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: NAU7802 Reading weight=%F [%d]" CR), raw, idx);
#endif

  if (!skipValidation) {
    // If the value is higher/lower than 100 kb/lbs then the reading is proably
    // wrong, just ignore the reading
    if (raw > 100) {
      Log.error(
          F("SCAL: NAU7802 Ignoring value since it's higher than 100kg, %F "
            "[%d]." CR),
          raw, idx);
      PERF_END("scale-read");
      return NAN;
    }

    if (raw < -100) {
      Log.error(F("SCAL: NAU7802 Ignoring value since it's less than -100kg %F "
                  "[%d]." CR),
                raw, idx);
      PERF_END("scale-read");
      return NAN;
    }
  }

  PERF_END("scale-read");
  return raw;
}

void Scale::tareNAU7802(UnitIndex idx) {
  if (!_nauScale[idx]) return;

  Log.notice(
      F("SCAL: NAU7802 set scale to zero, prepare for calibration [%d]." CR),
      idx);

  _nauScale[idx]->calculateZeroOffset();  // Default is 8 reads
  int32_t l = _nauScale[idx]->getZeroOffset();
  Log.verbose(F("SCAL: NAU7802 New scale offset found %l [%d]." CR), l, idx);
  myConfig.setScaleOffset(idx, l);
  myConfig.saveFile();
}

int32_t Scale::readRawNAU7802(UnitIndex idx) {
#if defined(DEBUG_LINK_SCALES)
  idx = UnitIndex::U1;
#endif
#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: NAU7802 Reading raw scale for [%d]." CR), idx);
#endif
  if (!_nauScale[idx]) return 0;
  PERF_BEGIN("scale-readraw");
  while (!_nauScale[idx]->available()) {
    delay(1);
  }
  int32_t l = _nauScale[idx]->getReading();
  _lastRaw[idx] = l;
#if LOG_LEVEL == 6
  Log.verbose(F("SCAL: NAU7802 Reading scale raw weight=%d [%d]" CR), l, idx);
#endif
  PERF_END("scale-readraw");
  return l;
}

void Scale::findFactorNAU7802(UnitIndex idx, float weight) {
  if (!_nauScale[idx]) return;

  _nauScale[idx]->calculateCalibrationFactor(
      weight, myConfig.getScaleReadCountCalibration());
  float f = _nauScale[idx]->getCalibrationFactor();
  Log.notice(
      F("SCAL: NAU7802 Detecting factor for weight %F, factor %F [%d]." CR),
      weight, f, idx);

  myConfig.setScaleFactor(idx, f);
  myConfig.saveFile();
  readNAU7802(idx, true);
}

// EOF
