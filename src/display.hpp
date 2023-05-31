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
#ifndef SRC_DISPLAY_HPP_
#define SRC_DISPLAY_HPP_

#include <SH1106Wire.h>
// #include <SSD1306Wire.h>
#include <LiquidCrystal_I2C.h>

#include <kegconfig.hpp>
#include <main.hpp>

enum FontSize {  // Font options
  FONT_1 = 1,    // Support LCD
  FONT_10 = 10,  // Support OLED 6 lines
  FONT_16 = 16,  // Support OLED 5 lines
  FONT_24 = 24   // Support OLED 3 lines
};

class Display {
 private:
  SH1106Wire* _displayOLED[2] = {0, 0};
  // SSD1306Wire* _displayOLED2[2] = {0, 0};
  LiquidCrystal_I2C* _displayLCD[2] = {0, 0};

  int _width[2] = {0, 0};
  int _height[2] = {0, 0};
  FontSize _fontSize[2] = {FontSize::FONT_10, FontSize::FONT_10};
  DisplayDriverType _driver = DisplayDriverType::OLED_1306;

  bool checkInitialized(UnitIndex idx);

 public:
  Display();
  void setup();
  void clear(UnitIndex idx);
  void show(UnitIndex idx);
  void setFont(UnitIndex idx, FontSize fs);
  int getFontHeight(UnitIndex idx) { return _fontSize[idx]; }
  int getTextWidth(UnitIndex idx, const String& text);

  int getDisplayWidth(UnitIndex idx) { return _width[idx]; }
  int getDisplayHeight(UnitIndex idx) { return _height[idx]; }

  void printPosition(UnitIndex index, int x, int y, const String& text);
  void printLine(UnitIndex index, int l, const String& text);
  void printLineCentered(UnitIndex index, int l, const String& text);

  void drawRect(UnitIndex idx, int x, int y, int w, int h);
  void fillRect(UnitIndex idx, int x, int y, int w, int h);

  void drawProgressBar(UnitIndex idx, int y, float percentage);
};

extern Display myDisplay;

#endif  // SRC_DISPLAY_HPP_

// EOF
