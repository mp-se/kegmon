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

#include <main.hpp>
#include <SSD1306Wire.h> 

enum FontSize { // OLED Size - 128x64
  FONT_10 = 10, // Support 6 lines 
  FONT_16 = 16, // Support 5 lines
  FONT_24 = 24  // Support 3 lines
};

class Display {
 private:
  SSD1306Wire *_display;
  int _width = 128;
  int _height = 64;
  FontSize _fontSize = FontSize::FONT_10;

 public:
  Display();
  void setup();
  void clear() { _display->clear(); }
  void show() { _display->display(); }
  void setFont(FontSize fs);
  int  getTextWidth(const String& text);

  void printPosition(int x, int y, const String& text);
  void printLineCentered(int l, const String& text);
};

extern Display myDisplay;

#endif  // SRC_DISPLAY_HPP_

// EOF
