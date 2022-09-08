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

// #define DRIVER_1106
#define DRIVER_1306

#if defined(DRIVER_1106)
#include <SH1106Wire.h>
#elif defined(DRIVER_1306)
#include <SSD1306Wire.h>
#endif

#include <main.hpp>

enum FontSize {  // OLED Size - 128x64
  FONT_10 = 10,  // Support 6 lines
  FONT_16 = 16,  // Support 5 lines
  FONT_24 = 24   // Support 3 lines
};

class Display {
 private:
#if defined(DRIVER_1106)
  SH1106Wire* _display[2] = {0, 0};
#elif defined(DRIVER_1306)
  SSD1306Wire* _display[2] = {0, 0};
#endif

  int _width[2] = {128, 128};
  int _height[2] = {64, 64};
  FontSize _fontSize[2] = {FontSize::FONT_10, FontSize::FONT_10};

  void scanI2C();

 public:
  Display();
  void setup(UnitIndex idx);
  void clear(UnitIndex idx) { _display[idx]->clear(); }
  void show(UnitIndex idx) { _display[idx]->display(); }
  void setFont(UnitIndex idx, FontSize fs);
  int getTextWidth(UnitIndex idx, const String& text);

  int getWidth(UnitIndex idx) { return _width[idx]; }
  int getHeight(UnitIndex idx) { return _height[idx]; }

  void printPosition(UnitIndex index, int x, int y, const String& text);
  void printLine(UnitIndex index, int l, const String& text);
  void printLineCentered(UnitIndex index, int l, const String& text);

  void drawRect(UnitIndex idx, int x, int y, int w, int h) {
    _display[idx]->drawRect(x, y, w, h);
  }
  int getCurrentFontSize(UnitIndex idx) { return _fontSize[idx]; }
};

extern Display myDisplay;

#endif  // SRC_DISPLAY_HPP_

// EOF
