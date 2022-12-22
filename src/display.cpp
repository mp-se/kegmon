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
#include <Wire.h>

#include <display.hpp>
#include <kegconfig.hpp>

#define DISPLAY_ADR1 0x3c
#define DISPLAY_ADR2 0x3d

Display::Display() {
  scanI2C();
#if defined(DRIVER_1106)
  _display[0] = new SH1106Wire(DISPLAY_ADR1, PIN_OLED_SDA, PIN_OLED_SCL);
  _display[1] = new SH1106Wire(DISPLAY_ADR2, PIN_OLED_SDA, PIN_OLED_SCL);
#elif defined(DRIVER_1306)
  _display[0] = new SSD1306Wire(DISPLAY_ADR1, PIN_OLED_SDA, PIN_OLED_SCL);
  _display[1] = new SSD1306Wire(DISPLAY_ADR2, PIN_OLED_SDA, PIN_OLED_SCL);
#endif
}

void Display::scanI2C() {
  /*
  #if LOG_LEVEL == 6
    Wire.begin();

    byte error, address;
    int n = 0;

    Log.verbose(F("Scanning I2C bus for devices: "));

    for (address = 1; address < 127; address++) {
      // The i2c_scanner uses the return value of
      // the Write.endTransmisstion to see if
      // a device did acknowledge to the address.

      Wire.beginTransmission(address);
      error = Wire.endTransmission();

      if (error == 0) {
        EspSerial.print(address, HEX);
        EspSerial.print("\t");
        n++;
      }
    }
    EspSerial.print("\n");
  #endif
  */
}

void Display::setup(UnitIndex idx) {
#if LOG_LEVEL == 6
  // Log.verbose(F("Disp: Setting up OLED display [%d]." CR), idx);
#endif

  _display[idx]->init();
  _display[idx]->displayOn();
  _display[idx]->flipScreenVertically();

  _height[idx] = _display[idx]->height();
  _width[idx] = _display[idx]->width();

  clear(idx);

  if (idx == 0) {  // Show on primary screen
    setFont(idx, FontSize::FONT_16);
    printLineCentered(idx, 0, CFG_APPNAME);
    printLineCentered(idx, 2, "Loading");
  }

  show(idx);
}

void Display::setFont(UnitIndex idx, FontSize fs) {
  if (!_display[idx]) return;

#if LOG_LEVEL == 6
    // Log.verbose(F("Disp: Setting font size %d [%d]." CR), fs, idx);
#endif
  _fontSize[idx] = fs;

  switch (fs) {
    case FontSize::FONT_10:
      _display[idx]->setFont(ArialMT_Plain_10);
      return;

    case FontSize::FONT_16:
      _display[idx]->setFont(ArialMT_Plain_16);
      return;

    case FontSize::FONT_24:
      _display[idx]->setFont(ArialMT_Plain_24);
      return;
  }
}

int Display::getTextWidth(UnitIndex idx, const String& text) {
  if (!_display[idx]) return -1;

  int w = _display[idx]->getStringWidth(text);
  return w;
}

void Display::printPosition(UnitIndex idx, int x, int y, const String& text) {
  if (!_display[idx]) return;

  if (x < 0) {
    int w = getTextWidth(idx, text);
    x = (_width[idx] - w) / 2;
  }

  _display[idx]->drawString(x, y, text);
}

void Display::printLine(UnitIndex idx, int l, const String& text) {
  if (!_display[idx]) return;

  printPosition(idx, 0, _fontSize[idx] * l, text);
}

void Display::printLineCentered(UnitIndex idx, int l, const String& text) {
  if (!_display[idx]) return;

  int w = getTextWidth(idx, text);
  printPosition(idx, (_width[idx] - w) / 2, _fontSize[idx] * l, text);
}

// EOF
