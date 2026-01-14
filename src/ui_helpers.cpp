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
#if defined(ENABLE_LVGL)

#include <ui_helpers.hpp>

/**
 * Create a label with specific position, size, alignment, and color
 */
lv_obj_t* ui_create_label(lv_obj_t* parent, const char* text, lv_coord_t x,
                          lv_coord_t y, lv_coord_t w, lv_coord_t h,
                          lv_text_align_t align, lv_color_t color,
                          const lv_font_t* font) {
  lv_obj_t* lbl = lv_label_create(parent);
  lv_label_set_text(lbl, text);
  lv_obj_set_pos(lbl, x, y);
  lv_obj_set_size(lbl, w, h);
  lv_obj_set_style_text_align(lbl, align, LV_PART_MAIN);
  lv_obj_set_style_text_color(lbl, color, LV_PART_MAIN);
  if (font) {
    lv_obj_set_style_text_font(lbl, font, LV_PART_MAIN);
  }
  return lbl;
}

/**
 * Create a styled label with predefined style
 */
lv_obj_t* ui_create_styled_label(lv_obj_t* parent, const char* text,
                                 lv_coord_t x, lv_coord_t y, lv_coord_t w,
                                 lv_coord_t h, lv_style_t* style) {
  lv_obj_t* lbl = lv_label_create(parent);
  lv_label_set_text(lbl, text);
  lv_obj_set_pos(lbl, x, y);
  lv_obj_set_size(lbl, w, h);
  if (style) {
    lv_obj_add_style(lbl, style, 0);
  }
  return lbl;
}

/**
 * Create a status bar label (smaller font, 12px)
 */
lv_obj_t* ui_create_status_label(lv_obj_t* parent, const char* text,
                                 lv_coord_t x, lv_coord_t y, lv_coord_t w,
                                 lv_coord_t h, lv_text_align_t align,
                                 lv_color_t color, const lv_font_t* font) {
  lv_obj_t* lbl = lv_label_create(parent);
  lv_label_set_text(lbl, text);
  lv_obj_set_pos(lbl, x, y);
  lv_obj_set_size(lbl, w, h);
  lv_obj_set_style_text_align(lbl, align, LV_PART_MAIN);
  lv_obj_set_style_text_color(lbl, color, LV_PART_MAIN);
  if (font) {
    lv_obj_set_style_text_font(lbl, font, LV_PART_MAIN);
  }
  return lbl;
}

/**
 * Create a button with specific position, size, alignment, and colors
 */
lv_obj_t* ui_create_button(lv_obj_t* parent, const char* label_text,
                           lv_coord_t x, lv_coord_t y, lv_coord_t w,
                           lv_coord_t h, lv_event_cb_t callback,
                           lv_color_t bg_color, lv_color_t text_color,
                           const lv_font_t* font) {
  lv_obj_t* btn = lv_btn_create(parent);
  lv_obj_set_pos(btn, x, y);
  lv_obj_set_size(btn, w, h);
  lv_obj_set_style_bg_color(btn, bg_color, LV_PART_MAIN);
  lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);

  // Add pressed state feedback - darker background and thicker border
  lv_color_t pressed_color = lv_color_darken(bg_color, 60);
  lv_obj_set_style_bg_color(btn, pressed_color,
                            LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_style_border_width(btn, 3, LV_PART_MAIN | LV_STATE_PRESSED);
  lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_PRESSED);

  if (callback) {
    lv_obj_add_event_cb(btn, callback, LV_EVENT_PRESSED, NULL);
  }

  lv_obj_t* lbl = lv_label_create(btn);
  lv_label_set_text(lbl, label_text);
  lv_obj_center(lbl);
  lv_obj_set_style_text_color(lbl, text_color, LV_PART_MAIN);
  if (font) {
    lv_obj_set_style_text_font(lbl, font, LV_PART_MAIN);
  }
  return btn;
}

/**
 * Create a styled button with predefined style
 */
lv_obj_t* ui_create_styled_button(lv_obj_t* parent, const char* label_text,
                                  lv_coord_t x, lv_coord_t y, lv_coord_t w,
                                  lv_coord_t h, lv_event_cb_t callback,
                                  lv_color_t bg_color, lv_style_t* style) {
  lv_obj_t* btn = lv_btn_create(parent);
  lv_obj_set_pos(btn, x, y);
  lv_obj_set_size(btn, w, h);
  lv_obj_set_style_bg_color(btn, bg_color, LV_PART_MAIN);
  lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);

  if (callback) {
    lv_obj_add_event_cb(btn, callback, LV_EVENT_PRESSED, NULL);
  }

  lv_obj_t* lbl = lv_label_create(btn);
  lv_label_set_text(lbl, label_text);
  lv_obj_center(lbl);
  if (style) {
    lv_obj_add_style(lbl, style, 0);
  }

  return btn;
}

/**
 * Get color theme for specified mode
 */
ui_theme_colors_t ui_get_theme_colors(ui_theme_t theme) {
  ui_theme_colors_t colors;

  if (theme == UI_THEME_DARK) {
    colors.bg = lv_color_hex(0x1F1F1F);
    colors.text = lv_color_hex(0xFFFFFF);
    colors.border = lv_color_hex(0x404040);
    colors.button_bg = lv_color_hex(0x0066FF);    // Bright blue for buttons
    colors.button_text = lv_color_hex(0x000000);  // Black text on buttons
    // Dark mode panel / alternate colors
    colors.panel_bg = lv_color_hex(0x263238);
    colors.panel_alt_bg = lv_color_hex(0x3E2723);
    colors.muted_bg = lv_color_hex(0x2B2B2B);
    colors.status_bg = lv_color_hex(0x0F0F0F);
    // Battery colors (keep bright for visibility)
    colors.battery_good = lv_color_hex(0x33FF33);
    colors.battery_mid = lv_color_hex(0xFFFF33);
    colors.battery_low = lv_color_hex(0xFF3333);
    colors.battery_border = lv_color_hex(0xAAAAAA);
  } else {
    colors.bg = lv_color_hex(0xFFFFFF);
    colors.text = lv_color_hex(0x000000);  // Black text on white background
    colors.border = lv_color_hex(0xE0E0E0);
    colors.button_bg = lv_color_hex(0x0066FF);    // Bright blue for buttons
    colors.button_text = lv_color_hex(0xFFFFFF);  // White text on buttons
    // Light mode panel / alternate colors
    colors.panel_bg = lv_color_hex(0xE3F2FD);  // light blue card
    colors.panel_alt_bg = lv_color_hex(0xFFF8E1);
    colors.muted_bg = lv_color_hex(0xE8E8E8);
    colors.status_bg = lv_color_hex(0xF0F0F0);
    // Battery colors
    colors.battery_good = lv_color_hex(0x33FF33);
    colors.battery_mid = lv_color_hex(0xFFFF33);
    colors.battery_low = lv_color_hex(0xFF3333);
    colors.battery_border = lv_color_hex(0x33AA33);
  }

  return colors;
}

/**
 * Apply theme colors to a single UI object
 */
void ui_apply_theme_to_object(lv_obj_t* obj, const ui_theme_colors_t* colors) {
  if (!obj || !colors) return;

  // Apply text color
  lv_obj_set_style_text_color(obj, colors->text, LV_PART_MAIN);
}

/**
 * Apply theme to screen background and all children
 */
void ui_apply_theme_to_screen(lv_obj_t* screen,
                              const ui_theme_colors_t* colors) {
  if (!screen || !colors) return;

  // Apply background to screen
  lv_obj_set_style_bg_color(screen, colors->bg, LV_PART_MAIN);

  // Apply theme to all child objects
  uint32_t child_count = lv_obj_get_child_cnt(screen);
  for (uint32_t i = 0; i < child_count; i++) {
    lv_obj_t* child = lv_obj_get_child(screen, i);
    if (child) {
      ui_apply_theme_to_object(child, colors);

      // Recursively apply to grandchildren (button labels)
      uint32_t grandchild_count = lv_obj_get_child_cnt(child);
      for (uint32_t j = 0; j < grandchild_count; j++) {
        lv_obj_t* grandchild = lv_obj_get_child(child, j);
        if (grandchild) {
          // Grandchildren are button labels, use button text color
          lv_obj_set_style_text_color(grandchild, colors->button_text,
                                      LV_PART_MAIN);
        }
      }
    }
  }
}
#endif  // ENABLE_LVGL

// EOF
