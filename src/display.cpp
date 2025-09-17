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
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <Wire.h>

#include <display.hpp>
#include <fonts.hpp>
#include <functional>
#include <log.hpp>
#include <looptimer.hpp>
#include <main.hpp>

constexpr auto TTF_CALIBRATION_FILENAME = "/tft.dat";

TaskHandle_t lvglTaskHandler;

#if defined(ENABLE_LVGL)
struct LVGL_Data lvglData;
#endif

void Display::setup() {
#if defined(ENABLE_TFT)
  Log.notice(
      F("DISP: TFT Config: MISO=%d, MOSI=%d, SCLK=%d, CS=%d, DC=%d, RST=%d, "
        "TOUCH_CS=%d" CR),
      TFT_MISO, TFT_MOSI, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST, TOUCH_CS);

  _tft = new TFT_eSPI();

  if (!_tft) {
    Log.warning(F("DISP: No TFT_eSPI driver is created!" CR));
    return;
  }

  _tft->init();
  _tft->setRotation(_rotation);
  clear();
  setFont(FontSize::FONT_9);
#else
  Log.warning(F("DISP: TFT driver support is not included in this build!" CR));
#endif
}

void Display::setRotation(Rotation r) {
#if defined(ENABLE_TFT)
  if (!_tft) {
    return;
  }

  _rotation = r;
  _tft->setRotation(_rotation);
#endif
}

void Display::setFont(FontSize f) {
#if defined(ENABLE_TFT)
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
#endif
}

void Display::printLine(int l, const String &text) {
#if defined(ENABLE_TFT)
  if (!_tft) return;

  uint16_t h = _tft->fontHeight();
  _tft->fillRect(0, l * h, _tft->width(), h, _backgroundColor);
  _tft->drawString(text.c_str(), 0, l * h, GFXFF);
#endif
}

void Display::printLineCentered(int l, const String &text) {
#if defined(ENABLE_TFT)
  if (!_tft) return;

  uint16_t h = _tft->fontHeight();
  uint16_t w = _tft->textWidth(text);
  _tft->fillRect(0, l * h, _tft->width(), h, _backgroundColor);
  _tft->drawString(text.c_str(), (_tft->width() - w) / 2, l * h, GFXFF);
#endif
}

void Display::clear(uint32_t color) {
#if defined(ENABLE_TFT)
  if (!_tft) return;

  _backgroundColor = color;
  _tft->fillScreen(_backgroundColor);
  delay(1);
#endif
}

void Display::updateButtons(bool beerEnabled, bool chamberEnabled) {
#if defined(ENABLE_LVGL)
  lvglData._showBeerBtn = beerEnabled;
  lvglData._showChamberBtn = chamberEnabled;
#endif
}

void Display::updateTemperatures(const char *mode, const char *state,
                                 const char *statusBar, float beerTemp,
                                 float chamberTemp, char tempFormat,
                                 bool darkmode) {
#if defined(ENABLE_LVGL)
  if (!_tft) return;

  lvglData._darkmode = darkmode;
  lvglData._tempFormat = tempFormat;
  lvglData._dataMode = mode;
  lvglData._dataState = state;
  lvglData._dataStatusBar = statusBar;

  char s[20];

  // Beer Temp
  if (isnan(beerTemp))
    snprintf(s, sizeof(s), "-- °%c", tempFormat);
  else
    snprintf(s, sizeof(s), "%0.1F°%c", beerTemp, tempFormat);

  lvglData._dataBeerTemp = s;

  // Beer Temp
  if (isnan(chamberTemp))
    snprintf(s, sizeof(s), "-- °%c", tempFormat);
  else
    snprintf(s, sizeof(s), "%0.1F°%c", chamberTemp, tempFormat);

  lvglData._dataChamberTemp = s;
#endif
}

void Display::calibrateTouch() {
#if defined(ENABLE_LVGL)
  if (!_tft) return;

  uint16_t x, y, pressed, i = 0;

  myDisplay.printLineCentered(4, "Press screen to calibrate");

  do {
    delay(300);
    pressed = _tft->getTouch(&x, &y, 600);
    // Log.info(F("DISP: Screen touched %d." CR), pressed);
  } while (!pressed && ++i < 10);

  if (pressed) {
    clear(TFT_GREEN);
    myDisplay.printLineCentered(4, "Touch detected");
    Log.info(F("DISP: Touch screen pressed, force calibration." CR));
    delay(3000);
  }

  File file = LittleFS.open(TTF_CALIBRATION_FILENAME, "r");

  if (file) {
    Log.info(F("DISP: Loading touch calibration data from file." CR));
    file.read(reinterpret_cast<uint8_t *>(&this->_touchCalibrationlData),
              sizeof(_touchCalibrationlData));
    file.close();
  } else {
    _touchCalibrationlData[0] = 0;
  }

  if (pressed || (_touchCalibrationlData[0] == 0)) {
    Log.info(F("DISP: Running calibration sequence." CR));

    clear();
    myDisplay.printLineCentered(4, "Calibration started");
    _tft->calibrateTouch(_touchCalibrationlData, TFT_GREEN, TFT_BLACK, 15);

    file = LittleFS.open(TTF_CALIBRATION_FILENAME, "w");

    if (file) {
      file.write(reinterpret_cast<uint8_t *>(&this->_touchCalibrationlData),
                 sizeof(_touchCalibrationlData));
      file.close();
    } else {
      Log.warning(F("DISP: Failed to write calibration data to file." CR));
    }

    myDisplay.printLineCentered(4, "Touch calibration completed");
    delay(3000);
  }

  myDisplay.printLineCentered(4, "");
#endif
}

bool Display::getTouch(uint16_t *x, uint16_t *y) {
#if defined(ENABLE_TFT)
  uint16_t xt, yt;
  uint8_t b = _tft->getTouch(&xt, &yt);

  if (b) {
    if (xt < 0) xt = 0;
    if (yt < 0) yt = 0;

    if (_rotation == Rotation::ROTATION_90) {
      *x = yt;
      *y = TFT_HEIGHT - xt;
    } else {  // Rotation::ROTATION_270
      *x = yt;
      *y = TFT_HEIGHT - xt;
    }
  }

  return b;
#else
  return false;
#endif
}

void Display::createUI() {
#if defined(ENABLE_LVGL)
  if (!_tft) return;

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
  }

  lvglData._display =
      lv_tft_espi_create(TFT_WIDTH, TFT_HEIGHT, draw_buf, DRAW_BUF_SIZE);

  if (_rotation == Rotation::ROTATION_90) {
    lv_display_set_rotation(lvglData._display, LV_DISPLAY_ROTATION_90);
  } else {  // Rotation::ROTATION_270
    lv_display_set_rotation(lvglData._display, LV_DISPLAY_ROTATION_270);
  }

  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreenHandler);

  // Create components
  lv_style_init(&lvglData._styleLeft);
  lv_style_init(&lvglData._styleCenter);
  lv_style_init(&lvglData._styleStatusBar);
  lv_style_set_text_font(&lvglData._styleLeft, &lv_font_montserrat_18);
  lv_style_set_text_font(&lvglData._styleCenter, &lv_font_montserrat_18);
  lv_style_set_text_font(&lvglData._styleStatusBar, &lv_font_montserrat_12);
  lv_style_set_text_align(&lvglData._styleLeft, LV_TEXT_ALIGN_LEFT);
  lv_style_set_text_align(&lvglData._styleCenter, LV_TEXT_ALIGN_CENTER);
  lv_style_set_text_align(&lvglData._styleStatusBar, LV_TEXT_ALIGN_CENTER);
  // lv_style_set_outline_width(&_styleLeft, 1);
  // lv_style_set_outline_width(&_styleCenter, 1);

  Log.notice(F("DISP: Creating UI components." CR));

  createLabel("Beer", 5, 82, 90, 26, &lvglData._styleLeft);
  createLabel("Chamber", 5, 119, 90, 26, &lvglData._styleLeft);

  lvglData._txtState = createLabel("", 5, 10, 195, 26, &lvglData._styleLeft);
  lvglData._txtMode = createLabel("", 5, 44, 195, 26, &lvglData._styleLeft);
  lvglData._txtBeerTemp =
      createLabel("", 110, 82, 90, 26, &lvglData._styleLeft);
  lvglData._txtChamberTemp =
      createLabel("", 110, 119, 90, 26, &lvglData._styleLeft);
  lvglData._txtTargetTemp =
      createLabel("", 110, 171, 90, 26, &lvglData._styleCenter);
  lvglData._txtStatusBar =
      createLabel("", 5, 214, 305, 16, &lvglData._styleStatusBar);

  lvglData._btnBeer =
      createButton("Beer", 205, 10, 100, 44, btnBeerEventHandler);
  lvglData._btnChamber =
      createButton("Chamber", 205, 60, 100, 44, btnChamberEventHandler);
  lvglData._btnOff = createButton("Off", 205, 110, 100, 44, btnOffEventHandler);
  lvglData._btnUp = createButton("+", 230, 161, 44, 44, btnUpEventHandler);
  lvglData._btnDown = createButton("-", 30, 161, 44, 44, btnDownEventHandler);

  xTaskCreatePinnedToCore(lvgl_loop_handler,  // Function to implement the task
                          "LVGL_Handler",     // Name of the task
                          10000,              // Stack size in words
                          NULL,               // Task input parameter
                          0,                  // Priority of the task
                          &lvglTaskHandler,   // Task handle.
                          0);                 // Core where the task should run
#endif
}

void Display::handleButtonEvent(char btn) {
#if defined(ENABLE_LVGL)
  Log.info(F("DISP: Button pressed, char=%c" CR), btn);

  switch (btn) {
    case 'o':
    case 'b':
    case 'f':
      lvglData._mode = btn;
      setNewControllerMode(lvglData._mode, lvglData._targetTemperature);
      break;

    case '+':
      lvglData._targetTemperature += 0.5;
      break;

    case '-':
      lvglData._targetTemperature -= 0.5;
      break;
  }
#endif
}

// LVGL Wrappers and Handlers
// **************************************************************************************************

#if defined(ENABLE_LVGL)
void log_print(lv_log_level_t level, const char *buf) {
  LV_UNUSED(level);
  Log.notice(F("LVGL: %s." CR), buf);
}

void touchscreenHandler(lv_indev_t *indev, lv_indev_data_t *data) {
  uint16_t x = 0, y = 0;

  if (myDisplay.getTouch(&x, &y)) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;

    // Log.notice(F("LVGL : %d:%d." CR), x, y);
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void btnBeerEventHandler(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    myDisplay.handleButtonEvent('b');
  }
}

void btnChamberEventHandler(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    myDisplay.handleButtonEvent('f');
  }
}

void btnOffEventHandler(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    myDisplay.handleButtonEvent('o');
  }
}

void btnUpEventHandler(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    myDisplay.handleButtonEvent('+');
  }
}

void btnDownEventHandler(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    myDisplay.handleButtonEvent('-');
  }
}

lv_obj_t *createLabel(const char *label, int32_t x, int32_t y, int32_t w,
                      int32_t h, lv_style_t *style) {
  lv_obj_t *lbl = lv_label_create(lv_screen_active());
  lv_label_set_text(lbl, label);
  lv_obj_set_size(lbl, w, h);
  lv_obj_set_pos(lbl, x, y);
  lv_obj_add_style(lbl, style, 0);
  return lbl;
}

void updateLabel(lv_obj_t *obj, const char *label) {
  lv_label_set_text(obj, label);
}

lv_obj_t *createButton(const char *label, int32_t x, int32_t y, int32_t w,
                       int32_t h, lv_event_cb_t handler) {
  lv_obj_t *btn;
  btn = lv_button_create(lv_screen_active());
  lv_obj_set_size(btn, w, h);
  lv_obj_set_pos(btn, x, y);
  lv_obj_add_event_cb(btn, handler, LV_EVENT_ALL, NULL);
  lv_obj_t *lbl = lv_label_create(btn);
  lv_label_set_text(lbl, label);
  lv_obj_center(lbl);
  return btn;
}

void lvgl_loop_handler(void *parameter) {
  LoopTimer taskLoop(500);

  for (;;) {
    if (taskLoop.hasExpired()) {
      taskLoop.reset();

      // Set dark mode if this is enabled in the settings
      lv_obj_t *scr = lv_scr_act();
      lv_color_t color;

      if (lvglData._darkmode) {
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x1F1F1F), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
        color = lv_color_white();
      } else {
        lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
        color = lv_color_black();
      }

      lv_obj_set_style_text_color(lvglData._txtState, color, 0);
      lv_obj_set_style_text_color(lvglData._txtMode, color, 0);
      lv_obj_set_style_text_color(lvglData._txtBeerTemp, color, 0);
      lv_obj_set_style_text_color(lvglData._txtChamberTemp, color, 0);
      lv_obj_set_style_text_color(lvglData._txtTargetTemp, color, 0);
      lv_obj_set_style_text_color(lvglData._txtStatusBar, color, 0);

      // Show/Hide buttons
      if (!lvglData._showBeerBtn)
        lv_obj_add_flag(lvglData._btnBeer, LV_OBJ_FLAG_HIDDEN);
      else
        lv_obj_remove_flag(lvglData._btnBeer, LV_OBJ_FLAG_HIDDEN);

      if (!lvglData._showChamberBtn)
        lv_obj_add_flag(lvglData._btnChamber, LV_OBJ_FLAG_HIDDEN);
      else
        lv_obj_remove_flag(lvglData._btnChamber, LV_OBJ_FLAG_HIDDEN);

      // Update text
      char s[20];
      snprintf(s, sizeof(s), "%0.1F°%c", lvglData._targetTemperature,
               lvglData._tempFormat);
      updateLabel(lvglData._txtTargetTemp, s);

      updateLabel(lvglData._txtState, lvglData._dataState.c_str());
      updateLabel(lvglData._txtMode, lvglData._dataMode.c_str());
      updateLabel(lvglData._txtBeerTemp, lvglData._dataBeerTemp.c_str());
      updateLabel(lvglData._txtChamberTemp, lvglData._dataChamberTemp.c_str());
      updateLabel(lvglData._txtStatusBar, lvglData._dataStatusBar.c_str());
    }

    lv_task_handler();
    lv_tick_inc(5);
    delay(5);
  }
}
#endif

// EOF
