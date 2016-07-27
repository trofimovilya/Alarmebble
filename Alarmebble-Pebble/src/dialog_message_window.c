#include "dialog_message_window.h"
#include "responses.h"

static Window *s_main_window;
static TextLayer *s_label_layer;
static Layer *s_background_layer, *s_icon_layer;

static Animation *s_appear_anim = NULL;
static GBitmap *s_icon_bitmap;
static int s_result_code = 0;

static void anim_stopped_handler(Animation *animation, bool finished,
                                 void *context) {
  s_appear_anim = NULL;
}

static void background_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(
      ctx, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorWhite));
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, 0);
}

static void icon_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(
      ctx, s_icon_bitmap,
      (GRect){.origin = bounds.origin, .size = bitmap_bounds.size});
}

static void window_unload(Window *window) {
  layer_destroy(s_background_layer);
  layer_destroy(s_icon_layer);

  text_layer_destroy(s_label_layer);

  gbitmap_destroy(s_icon_bitmap);

  window_destroy(window);
  s_main_window = NULL;
}

static void single_click_handler(ClickRecognizerRef recognizer, void *context) {
  window_stack_pop_all(true);
}

static void config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_SELECT, single_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, single_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_click_config_provider(window,
                                   (ClickConfigProvider)config_provider);

  const GEdgeInsets background_insets = {.top = bounds.size.h};
  s_background_layer = layer_create(grect_inset(bounds, background_insets));
  layer_set_update_proc(s_background_layer, background_update_proc);
  layer_add_child(window_layer, s_background_layer);

  s_icon_bitmap = s_result_code == STATUS_OK
                      ? gbitmap_create_with_resource(RESOURCE_ID_OK)
                      : gbitmap_create_with_resource(RESOURCE_ID_CROSS);
  GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);

  s_icon_layer =
      layer_create(GRect((bounds.size.w - bitmap_bounds.size.w) / 2,
                         bounds.size.h + DIALOG_MESSAGE_WINDOW_MARGIN,
                         bitmap_bounds.size.w, bitmap_bounds.size.h));
  layer_set_update_proc(s_icon_layer, icon_update_proc);
  layer_add_child(window_layer, s_icon_layer);

  s_label_layer = text_layer_create(
      GRect(DIALOG_MESSAGE_WINDOW_MARGIN,
            bounds.size.h + DIALOG_MESSAGE_WINDOW_MARGIN + bitmap_bounds.size.h,
            bounds.size.w - (2 * DIALOG_MESSAGE_WINDOW_MARGIN), bounds.size.h));

  switch (s_result_code) {
  case STATUS_OK:
    vibes_short_pulse();
    text_layer_set_text(s_label_layer, MESSAGE_OK);
    break;
  case STATUS_ERROR:
    vibes_long_pulse();
    text_layer_set_text(s_label_layer, MESSAGE_ERROR);
    break;
  case STATUS_NO_CONN:
    vibes_long_pulse();
    text_layer_set_text(s_label_layer, MESSAGE_NO_CONN);
    break;
  }

  text_layer_set_text_color(s_label_layer,
                            PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
  text_layer_set_background_color(s_label_layer, GColorClear);
  text_layer_set_text_alignment(s_label_layer, GTextAlignmentCenter);
  text_layer_set_font(s_label_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_label_layer));
}

static void timer_callback(void *context) { window_stack_pop_all(true); }

static void window_appear(Window *window) {
  if (s_appear_anim) {
    animation_unschedule(s_appear_anim);
  }

  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  GRect bitmap_bounds = gbitmap_get_bounds(s_icon_bitmap);

  Layer *label_layer = text_layer_get_layer(s_label_layer);

  GRect start = layer_get_frame(s_background_layer);
  GRect finish = bounds;
  Animation *background_anim =
      (Animation *)property_animation_create_layer_frame(s_background_layer,
                                                         &start, &finish);

  start = layer_get_frame(s_icon_layer);
  const GEdgeInsets icon_insets = {
      .top = (bounds.size.h - bitmap_bounds.size.h) / 2 - 20,
      .left = (bounds.size.w - bitmap_bounds.size.w) / 2};
  finish = grect_inset(bounds, icon_insets);
  Animation *icon_anim = (Animation *)property_animation_create_layer_frame(
      s_icon_layer, &start, &finish);

  start = layer_get_frame(label_layer);
  const GEdgeInsets finish_insets = {.top = DIALOG_MESSAGE_WINDOW_MARGIN +
                                            bitmap_bounds.size.h + 30,
                                     .right = DIALOG_MESSAGE_WINDOW_MARGIN,
                                     .left = DIALOG_MESSAGE_WINDOW_MARGIN};
  finish = grect_inset(bounds, finish_insets);
  Animation *label_anim = (Animation *)property_animation_create_layer_frame(
      label_layer, &start, &finish);

  s_appear_anim =
      animation_spawn_create(background_anim, icon_anim, label_anim, NULL);
  animation_set_handlers(s_appear_anim,
                         (AnimationHandlers){.stopped = anim_stopped_handler},
                         NULL);
  animation_set_delay(s_appear_anim, 100);
  animation_schedule(s_appear_anim);
}

void dialog_message_window_push(int result_code) {
  if (!s_main_window) {
    s_main_window = window_create();
    s_result_code = result_code;
    window_set_background_color(s_main_window, GColorBlack);
    window_set_window_handlers(s_main_window,
                               (WindowHandlers){.load = window_load,
                                                .unload = window_unload,
                                                .appear = window_appear});
    app_timer_register(5000, timer_callback, NULL);
  }
  window_stack_push(s_main_window, true);
}
