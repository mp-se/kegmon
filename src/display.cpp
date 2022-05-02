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
#include <display.hpp>

Display myDisplay;

Display::Display() {
  _display = new SSD1306Wire(0x3c, PIN_OLED_SDA, PIN_OLED_SCL);
}

void Display::setup() {
  //Log.verbose(F("Disp: Setting up OLED display." CR));

  _display->init();
  _display->displayOn();
  _display->flipScreenVertically();

  clear();
  setFont( FontSize::FONT_16 );
  printLineCentered(1, "Loading...");
  show();
}

void Display::setFont(FontSize fs) {
  //Log.verbose(F("Disp: Setting font size %d." CR), fs);
  _fontSize = fs;

  switch (fs) {
    case FontSize::FONT_10:
      _display->setFont(ArialMT_Plain_10);  
    return;

    case FontSize::FONT_16:
      _display->setFont(ArialMT_Plain_16);  
    return;

    case FontSize::FONT_24:    
      _display->setFont(ArialMT_Plain_24);  
    return;
  }
}

int Display::getTextWidth(const String& text) {
  int w = _display->getStringWidth(text); 
  //Log.verbose(F("Disp: Width of string %s is %d." CR), text.c_str(), w);
  return w;
}

void Display::printPosition(int x, int y, const String& text) {
  //Log.verbose(F("Disp: Printing text %s @ %d,%d." CR), text.c_str(), x, y);
  _display->drawString(x, y, text);
}

void Display::printLineCentered(int l, const String& text) {
  //Log.verbose(F("Disp: Printing text %s @ line %d." CR), text.c_str(), l);
  int w = getTextWidth(text);
  printPosition( (_width-w)/2, _fontSize*l, text );
}

// EOF
