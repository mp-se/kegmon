/*
MIT License

Copyright (c) 2026 Magnus

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
#ifndef SRC_UI_KEGMON_HPP_
#define SRC_UI_KEGMON_HPP_

#if defined(ENABLE_LVGL)

#include <lvgl.h>
#include <stdbool.h>

#include <changedetection.hpp>
#include <ui_helpers.hpp>

#ifdef __cplusplus
extern "C" {
#endif

/* Layout IDs */
#define KEGMON_LAYOUT_4KEG 0
#define KEGMON_LAYOUT_2KEG 1

/**
 * Initialize kegmon UI
 * Must be called after `hw_display_init()`
 *
 * @param disp LVGL display object
 * @param darkmode Initial theme (true=dark, false=light)
 * @param layout_id Layout configuration (KEGMON_LAYOUT_4KEG or
 * KEGMON_LAYOUT_2KEG)
 */
void kegmon_init(lv_disp_t* disp, bool darkmode, uint8_t layout_id);

/**
 * Main loop handler to apply pending UI updates. Call from LVGL thread.
 */
void kegmon_loop(void);

/**
 * Cleanup and destroy UI
 */
void kegmon_cleanup(void);

/* Granular setters (thread-safe): */
void kegmon_set_keg_name(uint8_t index, const char* name);
void kegmon_set_connected(uint8_t index, bool connected);
void kegmon_set_state(uint8_t index, const char* state_str);
void kegmon_set_keg_volume(uint8_t index, uint32_t keg_volume);
void kegmon_set_stable_weight(uint8_t index, float weight);
void kegmon_set_stable_volume(uint8_t index, float volume);
void kegmon_set_last_pour_volume(uint8_t index, float last_pour);
void kegmon_set_temperature(uint8_t index, float temp);
void kegmon_set_theme(bool darkmode);
void kegmon_set_volume_unit_format(const char* format);
void kegmon_set_temp_unit_format(const char* format);
void kegmon_set_event_type(uint8_t index, ChangeDetectionEventType event_type);
void kegmon_set_device_status(const char* status);

/**
 * Change the layout at runtime
 * @param layout_id Layout configuration (KEGMON_LAYOUT_4KEG or
 * KEGMON_LAYOUT_2KEG)
 */
void kegmon_set_layout(uint8_t layout_id);

/**
 * Get the current layout ID
 * @return Current layout configuration (KEGMON_LAYOUT_4KEG or
 * KEGMON_LAYOUT_2KEG)
 */
uint8_t kegmon_get_layout(void);

typedef struct {
  bool connected;
  char state[32];
  float stable_weight;
  int32_t keg_volume;
  float last_pour_volume;
  ChangeDetectionEventType event_type;
} kegmon_scale_t;

void kegmon_set_beer_meta(uint8_t index, float abv, int ebc, int ibu);

#ifdef __cplusplus
}
#endif

#endif  // ENABLE_LVGL

#endif  // SRC_UI_KEGMON_HPP_

// EOF
