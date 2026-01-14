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
#if defined(ENABLE_LVGL)

#include <changedetection.hpp>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ui_helpers.hpp>
#include <ui_kegmon.hpp>

static const uint8_t KEGMON_MAX_KEGS = 4;
static bool g_darkmode = false;
static lv_disp_t* g_disp = NULL;
static char g_volume_unit_format[10] = "";
static char g_temp_unit_format[10] = "";
static lv_obj_t* g_status_bar = NULL;
static lv_obj_t* g_status_label = NULL;
static char g_device_ip[64] = "--";
static bool g_device_connected = false;

/**
 * Layout manager
 */
typedef struct {
  uint8_t current_layout;
  uint8_t total_layouts;
} kegmon_layout_mgr_t;

static kegmon_layout_mgr_t layout_mgr = {
    .current_layout = KEGMON_LAYOUT_4KEG,
    .total_layouts = 2,
};

typedef struct {
  // UI objects for this keg
  lv_obj_t* container;
  lv_obj_t* lbl_name;
  lv_obj_t* lbl_volume;
  lv_obj_t* bar_fill;
  lv_obj_t* lbl_state;
  lv_obj_t* lbl_pours;
  lv_obj_t* lbl_meta;

  // Cached values to avoid redundant updates
  kegmon_scale_t last_scale;
  float last_temp;
  char last_name[32];
  char last_meta[64];
  uint32_t keg_capacity;
  bool created;
} kegmon_ui_t;

static kegmon_ui_t* g_kegs = NULL;

static void create_keg_ui(uint8_t idx, lv_obj_t* parent, int32_t x, int32_t y,
                          int32_t w, int32_t h) {
  kegmon_ui_t* k = &g_kegs[idx];
  k->container = lv_obj_create(parent);
  lv_obj_set_pos(k->container, x, y);
  lv_obj_set_size(k->container, w, h);
  ui_theme_colors_t theme =
      ui_get_theme_colors(g_darkmode ? UI_THEME_DARK : UI_THEME_LIGHT);
  lv_obj_set_style_bg_color(k->container, theme.panel_bg, LV_PART_MAIN);
  lv_obj_set_style_radius(k->container, 6, 0);
  lv_obj_set_style_pad_all(k->container, 4, 0);
  lv_obj_set_style_clip_corner(k->container, true, 0);
  lv_obj_set_style_border_color(k->container, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_width(k->container, 2, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->container, LV_OPA_80, LV_PART_MAIN);
  lv_obj_clear_flag(k->container, LV_OBJ_FLAG_SCROLLABLE);

  // Beer name (top area)
  k->lbl_name =
      ui_create_label(k->container, "", 2, 4, w - 12, 16, LV_TEXT_ALIGN_CENTER,
                      theme.text, &lv_font_montserrat_12);
  lv_obj_set_style_border_color(k->lbl_name, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->lbl_name, LV_OPA_60, LV_PART_MAIN);

  // Small metadata line under the name (ABV / EBC / IBU)
  k->lbl_meta =
      ui_create_label(k->container, "", 2, 4 + 16 + 2, w - 12, 12,
                      LV_TEXT_ALIGN_CENTER, theme.text, &lv_font_montserrat_10);
  lv_obj_set_style_text_color(k->lbl_meta, theme.text, LV_PART_MAIN);

  // Volume & progress bar in middle (left 60% of area)
  k->lbl_volume =
      ui_create_label(k->container, "", 2, h / 5 + 23, w - 12, 12,
                      LV_TEXT_ALIGN_CENTER, theme.text, &lv_font_montserrat_10);
  lv_obj_set_style_border_color(k->lbl_volume, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->lbl_volume, LV_OPA_60, LV_PART_MAIN);

  k->bar_fill = lv_bar_create(k->container);
  lv_obj_set_pos(k->bar_fill, 2, h / 5 + 22);
  lv_obj_set_size(k->bar_fill, w - 12, 12);
  lv_bar_set_range(k->bar_fill, 0, 100);
  lv_bar_set_value(k->bar_fill, 0, LV_ANIM_OFF);
  lv_obj_set_style_border_color(k->bar_fill, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->bar_fill, LV_OPA_60, LV_PART_MAIN);
  lv_obj_move_foreground(k->lbl_volume);

  // Combined "Last pour · Temp" label directly under the progress bar
  int bar_y = h / 5 + 26;
  int bar_h = 10;
  int info_y = bar_y + bar_h + 6;
  k->lbl_pours =
      ui_create_label(k->container, "", 2, info_y, w - 12, 12,
                      LV_TEXT_ALIGN_CENTER, theme.text, &lv_font_montserrat_10);
  lv_obj_set_style_border_color(k->lbl_pours, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->lbl_pours, LV_OPA_60, LV_PART_MAIN);
  lv_obj_clear_flag(k->lbl_pours, LV_OBJ_FLAG_SCROLLABLE);

  // State label (will be replaced by icons in next stage) placed below combined
  // info
  k->lbl_state =
      ui_create_label(k->container, "--", 2, info_y + 16, w - 12, 12,
                      LV_TEXT_ALIGN_CENTER, theme.text, &lv_font_montserrat_10);
  lv_obj_set_style_border_color(k->lbl_state, theme.border, LV_PART_MAIN);
  lv_obj_set_style_border_opa(k->lbl_state, LV_OPA_60, LV_PART_MAIN);
  lv_obj_clear_flag(k->lbl_state, LV_OBJ_FLAG_SCROLLABLE);

  // initialize cached values
  memset(&k->last_scale, 0, sizeof(k->last_scale));
  k->last_temp = -999.0f;
  k->last_name[0] = '\0';
  k->created = true;
  k->last_meta[0] = '\0';
}

/**
 * Delete any UI objects created by the active layout and clear pointers.
 *
 * This frees LVGL objects referenced in kegmon_ui_t structures and sets
 * those pointers to NULL so a fresh layout can be created.
 */
static void kegmon_cleanup_layout(void) {
  if (!g_kegs) return;
  for (uint8_t i = 0; i < KEGMON_MAX_KEGS; i++) {
    kegmon_ui_t* k = &g_kegs[i];
    if (k->container) {
      lv_obj_del(k->container);
      k->container = NULL;
    }
    k->lbl_name = NULL;
    k->lbl_volume = NULL;
    k->bar_fill = NULL;
    k->lbl_state = NULL;
    k->lbl_pours = NULL;
    k->lbl_meta = NULL;
    k->created = false;
  }
}

/**
 * Common initialization for the Kegmon UI.
 *
 * Performs one-time setup such as storing the display reference and
 * initializing default data buffers and the layout manager.
 *
 * @param disp Display pointer used by LVGL.
 * @param darkmode True to select dark theme defaults, false for light.
 */
static void kegmon_init_common(lv_disp_t* disp, bool darkmode) {
  g_disp = disp;
  g_darkmode = darkmode;

  // Get active screen
  lv_obj_t* scr = lv_scr_act();
  if (!scr) {
    return;
  }

  // Get theme colors
  ui_theme_colors_t theme_colors =
      ui_get_theme_colors(darkmode ? UI_THEME_DARK : UI_THEME_LIGHT);

  // Apply background
  lv_obj_set_style_bg_color(scr, theme_colors.bg, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

  // Initialize layout manager
  layout_mgr.current_layout = KEGMON_LAYOUT_4KEG;
  layout_mgr.total_layouts = 2;
}

/**
 * Setup layout 0 (4-keg): creates a 2x2 grid layout with all 4 kegs visible.
 */
static void kegmon_setup_layout_0(void) {
  if (!g_kegs) return;

  lv_obj_t* scr = lv_scr_act();
  if (!scr) return;

  int32_t W = lv_obj_get_width(scr);
  int32_t H = lv_obj_get_height(scr);

  const int margin = 4;
  const int gap = 6;
  const int status_h = 18;

  int32_t total_h_available = H - status_h - margin * 2 - gap;
  int32_t h = total_h_available / 2;

  int32_t total_w_available = W - margin * 2 - gap;
  int32_t w = total_w_available / 2;

  // Create all 4 keg UIs in 2x2 grid
  for (uint8_t i = 0; i < KEGMON_MAX_KEGS; i++) {
    int col = i % 2;
    int row = i / 2;
    int32_t x = margin + col * (w + gap);
    int32_t y = margin + row * (h + gap);
    create_keg_ui(i, scr, x, y, w, h);
    lv_obj_clear_flag(g_kegs[i].container, LV_OBJ_FLAG_HIDDEN);
  }
}

/**
 * Setup layout 1 (2-keg): creates a 1x2 grid layout with only 2 kegs visible.
 * Kegs 2 and 3 are created but hidden so they can be reused if switching
 * layouts.
 */
static void kegmon_setup_layout_1(void) {
  if (!g_kegs) return;

  lv_obj_t* scr = lv_scr_act();
  if (!scr) return;

  int32_t W = lv_obj_get_width(scr);
  int32_t H = lv_obj_get_height(scr);

  const int margin = 4;
  const int gap = 6;
  const int status_h = 18;

  int32_t total_h_available = H - status_h - margin * 2;
  int32_t h = total_h_available;  // Full height for 1 row

  int32_t total_w_available = W - margin * 2 - gap;
  int32_t w = total_w_available / 2;

  // Create all 4 keg UIs but only show kegs 0 and 1
  for (uint8_t i = 0; i < KEGMON_MAX_KEGS; i++) {
    if (i < 2) {
      // Kegs 0-1: visible in 1x2 grid
      int col = i % 2;
      int32_t x = margin + col * (w + gap);
      int32_t y = margin;
      create_keg_ui(i, scr, x, y, w, h);
      lv_obj_clear_flag(g_kegs[i].container, LV_OBJ_FLAG_HIDDEN);
    } else {
      // Kegs 2-3: created but hidden for layout switching
      create_keg_ui(i, scr, 0, 0, w, h);
      lv_obj_add_flag(g_kegs[i].container, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

void kegmon_init(lv_disp_t* disp, bool darkmode, uint8_t layout_id) {
  if (!disp) {
    return;
  }

  // allocate state (always allocate 4 kegs internally)
  if (!g_kegs) {
    g_kegs = reinterpret_cast<kegmon_ui_t*>(
        calloc(KEGMON_MAX_KEGS, sizeof(kegmon_ui_t)));
    if (!g_kegs) {
      return;
    }
  }

  // Common initialization (one-time setup)
  kegmon_init_common(disp, darkmode);

  // Clamp layout_id to valid range (0..1)
  if (layout_id >= layout_mgr.total_layouts) {
    layout_id = 0;
  }
  layout_mgr.current_layout = layout_id;

  // Setup requested layout
  if (layout_id == KEGMON_LAYOUT_4KEG) {
    kegmon_setup_layout_0();
  } else if (layout_id == KEGMON_LAYOUT_2KEG) {
    kegmon_setup_layout_1();
  }

  // Create bottom status label (matching gravitymon style)
  lv_obj_t* scr = lv_scr_act();
  int32_t H = lv_obj_get_height(scr);
  int32_t W = lv_obj_get_width(scr);
  const int status_h = 18;
  int status_y = H - status_h - 2;

  ui_theme_colors_t theme =
      ui_get_theme_colors(g_darkmode ? UI_THEME_DARK : UI_THEME_LIGHT);
  g_status_label = ui_create_status_label(scr, "", 4, status_y, W - 8, status_h,
                                          LV_TEXT_ALIGN_CENTER, theme.text,
                                          &lv_font_montserrat_12);
  lv_obj_set_style_bg_color(g_status_label, theme.status_bg, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(g_status_label, LV_OPA_60, LV_PART_MAIN);
  lv_obj_set_style_border_width(g_status_label, 0, LV_PART_MAIN);
  lv_obj_set_style_border_color(g_status_label, theme.border, LV_PART_MAIN);
  lv_obj_set_style_radius(g_status_label, 0, LV_PART_MAIN);
  lv_obj_move_foreground(g_status_label);
}

void kegmon_cleanup(void) {
  if (!g_kegs) return;
  for (uint8_t i = 0; i < KEGMON_MAX_KEGS; i++) {
    if (g_kegs[i].created && g_kegs[i].container)
      lv_obj_del(g_kegs[i].container);
  }
  free(g_kegs);
  g_kegs = NULL;
}

void kegmon_update_scale(uint8_t index, const kegmon_scale_t* s) {
  if (!g_kegs || index >= KEGMON_MAX_KEGS || !s) return;
  // copy into last_scale for compare in loop
  memcpy(&g_kegs[index].last_scale, s, sizeof(kegmon_scale_t));
}

void kegmon_set_temperature(uint8_t index, float temp) {
  if (!g_kegs || index >= KEGMON_MAX_KEGS) return;
  g_kegs[index].last_temp = temp;
}

// Granular setters follow the pattern in other UI modules: update cached state
void kegmon_set_keg_name(uint8_t index, const char* name) {
  if (!g_kegs || index >= KEGMON_MAX_KEGS) return;
  if (!name) return;
  snprintf(g_kegs[index].last_name, sizeof(g_kegs[index].last_name), "%s",
           name);
}

void kegmon_set_connected(uint8_t index, bool connected) {
  if (!g_kegs || index >= KEGMON_MAX_KEGS) return;
  g_kegs[index].last_scale.connected = connected;
}

void kegmon_set_state(uint8_t index, const char* state_str) {
  if (!g_kegs || index >= KEGMON_MAX_KEGS) return;
  if (!state_str) return;
  snprintf(g_kegs[index].last_scale.state,
           sizeof(g_kegs[index].last_scale.state), "%s", state_str);
}

void kegmon_set_keg_volume(uint8_t index, uint32_t keg_volume) {
  if (!g_kegs || index >= KEGMON_MAX_KEGS) return;
  g_kegs[index].keg_capacity = keg_volume;
}

void kegmon_set_stable_weight(uint8_t index, float weight) {
  if (!g_kegs || index >= KEGMON_MAX_KEGS) return;
  g_kegs[index].last_scale.stable_weight = weight;
}

void kegmon_set_stable_volume(uint8_t index, float volume) {
  if (!g_kegs || index >= KEGMON_MAX_KEGS) return;
  g_kegs[index].last_scale.keg_volume = volume;
}

void kegmon_set_last_pour_volume(uint8_t index, float last_pour) {
  if (!g_kegs || index >= KEGMON_MAX_KEGS) return;
  g_kegs[index].last_scale.last_pour_volume = last_pour;
}

void kegmon_set_beer_meta(uint8_t index, float abv, int ebc, int ibu) {
  if (!g_kegs || index >= KEGMON_MAX_KEGS) return;

  if (abv > 0.0f && ebc > 0 && ibu > 0) {
    snprintf(g_kegs[index].last_meta, sizeof(g_kegs[index].last_meta),
             "ABV %.1f%% EBC %d IBU %d", abv, ebc, ibu);
  } else if (abv > 0.0f && ebc > 0) {
    snprintf(g_kegs[index].last_meta, sizeof(g_kegs[index].last_meta),
             "ABV %.1f%% EBC %d", abv, ebc);
  } else if (abv > 0.0f && ibu > 0) {
    snprintf(g_kegs[index].last_meta, sizeof(g_kegs[index].last_meta),
             "ABV %.1f%% IBU %d", abv, ibu);
  } else if (ebc > 0 && ibu > 0) {
    snprintf(g_kegs[index].last_meta, sizeof(g_kegs[index].last_meta),
             "EBC %d IBU %d", ebc, ibu);
  } else {
    snprintf(g_kegs[index].last_meta, sizeof(g_kegs[index].last_meta), "");
  }
}

void kegmon_set_theme(bool darkmode) { g_darkmode = darkmode; }

void kegmon_set_volume_unit_format(const char* format) {
  if (format) {
    snprintf(g_volume_unit_format, sizeof(g_volume_unit_format), "%s", format);
  }
}

void kegmon_set_temp_unit_format(const char* format) {
  if (format) {
    snprintf(g_temp_unit_format, sizeof(g_temp_unit_format), "%s", format);
  }
}

void kegmon_set_event_type(uint8_t index, ChangeDetectionEventType event_type) {
  if (!g_kegs || index >= KEGMON_MAX_KEGS) return;
  g_kegs[index].last_scale.event_type = event_type;
}

void kegmon_set_device_status(const char* status) {
  if (!status) return;
  if (g_status_label) {
    lv_label_set_text(g_status_label, status);
  }
}

/**
 * Get the current layout ID.
 *
 * @return Current layout ID (KEGMON_LAYOUT_4KEG or KEGMON_LAYOUT_2KEG)
 */
uint8_t kegmon_get_layout(void) { return layout_mgr.current_layout; }

/**
 * Switch to the specified layout ID at runtime.
 *
 * Cleans up the current layout, resets state, and constructs the chosen
 * layout. If the requested `layout_id` is invalid it will wrap to 0.
 *
 * @param layout_id Layout index to activate (KEGMON_LAYOUT_4KEG or
 * KEGMON_LAYOUT_2KEG)
 */
void kegmon_set_layout(uint8_t layout_id) {
  if (!g_kegs) return;

  if (layout_id >= layout_mgr.total_layouts) {
    layout_id = 0;
  }

  // If the requested layout is already active, do nothing
  if (layout_id == layout_mgr.current_layout) {
    return;
  }

  // Cleanup current layout objects
  kegmon_cleanup_layout();

  // Update layout ID
  layout_mgr.current_layout = layout_id;

  // Setup new layout
  if (layout_id == KEGMON_LAYOUT_4KEG) {
    kegmon_setup_layout_0();
  } else if (layout_id == KEGMON_LAYOUT_2KEG) {
    kegmon_setup_layout_1();
  }
}

void kegmon_loop(void) {
  if (!g_kegs) return;

  for (uint8_t i = 0; i < KEGMON_MAX_KEGS; i++) {
    kegmon_ui_t* k = &g_kegs[i];
    if (!k->created) continue;

    // Apply scale values (cached in last_scale)
    char buf[64];
    const kegmon_scale_t* s = &k->last_scale;

    // Update beer name label from cached name (set by setter)
    lv_label_set_text(k->lbl_name, k->last_name);

    // Volume label: show keg_volume in the configured unit format
    if (s->keg_volume > 0) {
      snprintf(buf, sizeof(buf), "%d %s", static_cast<int>(s->keg_volume),
               g_volume_unit_format);
    } else {
      snprintf(buf, sizeof(buf), "--");
    }
    lv_label_set_text(k->lbl_volume, buf);

    // Progress bar: percent of capacity
    if (k->keg_capacity > 0) {
      int pct = static_cast<int>(static_cast<float>(s->keg_volume) * 100.0f /
                                 static_cast<float>(k->keg_capacity));
      if (pct < 0) pct = 0;
      if (pct > 100) pct = 100;
      lv_bar_set_value(k->bar_fill, pct, LV_ANIM_OFF);
    }

    // Use preformatted metadata string set by `kegmon_set_beer_meta()`
    lv_label_set_text(k->lbl_meta, k->last_meta);

    // State label / Event icon mapping (event_type takes precedence)
    switch (s->event_type) {
      case ChangeDetectionEventType::POUR_STARTED:
        snprintf(buf, sizeof(buf), LV_SYMBOL_DOWN " Pouring");
        break;
      case ChangeDetectionEventType::POUR_COMPLETED:
        snprintf(buf, sizeof(buf), LV_SYMBOL_OK " Poured");
        break;
      case ChangeDetectionEventType::STABLE_DETECTED:
        // snprintf(buf, sizeof(buf), LV_SYMBOL_REFRESH " Stable");
        snprintf(buf, sizeof(buf), LV_SYMBOL_MINUS " Stable");
        break;
      case ChangeDetectionEventType::INVALID_WEIGHT:
        snprintf(buf, sizeof(buf), LV_SYMBOL_WARNING " Error");
        break;
      case ChangeDetectionEventType::KEG_REMOVED:
        // snprintf(buf, sizeof(buf), LV_SYMBOL_CLOSE " Keg gone");
        snprintf(buf, sizeof(buf), LV_SYMBOL_CLOSE " Keg gone");
        break;
      case ChangeDetectionEventType::KEG_REPLACED:
        // snprintf(buf, sizeof(buf), LV_SYMBOL_REFRESH " Replaced");
        snprintf(buf, sizeof(buf), LV_SYMBOL_REFRESH " Replaced");
        break;
      case ChangeDetectionEventType::SYSTEM_STARTUP:
      default:
        if (s->state[0] != '\0')
          snprintf(buf, sizeof(buf), "%s", s->state);
        else
          snprintf(buf, sizeof(buf), "--");
        break;
    }
    lv_label_set_text(k->lbl_state, buf);

    // Combined "Last pour · Temp" label under the progress bar
    char info[64];
    if (!isnan(k->last_temp)) {
      snprintf(info, sizeof(info), "Last %.0f - %.1f°%s", s->last_pour_volume,
               k->last_temp, g_temp_unit_format);
    } else {
      snprintf(info, sizeof(info), "Last %.0f", s->last_pour_volume);
    }

    lv_label_set_text(k->lbl_pours, info);
  }
}

#endif  // ENABLE_LVGL

// EOF
