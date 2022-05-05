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
#include <scale.hpp>
#include <config.hpp>

Scale myScale;

Scale::Scale() {
}

void Scale::setup() {
  Log.verbose(F("Scal: Initializing scale, using offset %l." CR), myConfig.getScaleOffset(0));
  _scale[0] = new HX711();
  _scale[0]->begin(PIN_SCALE_SDA, PIN_SCALE_SCL);
  _scale[0]->set_offset(myConfig.getScaleOffset(UnitIndex::UNIT_1));

  // TODO: Add init code for second scale
}

float Scale::getValue(UnitIndex idx) {
  if (!_scale[idx])
    return 0;

  float fs = myConfig.getScaleFactor(idx);

  if (fs == 0.0) 
    fs = 1.0;

  _scale[idx]->set_scale(fs);

  float f = _scale[idx]->get_units();
  Log.verbose(F("Scal: Reading weight=%F (scale factor=%F) [%d]." CR), f, fs, idx);
  return reduceFloatPrecision(f, myConfig.getWeightPrecision());
}

void Scale::tare(UnitIndex idx) {
  if (!_scale[idx])
    return;

  Log.verbose(F("Scal: Set scale to zero, prepare for calibration [%d]." CR), idx);

  _scale[idx]->set_scale();
  _scale[idx]->tare();

  long l = _scale[idx]->read_average();
  
  Log.verbose(F("Scal: New scale offset found %l [%d]." CR), l, idx);
  _scale[idx]->set_offset(l);  

  myConfig.setScaleOffset(idx, l);
  myConfig.saveFile();
}

long Scale::getRawValue(UnitIndex idx) {
  if (!_scale[idx])
    return 0;

  return _scale[idx]->get_value(_readCount);
}

void Scale::findFactor(UnitIndex idx, float weight) {
  if (!_scale[idx])
    return;

  long l = getRawValue(idx);

  float f = l/weight;
  Log.verbose(F("Scal: Detecting factor for weight %F, raw %l %F [%d]." CR), weight, l, f, idx);

  myConfig.setScaleFactor(idx, f);
  myConfig.saveFile();
}

int Scale::calculateNoPints(UnitIndex idx) {
  if (!_scale[idx])
    return 0;

  float v = getValue(idx);
  float p = myConfig.getPintWeight(idx);

  if (p == 0.0) 
    p = 1;

  int pints = (v - myConfig.getKegWeight(idx)) / p;
  return pints<0 ? 0 : pints;
}

// EOF
