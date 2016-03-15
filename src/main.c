#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;
static TextLayer *s_weather_symbol_layer;

static GFont s_time_font;
static GFont s_weather_symbol_font;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
	setlocale(LC_TIME, "de_DE");

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);  
	
	// Write the current day and month into a buffer
  static char s_bufferDate[30];
  strftime(s_bufferDate, sizeof(s_bufferDate), "%a, %d %B ", tick_time);
	
	// Display this date on the TextLayer
  text_layer_set_text(s_date_layer, s_bufferDate);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
	
	// Create GBitmap
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);

	// Create BitmapLayer to display the GBitmap
	s_background_layer = bitmap_layer_create(bounds);

	// Set the bitmap onto the layer and add to the window
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	// layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
	
	// ++ TEXT LAYERS with specific bounds ++
	// Time
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(0, 0), bounds.size.w, 52));
	
	// Weather Symbol
	s_weather_symbol_layer = text_layer_create(
			GRect(0, PBL_IF_ROUND_ELSE(53, 53), bounds.size.w, 70));

	// Date
  s_date_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(126, 126), bounds.size.w, 24));

	// Weather
	s_weather_layer = text_layer_create(
  	GRect(0, PBL_IF_ROUND_ELSE(150, 150), bounds.size.w, 18));
	
	// GFonts
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BARCODE_46));
	s_weather_symbol_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_WEATHERSYMBOL_64));
	
	// STYLE TEXT
	// Time
  text_layer_set_background_color(s_time_layer, GColorOrange);
  text_layer_set_text_color(s_time_layer, GColorBlack);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	
	// Weather Symbol
	text_layer_set_background_color(s_weather_symbol_layer, GColorClear);
	text_layer_set_text_color(s_weather_symbol_layer, GColorWindsorTan);	
	text_layer_set_text_alignment(s_weather_symbol_layer, GTextAlignmentCenter);
	text_layer_set_text(s_weather_symbol_layer, "k");	
	
	// Date
	text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentRight);
	
	// Weather
	text_layer_set_background_color(s_weather_layer, GColorClear);
	text_layer_set_text_color(s_weather_layer, GColorWindsorTan);	
	text_layer_set_text_alignment(s_weather_layer, GTextAlignmentRight);
	text_layer_set_text(s_weather_layer, "Lädt... ");	
	
  // Apply to TextLayer
	text_layer_set_font(s_time_layer, s_time_font);
	text_layer_set_font(s_weather_symbol_layer, s_weather_symbol_font);
	text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_weather_symbol_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_date_layer));	
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
	text_layer_destroy(s_weather_layer);
	text_layer_destroy(s_weather_symbol_layer);
	
	// Unload GFont
	fonts_unload_custom_font(s_time_font);
	//fonts_unload_custom_font(s_weather_symbol_font);
	
	// Destroy GBitmap
	gbitmap_destroy(s_background_bitmap);
	
	// Destroy BitmapLayer
	bitmap_layer_destroy(s_background_layer);	
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	// Store incoming information
	static char temperature_buffer[8];
	static char conditions_buffer[32];
	static char weather_layer_buffer[32];
	
	// Read tuples for data
	Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
	Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);

	// If all data is available, use it
	if(temp_tuple && conditions_tuple) {
  snprintf(temperature_buffer, sizeof(temperature_buffer), "%d° C", (int)temp_tuple->value->int32);
  snprintf(conditions_buffer, sizeof(conditions_buffer), "%s ", conditions_tuple->value->cstring);
	}
	
	if(temp_tuple->value->int32 <= 10) {
	text_layer_set_text_color(s_weather_layer, GColorBlue);
	} else if(temp_tuple->value->int32 >= 25) {
		text_layer_set_text_color(s_weather_layer, GColorRed);
	}
	
	// Assemble full string and display
	snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
	text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
	
	// Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	// Make sure the time is displayed from the start
	update_time();
	window_set_background_color(s_main_window, GColorWhite);
	
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	
	// Open AppMessage
	const int inbox_size = 128;
	const int outbox_size = 128;
	app_message_open(inbox_size, outbox_size);

}

static void deinit() {
	window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}