/*
MIT License

Copyright (c) 2024-2025 Magnus

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

#if defined(ENABLE_TFT)
#include <TFT_eSPI.h>
#else
#define TFT_eSPI void
#define TFT_BLACK 0
#endif
#if defined(ENABLE_LVGL)
#include <lvgl.h>
#endif

enum FontSize { FONT_9 = 9, FONT_12 = 12, FONT_18 = 18, FONT_24 = 24 };

#if defined(ENABLE_LVGL)
struct LVGL_Data {
  lv_obj_t* _txtMode;
  lv_obj_t* _txtState;
  lv_obj_t* _txtBeerTemp;
  lv_obj_t* _txtChamberTemp;
  lv_obj_t* _txtTargetTemp;
  lv_obj_t* _txtStatusBar;
  lv_obj_t* _btnBeer;
  lv_obj_t* _btnChamber;
  lv_obj_t* _btnOff;
  lv_obj_t* _btnUp;
  lv_obj_t* _btnDown;
  lv_style_t _styleLeft;
  lv_style_t _styleCenter;
  lv_style_t _styleStatusBar;
  lv_style_t _styleBtnSelected;
  lv_style_t _styleBtnStandard;
  lv_display_t* _display;
  String _dataMode;
  String _dataState;
  String _dataBeerTemp;
  String _dataChamberTemp;
  String _dataStatusBar;
  bool _showBeerBtn;
  bool _showChamberBtn;
  volatile float _targetTemperature;
  volatile char _mode;
  char _tempFormat = 'C';
  bool _darkmode = false;
};

extern struct LVGL_Data lvglData;
#endif

class Display {
 public:
  enum Rotation {
    // ROTATION_0 = 0, // Not supported
    ROTATION_90 = 1,
    // ROTATION_180 = 2,  // Not supported
    ROTATION_270 = 3
  };

 private:
  // Basic TFT variables
  TFT_eSPI* _tft = NULL;
  FontSize _fontSize = FontSize::FONT_9;
  uint16_t _touchCalibrationlData[5] = {0, 0, 0, 0, 0};
  Rotation _rotation = ROTATION_90;
  uint32_t _backgroundColor = TFT_BLACK;

 public:
  Display() {}

  // Basic TFT methods
  void setup();
  void calibrateTouch();
  void clear(uint32_t color = TFT_BLACK);
  void setFont(FontSize f);
  void printLine(int l, const String& text);
  void printLineCentered(int l, const String& text);
  Rotation getRotation() { return _rotation; }
  void setRotation(Rotation rotation);

  // Misc methods
  void setTargetTemperature(float t) { 
#if defined(ENABLE_LVGL)
    lvglData._targetTemperature = t; 
#endif
  }
  void setMode(char m) { 
#if defined(ENABLE_LVGL)
    lvglData._mode = m; 
#endif
  }
  void updateTemperatures(const char* mode, const char* state,
                          const char* statusBar, float beerTemp,
                          float chamberTemp, char tempFormat, bool darkmode);

  void updateButtons(bool beerEnabled, bool chamberEnabled);

  // LVGL methods
  bool getTouch(uint16_t* x, uint16_t* y);  // Check for touch callback
  void handleButtonEvent(char btn);
  void createUI();
};

#if defined(ENABLE_LVGL)
// Wrappers to simplify interaction with LVGL
lv_obj_t* createButton(const char* label, int32_t x, int32_t y, int32_t w,
                       int32_t h, lv_event_cb_t handler);
lv_obj_t* createLabel(const char* label, int32_t x, int32_t y, int32_t w,
                      int32_t h, lv_style_t* style);
void updateLabel(lv_obj_t* obj, const char* label);
void setStyle(lv_obj_t* obj, lv_style_t* style);

void btnBeerEventHandler(lv_event_t* e);
void btnChamberEventHandler(lv_event_t* e);
void btnOffEventHandler(lv_event_t* e);
void btnUpEventHandler(lv_event_t* e);
void btnDownEventHandler(lv_event_t* e);

void touchscreenHandler(lv_indev_t* indev, lv_indev_data_t* data);
void log_print(lv_log_level_t level, const char* buf);
void lvgl_loop_handler(void* parameter);
#endif

extern Display myDisplay;

#endif  // SRC_DISPLAY_HPP_

// EOF
