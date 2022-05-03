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
  // Log.verbose(F("Scal: Initializing scale, using offset %l." CR), myConfig.getScaleOffset());
  _scale = new HX711();
  _scale->begin(PIN_SCALE_SDA, PIN_SCALE_SCL);
  _scale->set_offset(myConfig.getScaleOffset());  
}

float Scale::getValue() {
  float fs = myConfig.getScaleFactor();

  if (fs == 0.0) 
    fs = 1.0;

  _scale->set_scale(fs);

  float f = _scale->get_units();
  // Log.verbose(F("Scal: Reading weight=%F (scale factor=%F)." CR), f, fs);
  return reduceFloatPrecision(f, myConfig.getWeightPrecision());
}

void Scale::tare() {
  // Log.verbose(F("Scal: Set scale to zero, prepare for calibration." CR));

  _scale->set_scale();
  _scale->tare();

  long l = _scale->read_average();
  
  // Log.verbose(F("Scal: New scale offset found %l." CR), l);
  _scale->set_offset(l);  

  myConfig.setScaleOffset(l);
  myConfig.setScaleFactor(0);
  myConfig.saveFile();
}

long Scale::getRawValue() {
  return _scale->get_value(_readCount);
}

void Scale::findScale(float weight) {
  long l = getRawValue();

  float f = l/weight;
  // Log.verbose(F("Scal: Detecting factor for weight %F, raw %l %F." CR), weight, l, f);

  myConfig.setScaleFactor(f);
  myConfig.saveFile();
}

// EOF
