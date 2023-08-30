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

Display::Display() {}

bool Display::checkInitialized(UnitIndex idx) {
  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      if (!_displayOLED[idx]) return false;
      break;
    case DisplayDriverType::LCD:
      if (!_displayLCD[idx]) return false;
      break;
  }

  return true;
}

void Display::setup() {
  _driver = myConfig.getDisplayDriverType();

  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      Log.notice(F("DISP: Using display driver for OLED 0.96\"" CR));

      _displayOLED[0] =
          new SH1106Wire(DISPLAY_ADR1, myConfig.getPinDisplayData(),
                         myConfig.getPinDisplayClock());
      _displayOLED[1] =
          new SH1106Wire(DISPLAY_ADR2, myConfig.getPinDisplayData(),
                         myConfig.getPinDisplayClock());
      _width[0] = 127;
      _width[1] = 127;
      _height[0] = 63;
      _height[1] = 63;
      _fontSize[0] = FontSize::FONT_10;
      _fontSize[1] = FontSize::FONT_10;

      _displayOLED[0]->init();
      _displayOLED[1]->init();
      _displayOLED[0]->displayOn();
      _displayOLED[1]->displayOn();
      _displayOLED[0]->flipScreenVertically();
      _displayOLED[1]->flipScreenVertically();
      break;

    case DisplayDriverType::LCD:
      Log.notice(F("DISP: Using display driver for LED 20x4" CR));

      Wire.begin(myConfig.getPinDisplayData(), myConfig.getPinDisplayClock());

      _displayLCD[0] = new LiquidCrystal_I2C(PCF8574_ADDR_A21_A11_A01, 4, 5, 6,
                                             16, 11, 12, 13, 14, POSITIVE);
      _displayLCD[1] = new LiquidCrystal_I2C(PCF8574_ADDR_A21_A11_A00, 4, 5, 6,
                                             16, 11, 12, 13, 14, POSITIVE);
      _width[0] = 20;
      _width[1] = 20;
      _height[0] = 4;
      _height[1] = 4;
      _fontSize[0] = FontSize::FONT_1;
      _fontSize[0] = FontSize::FONT_1;

      _displayLCD[0]->begin(_width[0], _height[0]);
      _displayLCD[1]->begin(_width[1], _height[1]);

      _displayLCD[0]->createChar(0, START_DIV_0_OF_1);
      _displayLCD[0]->createChar(1, START_DIV_1_OF_1);
      _displayLCD[0]->createChar(2, DIV_0_OF_2);
      _displayLCD[0]->createChar(3, DIV_1_OF_2);
      _displayLCD[0]->createChar(4, DIV_2_OF_2);
      _displayLCD[0]->createChar(5, END_DIV_0_OF_1);
      _displayLCD[0]->createChar(6, END_DIV_1_OF_1);

      _displayLCD[1]->createChar(0, START_DIV_0_OF_1);
      _displayLCD[1]->createChar(1, START_DIV_1_OF_1);
      _displayLCD[1]->createChar(2, DIV_0_OF_2);
      _displayLCD[1]->createChar(3, DIV_1_OF_2);
      _displayLCD[1]->createChar(4, DIV_2_OF_2);
      _displayLCD[1]->createChar(5, END_DIV_0_OF_1);
      _displayLCD[1]->createChar(6, END_DIV_1_OF_1);
      break;
  }

  clear(UnitIndex::U1);
  clear(UnitIndex::U2);

  setFont(UnitIndex::U1, FontSize::FONT_16);
  printLineCentered(UnitIndex::U1, 0, CFG_APPNAME);
  printLineCentered(UnitIndex::U1, 2, "Loading");

  show(UnitIndex::U1);
  show(UnitIndex::U2);
}

void Display::setFont(UnitIndex idx, FontSize fs) {
  if (!checkInitialized(idx)) return;

  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      _fontSize[idx] = fs;

      switch (fs) {
        case FontSize::FONT_1:
          return;

        case FontSize::FONT_10:
          _displayOLED[idx]->setFont(ArialMT_Plain_10);
          return;

        case FontSize::FONT_16:
          _displayOLED[idx]->setFont(ArialMT_Plain_16);
          return;

        case FontSize::FONT_24:
          _displayOLED[idx]->setFont(ArialMT_Plain_24);
          return;
      }
      break;

    case DisplayDriverType::LCD:
      _fontSize[idx] = FontSize::FONT_1;
      break;
  }
}

int Display::getTextWidth(UnitIndex idx, const String& text) {
  int w = 0;

  if (!checkInitialized(idx)) return -1;

  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      w = _displayOLED[idx]->getStringWidth(text);
      break;

    case DisplayDriverType::LCD:
      w = text.length();
      break;
  }

  return w;
}

void Display::printPosition(UnitIndex idx, int x, int y, const String& text) {
  if (!checkInitialized(idx)) return;

  if (x < 0) {
    int w = getTextWidth(idx, text);
    x = (_width[idx] - w) / 2;
  }

  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      _displayOLED[idx]->drawString(x, y, text);
      break;

    case DisplayDriverType::LCD:
      if (y > (_height[idx] - 1)) return;

      _displayLCD[idx]->setCursor(x, y);
      _displayLCD[idx]->print(text);
      break;
  }
}

void Display::printLine(UnitIndex idx, int l, const String& text) {
  if (!checkInitialized(idx)) return;

  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      printPosition(idx, 0, _fontSize[idx] * l, text);
      break;

    case DisplayDriverType::LCD:
      printPosition(idx, 0, _fontSize[idx] * l, text);
      break;
  }
}

void Display::printLineCentered(UnitIndex idx, int l, const String& text) {
  if (!checkInitialized(idx)) return;

  int w = getTextWidth(idx, text);

  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      printPosition(idx, (_width[idx] - w) / 2, _fontSize[idx] * l, text);
      break;

    case DisplayDriverType::LCD:
      printPosition(idx, (_width[idx] - w) / 2, l, text);
      break;
  }
}

void Display::clear(UnitIndex idx) {
  if (!checkInitialized(idx)) return;

  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      _displayOLED[idx]->clear();
      break;

    case DisplayDriverType::LCD:
      _displayLCD[idx]->clear();
      break;
  }
}

void Display::show(UnitIndex idx) {
  if (!checkInitialized(idx)) return;

  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      _displayOLED[idx]->display();
      break;

    case DisplayDriverType::LCD:
      break;
  }
}

void Display::drawRect(UnitIndex idx, int x, int y, int w, int h) {
  if (!checkInitialized(idx)) return;

  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      _displayOLED[idx]->drawRect(x, y, w, h);
      break;

    case DisplayDriverType::LCD:
      break;
  }
}

void Display::fillRect(UnitIndex idx, int x, int y, int w, int h) {
  if (!checkInitialized(idx)) return;

  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      _displayOLED[idx]->fillRect(x, y, w, h);
      break;

    case DisplayDriverType::LCD:
      break;
  }
}

void Display::drawProgressBar(UnitIndex idx, int y, float percentage) {
  if (!checkInitialized(idx)) return;

  int col = 0;

  switch (_driver) {
    case DisplayDriverType::OLED_1306:
      col = map(percentage, 0, 100, 0, _width[idx]);

      _displayOLED[idx]->drawRect(1, y + 1, _width[idx] - 2,
                                  _fontSize[idx] - 2);
      _displayOLED[idx]->fillRect(1, y + 1, col, _fontSize[idx] - 2);
      break;

    case DisplayDriverType::LCD:
      _displayLCD[idx]->setCursor(0, y);

      // Each character displays 2 vertical bars, but the first and last
      // character displays only one. Map range (0 ~ 100) to range (0 ~
      // LCD_NB_COLUMNS * 2 - 2)
      col = map(percentage, 0, 100, 0, _width[idx] * 2 - 2);

      // Print the progress bar
      for (int i = 0; i < _width[idx]; ++i) {
        if (i == 0) {
          // Char 0 = empty start, Char 1 = full start
          _displayLCD[idx]->write(col == 0 ? 0 : 1);
          col -= 1;  // First item only have one halv bar
        } else if (i == (_width[idx] - 1)) {
          // Char 5 = full end, Char 6 = empty end
          _displayLCD[idx]->write(col > 0 ? 6 : 5);
        } else {
          if (col <= 0) {
            // Char 2 = empty middle
            _displayLCD[idx]->write(2);
          } else {
            // Char 3 = half middle, Char 4 = full middle
            _displayLCD[idx]->write(col >= 2 ? 4 : 3);
            col -= 2;  // One char equals to 1-2 indicators.
          }
        }
      }
      break;
  }
}

// EOF
