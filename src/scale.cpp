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
#include <config.hpp>
#include <scale.hpp>

Scale myScale;

Scale::Scale() {}

void Scale::setup(bool force) {
  if (!_scale[0] || force) {
    if (_scale[0]) delete _scale[0];

    Log.verbose(F("Scal: Initializing scale [0], using offset %l." CR),
                myConfig.getScaleOffset(0));
    _scale[0] = new HX711();
    _scale[0]->begin(PIN_SCALE1_SDA, PIN_SCALE1_SCL);
    _scale[0]->set_offset(myConfig.getScaleOffset(UnitIndex::UNIT_1));

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

    Log.verbose(F("Scal: Initializing scale [1], using offset %l." CR),
                myConfig.getScaleOffset(1));
    _scale[1] = new HX711();
    _scale[1]->begin(PIN_SCALE2_SDA, PIN_SCALE2_SCL);
    _scale[1]->set_offset(myConfig.getScaleOffset(UnitIndex::UNIT_2));

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
  setScaleFactor(UnitIndex::UNIT_1);
  setScaleFactor(UnitIndex::UNIT_2);
}

void Scale::setScaleFactor(UnitIndex idx) {
  if (!_scale[idx]) return;

  float fs = myConfig.getScaleFactor(idx);

  if (fs == 0.0) fs = 1.0;

  _scale[idx]->set_scale(
      fs);  // apply the saved scale factor so we get valid results
}

float Scale::getValue(UnitIndex idx) {
  if (!_scale[idx]) return 0;

  float f = _scale[idx]->get_units(_readCount);
  Log.verbose(F("Scal: Reading weight=%F [%d]." CR), f, idx);
  _lastValue[idx] = f;  // cache the last read value, will be used by API's and
                        // updated in loop()
  return f;
}

void Scale::tare(UnitIndex idx) {
  if (!_scale[idx]) return;

  Log.verbose(F("Scal: Set scale to zero, prepare for calibration [%d]." CR),
              idx);

  _scale[idx]->set_scale();  // set scale factor to 1
  _scale[idx]->tare();       // zero weight
  int32_t l = _scale[idx]->get_offset();
  Log.verbose(F("Scal: New scale offset found %l [%d]." CR), l, idx);

  myConfig.setScaleOffset(idx, l);  // save the offset
  myConfig.saveFile();
}

int32_t Scale::getRawValue(UnitIndex idx) {
  if (!_scale[idx]) return 0;

  int32_t l = _scale[idx]->read_average(
      _readCount);  // get the raw value without applying scaling factor
  return l;
}

void Scale::findFactor(UnitIndex idx, float weight) {
  if (!_scale[idx]) return;

  float l = getValue(idx);  // get value with applied scaling factor
  float f = l / weight;
  Log.verbose(F("Scal: Detecting factor for weight %F, raw %l %F [%d]." CR),
              weight, l, f, idx);

  myConfig.setScaleFactor(idx, f);
  myConfig.saveFile();  // save the factor to file

  setScaleFactor(idx);  // apply the factor after it has been saved
  getValue(idx);  // read the value again to update the cached value based on
                  // new factor
}

int Scale::calculateNoPints(UnitIndex idx, float weight) {
  if (!_scale[idx]) return 0;

  float p = myConfig.getPintWeight(idx);

  if (p == 0.0) p = 1;

  int pints = (weight - myConfig.getKegWeight(idx)) / p;
  return pints < 0 ? 0 : pints;
}

// EOF
