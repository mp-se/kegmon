/*
MIT License

Copyright (c) 2025 Magnus

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
#ifndef SRC_UI_HELPERS_HPP_
#define SRC_UI_HELPERS_HPP_

#if defined(ENABLE_LVGL)
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Theme mode enumeration
 */
typedef enum {
  UI_THEME_LIGHT,
  UI_THEME_DARK,
} ui_theme_t;

/**
 * Color theme definition
 */
typedef struct {
  lv_color_t bg;           // Background color
  lv_color_t text;         // Text color
  lv_color_t border;       // Border/accent color
  lv_color_t button_bg;    // Button background color
  lv_color_t button_text;  // Button text color
  // Additional palette entries for panels, status bar and battery states
  lv_color_t panel_bg;        // Card/panel background (e.g., gravity card)
  lv_color_t panel_alt_bg;    // Alternate panel background (e.g., temp)
  lv_color_t muted_bg;        // Muted background for small boxes (time/rssi)
  lv_color_t status_bg;       // Status bar background
  lv_color_t battery_good;    // Battery fill - good
  lv_color_t battery_mid;     // Battery fill - medium
  lv_color_t battery_low;     // Battery fill - low
  lv_color_t battery_border;  // Battery border color
} ui_theme_colors_t;

/**
 * Get color theme for specified mode
 */
ui_theme_colors_t ui_get_theme_colors(ui_theme_t theme);

/**
 * Apply theme colors to a UI object (label or button)
 */
void ui_apply_theme_to_object(lv_obj_t* obj, const ui_theme_colors_t* colors);

/**
 * Apply theme to all objects in a screen
 */
void ui_apply_theme_to_screen(lv_obj_t* scr, const ui_theme_colors_t* colors);

/**
 * Create a label with specific position, size, alignment, color, and font
 * @param parent Parent object
 * @param text Label text
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param align Text alignment (LV_TEXT_ALIGN_LEFT, etc.)
 * @param color Text color
 * @param font Text font (e.g., &lv_font_montserrat_18)
 * @return Created label object
 */
lv_obj_t* ui_create_label(lv_obj_t* parent, const char* text, lv_coord_t x,
                          lv_coord_t y, lv_coord_t w, lv_coord_t h,
                          lv_text_align_t align, lv_color_t color,
                          const lv_font_t* font);

/**
 * Create a styled label with predefined style
 * @param parent Parent object
 * @param text Label text
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param style Predefined lv_style_t with font, alignment, and color
 * @return Created label object
 */
lv_obj_t* ui_create_styled_label(lv_obj_t* parent, const char* text,
                                 lv_coord_t x, lv_coord_t y, lv_coord_t w,
                                 lv_coord_t h, lv_style_t* style);

/**
 * Create a status bar label
 * @param parent Parent object
 * @param text Label text
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param align Text alignment
 * @param color Text color
 * @param font Text font (e.g., &lv_font_montserrat_12)
 * @return Created label object
 */
lv_obj_t* ui_create_status_label(lv_obj_t* parent, const char* text,
                                 lv_coord_t x, lv_coord_t y, lv_coord_t w,
                                 lv_coord_t h, lv_text_align_t align,
                                 lv_color_t color, const lv_font_t* font);

/**
 * Create a button with label
 * @param parent Parent object
 * @param label_text Button label text
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param callback Event callback (NULL for no callback)
 * @param bg_color Background color
 * @param text_color Text color
 * @param font Text font (e.g., &lv_font_montserrat_18)
 * @return Created button object
 */
lv_obj_t* ui_create_button(lv_obj_t* parent, const char* label_text,
                           lv_coord_t x, lv_coord_t y, lv_coord_t w,
                           lv_coord_t h, lv_event_cb_t callback,
                           lv_color_t bg_color, lv_color_t text_color,
                           const lv_font_t* font);

/**
 * Create a styled button with predefined style
 * @param parent Parent object
 * @param label_text Button label text
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 * @param callback Event callback (NULL for no callback)
 * @param bg_color Background color
 * @param style Predefined lv_style_t for text styling
 * @return Created button object
 */
lv_obj_t* ui_create_styled_button(lv_obj_t* parent, const char* label_text,
                                  lv_coord_t x, lv_coord_t y, lv_coord_t w,
                                  lv_coord_t h, lv_event_cb_t callback,
                                  lv_color_t bg_color, lv_style_t* style);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ENABLE_LVGL

#endif  // SRC_UI_HELPERS_HPP_

// EOF
