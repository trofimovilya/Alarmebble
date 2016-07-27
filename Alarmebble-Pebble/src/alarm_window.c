#include "alarm_window.h"
#include "selection_layer.h"
#include <pebble.h>
#include <time.h>

static char sub_text[64];
static bool alarm_mode = true;
static bool block = true;
static int sub_text_minutes = 0;
static int sub_text_hours = 0;

static char *selection_handle_get_text(int index, void *context) {
  AlarmWindow *alarm_window = (AlarmWindow *)context;
  int digit = (int)alarm_window->alarm.time[index];
  snprintf(alarm_window->field_buffs[index], 3, "%02d", digit);
  return alarm_window->field_buffs[index];
}

static void selection_handle_complete(void *context) {
  if (block) {
    return;
  }

  AlarmWindow *alarm_window = (AlarmWindow *)context;

  if (alarm_mode) {
    alarm_window->callbacks.alarm_complete(alarm_window->alarm.time[0],
                                           alarm_window->alarm.time[1]);
  } else {
    alarm_window->callbacks.alarm_complete(sub_text_hours, sub_text_minutes);
  }
}

static void update_time(AlarmWindow *window) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  int current_in_min = tick_time->tm_hour * 60 + tick_time->tm_min;
  int future_in_min = window->alarm.time[0] * 60 + window->alarm.time[1];

  int sub_time = (alarm_mode ? (1440 - current_in_min + future_in_min)
                             : (current_in_min + future_in_min)) % 1440;
  sub_text_hours = sub_time / 60;
  sub_text_minutes = sub_time % 60;

  if ((alarm_mode && sub_text_hours == 0 && sub_text_minutes == 0) ||
      (!alarm_mode && window->alarm.time[0] == 0 &&
       window->alarm.time[1] == 0)) {
    snprintf(sub_text, sizeof(sub_text), ALARM_WINDOW_WRONG_TIME);
    block = true;
  } else {
    snprintf(sub_text, sizeof(sub_text),
             alarm_mode ? "%02d:%02d\nfrom now" : "wake up at\n%02d:%02d",
             sub_text_hours, sub_text_minutes);
    block = false;
  }
}

static void selection_handle_change_mode(void *context) {
  alarm_mode = !alarm_mode;
  AlarmWindow *alarm_window = (AlarmWindow *)context;
  text_layer_set_text(alarm_window->main_text, alarm_mode
                                                   ? ALARM_WINDOW_SET_ALARM
                                                   : ALARM_WINDOW_SLEEP_FOR);
  update_time(alarm_window);
  vibes_short_pulse();
}

static void selection_handle_inc(int index, uint8_t clicks, void *context) {
  AlarmWindow *alarm_window = (AlarmWindow *)context;

  alarm_window->alarm.time[index]++;
  if (alarm_window->alarm.time[0] > 23 || alarm_window->alarm.time[1] > 59) {
    alarm_window->alarm.time[index] = 0;
  }

  update_time(alarm_window);
}

static void selection_handle_dec(int index, uint8_t clicks, void *context) {
  AlarmWindow *alarm_window = (AlarmWindow *)context;
  alarm_window->alarm.time[index]--;

  if (alarm_window->alarm.time[index] < 0) {
    if (index == 0) {
      alarm_window->alarm.time[0] = 23;
    } else if (index == 1) {
      alarm_window->alarm.time[1] = 59;
    }
  }

  update_time(alarm_window);
}

AlarmWindow *alarm_window_create(AlarmWindowCallbacks callbacks) {
  AlarmWindow *alarm_window = (AlarmWindow *)malloc(sizeof(AlarmWindow));
  if (alarm_window) {
    alarm_window->window = window_create();
    alarm_window->callbacks = callbacks;
    if (alarm_window->window) {
      alarm_window->field_selection = 0;
      window_set_background_color(
          alarm_window->window, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorWhite));
      alarm_window->alarm.time[0] = 6;
      alarm_window->alarm.time[1] = 0;

      // Get window parameters
      Layer *window_layer = window_get_root_layer(alarm_window->window);
      GRect bounds = layer_get_bounds(window_layer);

      // Main TextLayer
      const GEdgeInsets main_text_insets = {.top = 24};
      alarm_window->main_text =
          text_layer_create(grect_inset(bounds, main_text_insets));
      text_layer_set_text(alarm_window->main_text, ALARM_WINDOW_SET_ALARM);
      text_layer_set_font(alarm_window->main_text,
                          fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
      text_layer_set_text_alignment(alarm_window->main_text,
                                    GTextAlignmentCenter);
      text_layer_set_text_color(alarm_window->main_text,
                                PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
      text_layer_set_background_color(alarm_window->main_text, GColorClear);
      layer_add_child(window_layer,
                      text_layer_get_layer(alarm_window->main_text));
      // Sub TextLayer
      const GEdgeInsets sub_text_insets = {
          .top = 105, .right = 5, .bottom = 5, .left = 5};
      alarm_window->sub_text =
          text_layer_create(grect_inset(bounds, sub_text_insets));

      update_time(alarm_window);
      text_layer_set_text(alarm_window->sub_text, sub_text);
      text_layer_set_text_color(alarm_window->sub_text,
                                PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
      text_layer_set_background_color(alarm_window->sub_text, GColorClear);
      text_layer_set_text_alignment(alarm_window->sub_text,
                                    GTextAlignmentCenter);
      text_layer_set_font(alarm_window->sub_text,
                          fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
      layer_add_child(window_layer,
                      text_layer_get_layer(alarm_window->sub_text));

      // Create selection layer
      const GEdgeInsets selection_insets =
          GEdgeInsets((bounds.size.h - ALARM_WINDOW_SIZE.h) / 2,
                      (bounds.size.w - ALARM_WINDOW_SIZE.w) / 2);

      alarm_window->selection = selection_layer_create(
          grect_inset(bounds, selection_insets), ALARM_WINDOW_NUM_CELLS);

      for (int i = 0; i < ALARM_WINDOW_NUM_CELLS; i++) {
        selection_layer_set_cell_width(alarm_window->selection, i, 62);
      }

      selection_layer_set_cell_padding(alarm_window->selection, 4);
      selection_layer_set_active_bg_color(
          alarm_window->selection,
          PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorBlack));
      selection_layer_set_inactive_bg_color(
          alarm_window->selection,
          PBL_IF_COLOR_ELSE(GColorBlack, GColorLightGray));
      selection_layer_set_click_config_onto_window(alarm_window->selection,
                                                   alarm_window->window);
      selection_layer_set_callbacks(
          alarm_window->selection, alarm_window,
          (SelectionLayerCallbacks){
              .get_cell_text = selection_handle_get_text,
              .complete = selection_handle_complete,
              .increment = selection_handle_inc,
              .decrement = selection_handle_dec,
              .change_mode = selection_handle_change_mode,
          });
      layer_add_child(window_get_root_layer(alarm_window->window),
                      alarm_window->selection);

      // Create status bar
      alarm_window->status = status_bar_layer_create();
      status_bar_layer_set_colors(alarm_window->status, GColorBlack,
                                  GColorWhite);
      layer_add_child(window_layer,
                      status_bar_layer_get_layer(alarm_window->status));
      return alarm_window;
    }
  }

  APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create AlarmWindow");
  return NULL;
}

void alarm_window_destroy(AlarmWindow *alarm_window) {
  if (alarm_window) {
    status_bar_layer_destroy(alarm_window->status);
    selection_layer_destroy(alarm_window->selection);
    text_layer_destroy(alarm_window->sub_text);
    text_layer_destroy(alarm_window->main_text);
    free(alarm_window);
    alarm_window = NULL;
    return;
  }
}

void alarm_window_push(AlarmWindow *alarm_window, bool animated) {
  window_stack_push(alarm_window->window, animated);
}

void alarm_window_pop(AlarmWindow *alarm_window, bool animated) {
  window_stack_remove(alarm_window->window, animated);
}

bool alarm_window_get_topmost_window(AlarmWindow *alarm_window) {
  return window_stack_get_top_window() == alarm_window->window;
}

void alarm_window_set_highlight_color(AlarmWindow *alarm_window, GColor color) {
  alarm_window->highlight_color = color;
  selection_layer_set_active_bg_color(alarm_window->selection, color);
}
