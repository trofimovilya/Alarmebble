#include <pebble.h>

#include "alarm_window.h"
#include "dialog_message_window.h"
#include "responses.h"

static void alarm_complete_callback(int h, int m) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_int(iter, 0, &h, sizeof(int), true);
  dict_write_int(iter, 1, &m, sizeof(int), true);

  app_message_outbox_send();
}

static void outbox_failed_handler(DictionaryIterator *iter,
                                  AppMessageResult reason, void *context) {
  dialog_message_window_push(STATUS_NO_CONN);
}

static void inbox_recieved_handler(DictionaryIterator *iter, void *context) {
  Tuple *data = dict_read_first(iter);
  if (data) {
    dialog_message_window_push((int)data->value->int32);
  }
}

static void init() {
  app_message_register_outbox_failed(outbox_failed_handler);
  app_message_register_inbox_received(inbox_recieved_handler);

  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);

  AlarmWindow *alarm_window = alarm_window_create(
      (AlarmWindowCallbacks){.alarm_complete = alarm_complete_callback});
  alarm_window_push(alarm_window, true);
}

static void deinit() {}

int main() {
  init();
  app_event_loop();
  deinit();
}
