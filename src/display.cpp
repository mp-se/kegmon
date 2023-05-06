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

#if defined(DRIVER_LCD)
// Custom characters for the LCD progress bar
byte START_DIV_0_OF_1[8] = {B01111, B11000, B10000, B10000,
                            B10000, B10000, B11000, B01111};  // Char start 0/1

byte START_DIV_1_OF_1[8] = {B01111, B11000, B10011, B10111,
                            B10111, B10011, B11000, B01111};  // Char start 1/1

byte DIV_0_OF_2[8] = {B11111, B00000, B00000, B00000,
                      B00000, B00000, B00000, B11111};  // Char middle 0/2

byte DIV_1_OF_2[8] = {B11111, B00000, B11000, B11000,
                      B11000, B11000, B00000, B11111};  // Middle tank 1/2

byte DIV_2_OF_2[8] = {B11111, B00000, B11011, B11011,
                      B11011, B11011, B00000, B11111};  // Middle tank 2/2

byte END_DIV_0_OF_1[8] = {B11110, B00011, B00001, B00001,
                          B00001, B00001, B00011, B11110};  // Char end 0/1

byte END_DIV_1_OF_1[8] = {B11110, B00011, B11001, B11101,
                          B11101, B11001, B00011, B11110};  // Char thin 1/1
#endif

Display::Display() {
#if defined(DRIVER_1106)
  _display[0] = new SH1106Wire(DISPLAY_ADR1, PIN_OLED_SDA, PIN_OLED_SCL);
  _display[1] = new SH1106Wire(DISPLAY_ADR2, PIN_OLED_SDA, PIN_OLED_SCL);
#elif defined(DRIVER_1306)
  _display[0] = new SSD1306Wire(DISPLAY_ADR1, PIN_OLED_SDA, PIN_OLED_SCL);
  _display[1] = new SSD1306Wire(DISPLAY_ADR2, PIN_OLED_SDA, PIN_OLED_SCL);
#elif defined(DRIVER_LCD)
  _display[0] = new LiquidCrystal_I2C(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11,
                                      12, 13, 14, POSITIVE);
  _display[1] = new LiquidCrystal_I2C(PCF8574_ADDR_A21_A11_A00, 4, 5, 6, 16, 11,
                                      12, 13, 14, POSITIVE);
#endif
}

void Display::setup(UnitIndex idx) {
#if LOG_LEVEL == 6
  // Log.verbose(F("Disp: Setting up OLED display [%d]." CR), idx);
#endif

#if defined(DRIVER_LCD)
  _display[idx]->begin(_width[idx], _height[idx]);

  _display[0]->createChar(0, START_DIV_0_OF_1);
  _display[0]->createChar(1, START_DIV_1_OF_1);
  _display[0]->createChar(2, DIV_0_OF_2);
  _display[0]->createChar(3, DIV_1_OF_2);
  _display[0]->createChar(4, DIV_2_OF_2);
  _display[0]->createChar(5, END_DIV_0_OF_1);
  _display[0]->createChar(6, END_DIV_1_OF_1);

  _display[1]->createChar(0, START_DIV_0_OF_1);
  _display[1]->createChar(1, START_DIV_1_OF_1);
  _display[1]->createChar(2, DIV_0_OF_2);
  _display[1]->createChar(3, DIV_1_OF_2);
  _display[1]->createChar(4, DIV_2_OF_2);
  _display[1]->createChar(5, END_DIV_0_OF_1);
  _display[1]->createChar(6, END_DIV_1_OF_1);
#else
  _display[idx]->init();
  _display[idx]->displayOn();
  _display[idx]->flipScreenVertically();
  _height[idx] = _display[idx]->height();
  _width[idx] = _display[idx]->width();
#endif

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

#if defined(DRIVER_LCD)
  _fontSize[idx] = FontSize::FONT_1;
#else
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
#endif
}

int Display::getTextWidth(UnitIndex idx, const String& text) {
  if (!_display[idx]) return -1;

#if defined(DRIVER_LCD)
  return text.length();
#else
  int w = _display[idx]->getStringWidth(text);
  return w;
#endif
}

void Display::printPosition(UnitIndex idx, int x, int y, const String& text) {
  if (!_display[idx]) return;

  if (x < 0) {
    int w = getTextWidth(idx, text);
    x = (_width[idx] - w) / 2;
  }

#if defined(DRIVER_LCD)
  _display[idx]->setCursor(x, y);
  _display[idx]->print(text);
#else
  _display[idx]->drawString(x, y, text);
#endif
}

void Display::printLine(UnitIndex idx, int l, const String& text) {
  if (!_display[idx]) return;

  printPosition(idx, 0, _fontSize[idx] * l, text);
}

void Display::printLineCentered(UnitIndex idx, int l, const String& text) {
  if (!_display[idx]) return;

  int w = getTextWidth(idx, text);

#if defined(DRIVER_LCD)
  printPosition(idx, (_width[idx] - w) / 2, l, text);
#else
  printPosition(idx, (_width[idx] - w) / 2, _fontSize[idx] * l, text);
#endif
}

void Display::clear(UnitIndex idx) { _display[idx]->clear(); }

void Display::show(UnitIndex idx) {
#if !defined(DRIVER_LCD)
  _display[idx]->display();
#endif
}

void Display::drawRect(UnitIndex idx, int x, int y, int w, int h) {
#if !defined(DRIVER_LCD)
  _display[idx]->drawRect(x, y, w, h);
#endif
}

void Display::fillRect(UnitIndex idx, int x, int y, int w, int h) {
#if !defined(DRIVER_LCD)
  _display[idx]->fillRect(x, y, w, h);
#endif
}

void Display::drawProgressBar(UnitIndex idx, int y, float percentage) {
#if defined(DRIVER_LCD)
  _display[idx]->setCursor(0, y);

  // Each character displays 2 vertical bars, but the first and last character
  // displays only one. Map range (0 ~ 100) to range (0 ~ LCD_NB_COLUMNS * 2 -
  // 2)
  int col = map(percentage, 0, 100, 0, _width[idx] * 2 - 2);

  // Print the progress bar
  for (int i = 0; i < _width[idx]; ++i) {
    if (i == 0) {
      // Char 0 = empty start, Char 1 = full start
      _display[idx]->write(col == 0 ? 0 : 1);
      col -= 1;  // First item only have one halv bar
    } else if (i == (_width[idx] - 1)) {
      // Char 5 = full end, Char 6 = empty end
      _display[idx]->write(col > 0 ? 6 : 5);
    } else {
      if (col <= 0) {
        // Char 2 = empty middle
        _display[idx]->write(2);
      } else {
        // Char 3 = half middle, Char 4 = full middle
        _display[idx]->write(col >= 2 ? 4 : 3);
        col -= 2;  // One char equals to 1-2 indicators.
      }
    }
  }
#else
  int w = map(percentage, 0, 100, 0, _width[idx]);

  _display[idx]->drawRect(0, y + 1, _width[idx], _fontSize[idx] - 2);
  _display[idx]->fillRect(0, y + 1, w, _fontSize[idx] - 2);
#endif
}

// EOF
