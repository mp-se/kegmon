/*
MIT License

Copyright (c) 2024-2026 Magnus

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

#include <SPI.h>

#include <changedetection.hpp>
#include <kegconfig.hpp>
#include <main.hpp>

#if defined(ENABLE_LVGL)
#include <lvgl.h>
#endif // ENABLE_LVGL

#if defined(ENABLE_TFT)
#include <TFT_eSPI.h>
#include <freertos/semphr.h>
#include <ui_helpers.hpp>
#include <ui_kegmon.hpp>
#endif // ENABLE_TFT

enum FontSize { FONT_9 = 9, FONT_12 = 12, FONT_18 = 18, FONT_24 = 24 };

class Display {
 private:
#if defined(ENABLE_TFT)
  TFT_eSPI* _tft = nullptr;
  uint32_t _backgroundColor = TFT_BLACK;
  // uint16_t _touchCalibrationlData[5] = {0, 0, 0, 0, 0};
  SemaphoreHandle_t _uiSemaphore = nullptr;
#endif
#if defined(ENABLE_LVGL)
  lv_display_t* _display = nullptr;
#endif // ENABLE_LVGL

  FontSize _fontSize = FontSize::FONT_9;
  // Rotation _rotation = ROTATION_90;

 public:
  Display();
  void setup();
  // void calibrateTouch();

  SPIClass& getSPI() {
#if defined(ENABLE_TFT)
    return _tft->getSPIinstance();
#else
    return SPI;
#endif
  }

  void clear(uint32_t color);
  void setFont(FontSize f);
  void printLine(int l, const String& text);
  void printLineCentered(int l, const String& text);
  // Rotation getRotation() { return _rotation; }
  // void setRotation(Rotation rotation);

  void createUI(uint8_t layoutId = 0);

  // Keg/Scale Data API (continuous sensor readings)
  void setScaleData(uint8_t index, float stable_weight, float stable_volume,
                    float last_pour_volume, float temperature, bool connected,
                    const char* state_str);

  // Keg Event API (discrete events from event loop)
  void setScaleEvent(uint8_t index, ChangeDetectionEventType event_type);

  // Keg Hardware Info API
  void setKegInfo(uint8_t index, uint32_t keg_volume);

  // Beer Info API
  void setBeerInfo(uint8_t index, const char* name, float abv, int ebc,
                   int ibu);

  // UI State API
  void setDisplayFormat(const char* volume_unit_format,
                        const char* temp_unit_format);
  void setTheme(bool darkmode);
  void setLayout(uint8_t layout_id);
  uint8_t getLayout();

  // Device Status API
  void setStatus(const char* status);

  // Main loop handler for LVGL thread
  void loop();

  // LVGL methods
  // bool getTouch(uint16_t* x, uint16_t* y);  // Check for touch callback
  // void handleGestureEventEvent(char gesture);
  SemaphoreHandle_t getUISemaphore() { return _uiSemaphore; }
};

// LVGL handlers and utilities
#if defined(ENABLE_TFT)
// void touchScreenHandler(lv_indev_t* indev, lv_indev_data_t* data);
// void gestureScreenHandler(lv_event_t* e);
// void btnLeftEventHandler(lv_event_t* e);
// void btnRightEventHandler(lv_event_t* e);
// void gestureLeft();
// void gestureRight();
#endif
#if defined(ENABLE_LVGL)
void log_print(lv_log_level_t level, const char* buf);
void lvgl_loop_handler(void* parameter);
#endif // ENABLE_LVGL

extern Display myDisplay;

#endif  // SRC_DISPLAY_HPP_

// EOF
