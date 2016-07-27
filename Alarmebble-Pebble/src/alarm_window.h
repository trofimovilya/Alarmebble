#pragma once

#include <pebble.h>

#define ALARM_WINDOW_NUM_CELLS 2
#define ALARM_WINDOW_MAX_VALUE 60
#define ALARM_WINDOW_SIZE GSize(128, 34)

#define ALARM_WINDOW_WRONG_TIME "Are you gonna sleep?"
#define ALARM_WINDOW_SET_ALARM "Set alarm for"
#define ALARM_WINDOW_SLEEP_FOR "Sleep for"
#define ALARM_WINDOW_FROM_NOW "from now"
#define ALARM_WINDOW_WAKE_UP_AT "wake up at"

typedef struct { int time[ALARM_WINDOW_NUM_CELLS]; } ALARM;

typedef void (*AlarmWindowComplete)(int h, int m);

typedef struct AlarmWindowCallbacks {
  AlarmWindowComplete alarm_complete;
} AlarmWindowCallbacks;

typedef struct {
  Window *window;
  TextLayer *main_text, *sub_text;
  Layer *selection;
  GColor highlight_color;
  StatusBarLayer *status;
  AlarmWindowCallbacks callbacks;

  ALARM alarm;
  char field_buffs[ALARM_WINDOW_NUM_CELLS][2];
  int8_t field_selection;
} AlarmWindow;

AlarmWindow *alarm_window_create(AlarmWindowCallbacks alarm_window_callbacks);

void alarm_window_destroy(AlarmWindow *alarm_window);
void alarm_window_push(AlarmWindow *alarm_window, bool animated);
void alarm_window_pop(AlarmWindow *alarm_window, bool animated);
bool alarm_window_get_topmost_window(AlarmWindow *alarm_window);
void alarm_window_set_highlight_color(AlarmWindow *alarm_window, GColor color);
