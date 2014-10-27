#include "pebble.h"

#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 4

#define NUM_NEARBY_BUSES 5
#define NUM_NEARBY_MENU_SECTIONS 1
	
typedef struct {
  char* bus_route;
	char* stop;
	char* direction;
	char* arrival_time;
} BusInfo;

BusInfo nearby_buses[NUM_NEARBY_BUSES];


static char* js_msg_recieved = "hold on..."; // default message on load

static char r_nearby_datum[160];
	
// Key values for AppMessage Dictionary
enum {
	STATUS_KEY = 0,
	DATA_KEY,
	DEBUG_KEY
};

//iterator used for populating the nearby_buses array
int current_bus = 0;

//function decs
void window_unload(Window *window);
void window_load(Window *window);

//windows that will be pushed onto the stack for UI
static Window *window;
static Window* nearby_buses_window;
static Window* bus_info_window;

//window layers to place UI stuff on
Layer *window_layer;
Layer* nearby_buses_window_layer;
Layer* bus_info_window_layer;

//--the UI stuff that the user actually interacts with--
static MenuLayer *menu_layer;
static MenuLayer *nearby_menu_layer;

//all text layers used for displaying 
static TextLayer* route_text_layer;
static TextLayer* stop_text_layer;
static TextLayer* direction_text_layer;
static TextLayer* arrival_text_layer;
static TextLayer* direction_text_layer;
static TextLayer* arrival_text_layer;
//static TextLayer* second_arrival_text_layer;


//--Endu UI stuff--


//test text layer - DELETE
static TextLayer* server_stuff;


//-----------------Function Callbacks-----------------


// A callback is used to specify the amount of sections of menu items
// With this, you can dynamically add and remove sections
static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}
static uint16_t nearby_menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_NEARBY_MENU_SECTIONS;
}

// Each section has a number of items;  we use a callback to specify this
// You can also dynamically add and remove items using this
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  if (section_index == 0) {
      return NUM_FIRST_MENU_ITEMS;
  }
	return 0;
}

static uint16_t nearby_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	if (section_index == 0){
		return NUM_NEARBY_BUSES;
	}
	return 0;
}



// A callback is used to specify the height of the section header
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t nearby_menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

// Here we draw what each header is
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  if (section_index == 0) {
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "PebbleNextbus");
  }
}

static void nearby_menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  if(section_index == 0) {
      menu_cell_basic_header_draw(ctx, cell_layer, "Nearby Buses");
  }
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  if(cell_index->section == 0) {
      // Use the row to specify which item we'll draw
      switch (cell_index->row) {
        case 0:
          // This is a basic menu item with a title and subtitle - ""
          menu_cell_basic_draw(ctx, cell_layer, "Nearby Buses", "", NULL);
          break;
				case 1:
		  		menu_cell_basic_draw(ctx, cell_layer, "Select Stop", "From a predefined list", NULL);
		  		break;
				case 2:
		  		menu_cell_basic_draw(ctx, cell_layer, "Favorite Stops", "", NULL);
		  		break;
				case 3:
		  		menu_cell_basic_draw(ctx, cell_layer, "Information", "", NULL);
		  		break;
			}
  }
}

static void nearby_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	
	// Determine which section we're going to draw in
  if (cell_index->section == 0) {
    
		// Use the row to specify which item we'll draw
		int arr_pos = cell_index->row;
		
		
		//if bus was not init, then don't do anything
		if(strcmp(nearby_buses[arr_pos].bus_route,"")==0){
			return;
		}
		
		char title[70];// = malloc(strlen(nearby_buses[arr_pos].bus_route)*2);
		
		strcat(title, nearby_buses[arr_pos].bus_route);
		//strcat(title, " ");
		//strcat(title, nearby_buses[arr_pos].stop);
		strcat(title, "   ");
		strcat(title, nearby_buses[arr_pos].arrival_time);

    menu_cell_basic_draw(ctx, cell_layer, title, nearby_buses[arr_pos].stop, NULL);
//		free(title);

  }
}


// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	
  // Use the row to specify which item will receive the select action
	
	//need to get the bus options, then load them into the nearby_menu_layer
	//this is possible b/c it gets drawn only after it is added to the window
	//get_buses_from_server();

	
	//hide the main menu layer
	layer_set_hidden((Layer *)menu_layer, true);

	//show the nearby buses
	window_stack_push(nearby_buses_window, true);
}

//Show all of the Bus info on the screen
void load_bus_info(int index){
	
	BusInfo* b_info = &(nearby_buses[index]);
	
	//if the bus wasn't set, then do nothing
	if(strcmp(b_info->bus_route,"") == 0){
		return;
	}
	
	//hide nearby menu list
	layer_set_hidden((Layer *)nearby_menu_layer, true);
	
		route_text_layer = text_layer_create(GRect(0,0,144,50));
//	text_layer_set_background_color(route_text_layer, GColorClear);
//  text_layer_set_text_color(route_text_layer, GColorBlack);
	text_layer_set_text_alignment(route_text_layer, GTextAlignmentCenter);
	
	//route layer
	text_layer_set_font(route_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
	text_layer_set_text(route_text_layer, b_info->bus_route);
	
	//stop layer
	stop_text_layer = text_layer_create(GRect(0, 50, 144, 20));
	text_layer_set_text(stop_text_layer, b_info->stop);
					
	//direction layer
	direction_text_layer = text_layer_create(GRect(0, 70, 144, 20));
	text_layer_set_text(direction_text_layer, b_info->direction);
	
	//arrival time
	arrival_text_layer = text_layer_create(GRect(0, 90, 144, 70));
	text_layer_set_font(arrival_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(arrival_text_layer, GTextAlignmentCenter);
	text_layer_set_text(arrival_text_layer, b_info->arrival_time);																							 
		
	
	
	//push bus_info_window on stack
	window_stack_push(bus_info_window, true);
}

void nearby_menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
	load_bus_info(cell_index->row);
}



//-------------------End Callbacks-------------------------


//this should be called when the user selectes "nearby buses"
void get_buses_from_server(){

	int iterator;
	for(iterator=0;iterator<NUM_NEARBY_BUSES;iterator++){
		BusInfo* b_info = malloc(sizeof(BusInfo));

		switch(iterator){
			case 0:
				b_info->bus_route = "A";
				b_info->stop = "Longview";
				b_info->direction = "to family practice";
				b_info->arrival_time = "15 min";
				break;
			case 1:
				b_info->bus_route = "N";
				b_info->stop = "Town hall";
				b_info->direction = "Northside";
				b_info->arrival_time = "30 min";
				break;
			case 2:
				b_info->bus_route = "JFX";
				b_info->stop = "Sitterson hall";
				b_info->direction = "to jones ferry road";
				b_info->arrival_time = "2 min";
				break;
			case 3:
				b_info->bus_route = "U";
				b_info->stop = "Parker and South Road";
				b_info->direction = "to unc hospitals";
				b_info->arrival_time = "8 min";
				break;
			case 4:
				b_info->bus_route = "RU";
				b_info->stop = "student stores";
				b_info->direction = "to Franklin street";
				b_info->arrival_time = "7 min";
				break;
			default:
				b_info->bus_route = "";
				b_info->stop = "";
				b_info->direction = "";
				b_info->arrival_time = "";
				break;
		}
			
				
		
			nearby_buses[iterator] = *b_info;
			free(b_info);
		
	}
	
}



//-------------------Window loading and unloading-------------------------

// This initializes the menu upon window load
void window_load(Window *window) {
  // Here we load the bitmap assets
  // resource_init_current_app must be called before all asset loading
	
  // Now we prepare to initialize the menu layer
  // We need the bounds to specify the menu layer's viewport size
  // In this case, it'll be the same as the window's
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  menu_layer = menu_layer_create(bounds);
	server_stuff = text_layer_create(bounds);
	

  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });


  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(menu_layer, window);

  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
	//layer_add_child(window_layer, text_layer_get_layer(server_stuff));
}


void nearby_buses_window_load(Window *window) {
	nearby_buses_window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(nearby_buses_window_layer);
	
	nearby_menu_layer = menu_layer_create(bounds);
	
	menu_layer_set_callbacks(nearby_menu_layer, NULL, (MenuLayerCallbacks){
		.get_num_sections = nearby_menu_get_num_sections_callback,
    .get_num_rows = nearby_menu_get_num_rows_callback,
    .get_header_height = nearby_menu_get_header_height_callback,
    .draw_header = nearby_menu_draw_header_callback,
    .draw_row = nearby_menu_draw_row_callback,
    .select_click = nearby_menu_select_callback,
	});
	
	// Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(nearby_menu_layer, window);

  // Add it to the window for display
  layer_add_child(nearby_buses_window_layer, menu_layer_get_layer(nearby_menu_layer));
	
}


	
void bus_info_window_load(Window *window) {
	bus_info_window_layer = window_get_root_layer(window);
	
	
	//add all layers to screen
	layer_add_child(bus_info_window_layer, text_layer_get_layer(route_text_layer));
	layer_add_child(bus_info_window_layer, text_layer_get_layer(stop_text_layer));
	layer_add_child(bus_info_window_layer, text_layer_get_layer(direction_text_layer));
	layer_add_child(bus_info_window_layer, text_layer_get_layer(arrival_text_layer));
//	layer_add_child(window_layer, text_layer_get_layer(second_arrival_text_layer));	
}


void window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(menu_layer);
	text_layer_destroy(server_stuff);
}

void nearby_buses_window_unload(){
	menu_layer_destroy(nearby_menu_layer);
	//layer_mark_dirty(menu_layer_get_layer(menu_layer));
		
	window_stack_pop(window);
	window_stack_push(window, true);
}

void bus_info_window_unload(){
	text_layer_destroy(route_text_layer);
	text_layer_destroy(stop_text_layer);
	text_layer_destroy(direction_text_layer);
	text_layer_destroy(arrival_text_layer);
	//text_layer_destroy(second_arrival_text_layer);
	
	//layer_mark_dirty(menu_layer_get_layer(nearby_menu_layer));
	
	window_stack_pop(nearby_buses_window);
	window_stack_push(nearby_buses_window, true);
	
}


//-------------------End window loading/unloading-----------------------


//-------------------Message passing to phone-------------------------

// Write message to buffer & send
void send_ack_message(void){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, STATUS_KEY, 0x1);
	
	dict_write_end(iter);
  app_message_outbox_send();
}

// Write message to buffer & send
void send_nack_message(void){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_uint8(iter, STATUS_KEY, 0x2);
	
	dict_write_end(iter);
  	app_message_outbox_send();
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *rec_debug = dict_find(received, DEBUG_KEY);
	Tuple *rec_data = dict_find(received, DATA_KEY);
	/*
	if(current_bus>NUM_NEARBY_BUSES){
			send_ack_message();
			return;
	}
	char* result;
	if (rec_debug) {
		result = malloc(strlen(rec_debug->value->cstring));
		text_layer_set_text(server_stuff, rec_debug->value->cstring);
		strcpy(result,rec_debug->value->cstring);
		
		layer_mark_dirty(window_layer);
		
	} else if (rec_data) {
		result = malloc(strlen(rec_data->value->cstring));
		text_layer_set_text(server_stuff, rec_data->value->cstring);
		layer_mark_dirty(window_layer);
		strcpy(result,rec_data->value->cstring);
	}
	else{
		result = " ";
	}
	
	strcpy(nearby_buses[current_bus].bus_route,result);
	nearby_buses[current_bus].arrival_time = "X";
	nearby_buses[current_bus].stop = "X";
	nearby_buses[current_bus].direction = "X";
	
	free(result);
	current_bus++;
	
	*/
	send_ack_message();
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {
	send_nack_message();
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	send_nack_message();
}


//-------------------------End message handling-------------------------------

int main(void) {
	get_buses_from_server();
  window = window_create();
	nearby_buses_window = window_create();
	bus_info_window = window_create();

  // Setup the window handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
	window_set_window_handlers(nearby_buses_window, (WindowHandlers){
		.load = nearby_buses_window_load,
		.unload = nearby_buses_window_unload,
	});
	window_set_window_handlers(bus_info_window, (WindowHandlers){
		.load = bus_info_window_load,
		.unload = bus_info_window_unload,
	});

  window_stack_push(window, true);
	
	
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
	
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	send_ack_message();
	

  app_event_loop();

  window_destroy(window);
	window_destroy(nearby_buses_window);
	window_destroy(bus_info_window);
	
	app_message_deregister_callbacks();
}
