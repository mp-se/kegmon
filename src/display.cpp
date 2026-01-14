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
// #include <Touch_CST328.h>

#include <cstdio>
#include <display.hpp>
#include <fonts.hpp>
#include <log.hpp>
#include <looptimer.hpp>
#include <ui_helpers.hpp>

#if defined(ENABLE_TFT)
#include <ui_kegmon.hpp>
TaskHandle_t lvglTaskHandler;
#endif

// constexpr auto TTF_CALIBRATION_FILENAME = "/tft.dat";

Display::Display() { _tft = new TFT_eSPI(); }

void Display::setup() {
  if (!_tft) return;

  _tft->init();
  _tft->setSwapBytes(true);
  _tft->setRotation(1);  // 90 degrees
  clear(TFT_BLACK);
  setFont(FontSize::FONT_9);

#if TOUCH_CS == -1  // Using CST328 touch on waveshare
  if (!Touch_Init()) {
    Log.error(F("DISP: Unable to initialize CST328 touch controller." CR));
  }
#endif
}

void Display::setFont(FontSize f) {
  if (!_tft) return;

  switch (f) {
    default:
    case FontSize::FONT_9:
      _tft->setFreeFont(FF17);
      break;
    case FontSize::FONT_12:
      _tft->setFreeFont(FF18);
      break;
    case FontSize::FONT_18:
      _tft->setFreeFont(FF19);
      break;
    case FontSize::FONT_24:
      _tft->setFreeFont(FF20);
      break;
  }
}

void Display::printLine(int l, const String &text) {
  if (!_tft) return;

  uint16_t h = _tft->fontHeight();
  _tft->fillRect(0, l * h, _tft->width(), h, TFT_BLACK);
  _tft->drawString(text.c_str(), 0, l * h, GFXFF);
}

void Display::printLineCentered(int l, const String &text) {
  if (!_tft) return;

  uint16_t h = _tft->fontHeight();
  uint16_t w = _tft->textWidth(text);
  _tft->fillRect(0, l * h, _tft->width(), h, TFT_BLACK);
  _tft->drawString(text.c_str(), (_tft->width() - w) / 2, l * h, GFXFF);
}

void Display::clear(uint32_t color) {
  if (!_tft) return;

  _backgroundColor = color;
  _tft->fillScreen(_backgroundColor);
  delay(1);
}

void Display::createUI(uint8_t layoutId) {
  if (!_tft) return;

#if defined(ENABLE_LVGL)
  // Create UI semaphore for thread-safe access
  _uiSemaphore = xSemaphoreCreateMutex();
  if (!_uiSemaphore) {
    Log.error(F("DISP: Failed to create UI semaphore." CR));
    return;
  }

  Log.notice(F("DISP: Using LVL v%d.%d.%d." CR), lv_version_major(),
             lv_version_minor(), lv_version_patch());

  lv_init();
  lv_log_register_print_cb(log_print);

#define DRAW_BUF_SIZE (TFT_WIDTH * TFT_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

  void *draw_buf = ps_malloc(DRAW_BUF_SIZE);

  if (!draw_buf) {
    Log.error(
        F("DISP: Failed to allocate ps ram for display buffer, size=%d" CR),
        DRAW_BUF_SIZE);
    return;
  }

  _display = lv_tft_espi_create(TFT_WIDTH, TFT_HEIGHT, draw_buf, DRAW_BUF_SIZE);

  lv_display_set_rotation(_display, LV_DISPLAY_ROTATION_90);

  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // lv_indev_set_read_cb(indev, touchScreenHandler);

  // Register gesture event handler for the main screen
  lv_obj_t *scr = lv_scr_act();
  // lv_obj_add_event_cb(scr, gestureScreenHandler, LV_EVENT_GESTURE, NULL);

  // Initialize the kegmon UI with selected layout
  kegmon_init(_display, false, layoutId);

  xTaskCreatePinnedToCore(lvgl_loop_handler,  // Function to implement the task
                          "LVGL_Handler",     // Name of the task
                          10000,              // Stack size in words
                          NULL,               // Task input parameter
                          0,                  // Priority of the task
                          &lvglTaskHandler,   // Task handle.
                          0);                 // Core where the task should run

#endif // ENABLE_LVGL
}

void Display::setScaleData(uint8_t index, float stable_weight, float stable_volume,
                           float last_pour_volume, float temperature,
                           bool connected, const char *state_str) {
#if defined(ENABLE_LVGL)
  kegmon_set_stable_weight(index, stable_weight);
  kegmon_set_stable_volume(index, stable_volume);
  kegmon_set_last_pour_volume(index, last_pour_volume);
  kegmon_set_temperature(index, temperature);
  kegmon_set_connected(index, connected);
  kegmon_set_state(index, state_str);
#endif // ENABLE_LVGL
}

void Display::setScaleEvent(uint8_t index, ChangeDetectionEventType event_type) {
#if defined(ENABLE_LVGL)
  kegmon_set_event_type(index, event_type);
#endif // ENABLE_LVGL
}

void Display::setKegInfo(uint8_t index, uint32_t keg_volume) {
#if defined(ENABLE_LVGL)
  kegmon_set_keg_volume(index, keg_volume);
#endif // ENABLE_LVGL
}

void Display::setBeerInfo(uint8_t index, const char *name, float abv, int ebc,
                          int ibu) {
#if defined(ENABLE_LVGL)
  kegmon_set_keg_name(index, name);
  kegmon_set_beer_meta(index, abv, ebc, ibu);
#endif // ENABLE_LVGL
}

void Display::setDisplayFormat(const char *volume_unit_format,
                               const char *temp_unit_format) {
#if defined(ENABLE_LVGL)
  kegmon_set_volume_unit_format(volume_unit_format);
  kegmon_set_temp_unit_format(temp_unit_format);
#endif // ENABLE_LVGL
}

void Display::setTheme(bool darkmode) { 
#if defined(ENABLE_LVGL)
  kegmon_set_theme(darkmode); 
#endif // ENABLE_LVGL
}

void Display::setLayout(uint8_t layout_id) {
#if defined(ENABLE_LVGL)
  if (kegmon_get_layout() == layout_id) {
    return;
  }

  if (_uiSemaphore && xSemaphoreTake(_uiSemaphore, pdMS_TO_TICKS(100))) {
    kegmon_set_layout(layout_id);
    xSemaphoreGive(_uiSemaphore);
  } else {
    Log.warning(
        F("DISP: Failed to acquire UI semaphore for layout change." CR));
  }
#endif // ENABLE_LVGL
}

uint8_t Display::getLayout() { 
#if defined(ENABLE_LVGL)
  return kegmon_get_layout();
#else
  return 0;
#endif // ENABLE_LVGL
}

void Display::setStatus(const char *status) {
#if defined(ENABLE_LVGL)
  kegmon_set_device_status(status);
#endif // ENABLE_LVGL
}

void Display::loop() { 
#if defined(ENABLE_LVGL)
  kegmon_loop(); 
#endif // ENABLE_LVGL
}

// void Display::calibrateTouch() {
// #if defined(ENABLE_LVGL) && \
//     TOUCH_CS != -1  // Only needed when using TFT_eSPI touch handler
//   if (!_tft) return;

//   uint16_t x, y, pressed, i = 0;

//   myDisplay.printLineCentered(4, "Press screen to calibrate");

//   do {
//     delay(300);
//     pressed = _tft->getTouch(&x, &y, 600);
//     Log.info(F("DISP: Screen touched %d." CR), pressed);
//   } while (!pressed && ++i < 10);

//   if (pressed) {
//     clear(TFT_GREEN);
//     myDisplay.printLineCentered(4, "Touch detected");
//     Log.info(F("DISP: Touch screen pressed, force calibration." CR));
//     delay(3000);
//   }

//   File file = LittleFS.open(TTF_CALIBRATION_FILENAME, "r");

//   if (file) {
//     Log.info(F("DISP: Loading touch calibration data from file." CR));
//     file.read(reinterpret_cast<uint8_t *>(&this->_touchCalibrationlData),
//               sizeof(_touchCalibrationlData));
//     file.close();
//   } else {
//     _touchCalibrationlData[0] = 0;
//   }

//   if (pressed || (_touchCalibrationlData[0] == 0)) {
//     Log.info(F("DISP: Running calibration sequence." CR));

//     clear(TFT_BLACK);
//     myDisplay.printLineCentered(4, "Calibration started");
//     _tft->calibrateTouch(_touchCalibrationlData, TFT_GREEN, TFT_BLACK, 15);

//     file = LittleFS.open(TTF_CALIBRATION_FILENAME, "w");

//     if (file) {
//       file.write(reinterpret_cast<uint8_t *>(&this->_touchCalibrationlData),
//                  sizeof(_touchCalibrationlData));
//       file.close();
//     } else {
//       Log.warning(F("DISP: Failed to write calibration data to file." CR));
//     }

//     myDisplay.printLineCentered(4, "Touch calibration completed");
//     delay(3000);
//   }

//   myDisplay.printLineCentered(4, "");
// #endif
// }

// bool Display::getTouch(uint16_t *x, uint16_t *y) {
// #if defined(ENABLE_TFT)

// #if TOUCH_CS == -1  // Using CST328 touch on waveshare
//   uint16_t xt[CST328_LCD_TOUCH_MAX_POINTS] = {0};
//   uint16_t yt[CST328_LCD_TOUCH_MAX_POINTS] = {0};
//   uint16_t strength[CST328_LCD_TOUCH_MAX_POINTS] = {0};
//   uint8_t cnt = 0;

//   Touch_Read_Data();
//   uint8_t b =
//       Touch_Get_XY(xt, yt, strength, &cnt, uint8_t
//       CST328_LCD_TOUCH_MAX_POINTS);

//   if (b && cnt > 0) {
//     // if (_rotation == Rotation::ROTATION_90) {
//     //   *x = yt[0];
//     //   *y = TFT_HEIGHT - xt[0];
//     // } else {  // Rotation::ROTATION_270
//     //   *x = yt[0];
//     //   *y = TFT_HEIGHT - xt[0];
//     // }

//     *x = TFT_WIDTH - xt[0];
//     *y = TFT_HEIGHT - yt[0];
//     return true;
//   }
// #else
//   uint16_t xt, yt;
//   uint8_t b = _tft->getTouch(&xt, &yt);

//   if (b) {
//     if (xt < 0) xt = 0;
//     if (yt < 0) yt = 0;

//     // if (_rotation == Rotation::ROTATION_90) {
//     *x = yt;
//     *y = TFT_HEIGHT - xt;
//     // } else {  // Rotation::ROTATION_270
//     //   *x = yt;
//     //   *y = TFT_HEIGHT - xt;
//     // }
//     return true;
//   }
// #endif
//   return false;
// #else
//   return false;
// #endif
// }

// LVGL Wrappers and Handlers
// **************************************************************************************************

#if defined(ENABLE_TFT)
// void touchScreenHandler(lv_indev_t *indev, lv_indev_data_t *data) {
//   uint16_t x = 0, y = 0;

//   if (myDisplay.getTouch(&x, &y)) {
//     data->state = LV_INDEV_STATE_PRESSED;
//     data->point.x = x;
//     data->point.y = y;

//     Log.verbose(F("LVGL: %d:%d." CR), x, y);
//   } else {
//     data->state = LV_INDEV_STATE_RELEASED;
//   }
// }

// void btnLeftEventHandler(lv_event_t *e) {
//   if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
//     gestureLeft();
//   }
// }

// void btnRightEventHandler(lv_event_t *e) {
//   if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
//     gestureRight();
//   }
// }

// void gestureScreenHandler(lv_event_t *e) {
//   lv_event_code_t code = lv_event_get_code(e);
//   if (code == LV_EVENT_GESTURE) {
//     lv_dir_t gesture = lv_indev_get_gesture_dir(lv_indev_get_act());
//     switch (gesture) {
//       case LV_DIR_LEFT:
//         Log.info(F("DISP: Gesture LEFT." CR));
//         gestureLeft();
//         break;
//       case LV_DIR_RIGHT:
//         Log.info(F("DISP: Gesture RIGHT." CR));
//         gestureRight();
//         break;
//       case LV_DIR_TOP:
//         Log.info(F("DISP: Gesture UP." CR));
//         break;
//       case LV_DIR_BOTTOM:
//         Log.info(F("DISP: Gesture DOWN." CR));
//         break;
//       default:
//         break;
//     }
//   }
// }

#if defined(ENABLE_LVGL)
void log_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Log.notice(F("LVGL: %s." CR), buf);
}

void lvgl_loop_handler(void *parameter) {
  LoopTimer taskLoop(500);

  for (;;) {
    // Call the kegmon main loop to update all UI elements
    // Protect with semaphore to prevent crashes during layout changes
    if (myDisplay.getUISemaphore() &&
        xSemaphoreTake(myDisplay.getUISemaphore(), pdMS_TO_TICKS(100))) {
      if (taskLoop.hasExpired()) {
        taskLoop.reset();
        myDisplay.loop();
      }
    }

    lv_task_handler();
    lv_tick_inc(10);
    xSemaphoreGive(myDisplay.getUISemaphore());

    delay(10);
  }
}
#endif // ENABLE_LVGL

#else

Display::Display() {}

void Display::setup() {}

void Display::createUI(uint8_t layoutId) {}

void Display::calibrateTouch() {}

void Display::setFont(FontSize f) {}

void Display::printLine(int l, const String& text) {}

void Display::printLineCentered(int l, const String& text) {}

void Display::clear(uint32_t color) {}

void Display::setScaleData(uint8_t index, float stable_weight, float stable_volume,
                              float last_pour_volume, float temperature,
                              bool connected, const char* state_str) {}

void Display::setScaleEvent(uint8_t index, ChangeDetectionEventType event_type) {}

void Display::setKegInfo(uint8_t index, uint32_t keg_volume) {}

void Display::setBeerInfo(uint8_t index, const char* name, float abv, int ebc,
                          int ibu) {}

void Display::setDisplayFormat(const char* volume_unit_format,
                               const char* temp_unit_format) {}

void Display::setTheme(bool darkmode) {}

void Display::setLayout(uint8_t layout_id) {}

uint8_t Display::getLayout() { return 0; }

void Display::setStatus(const char* status) {}

void Display::loop() {}

#endif  // ENABLE_TFT

// EOF
