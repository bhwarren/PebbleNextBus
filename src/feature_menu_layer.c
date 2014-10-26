#include "pebble.h"

#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 4

#define NUM_NEARBY_BUSES 15
#define NUM_NEARBY_MENU_SECTIONS 1
	
typedef struct {
  char* bus_route;
	char* stop;
	char* direction;
	char* arrival_time;
//	int second_arrival_time;
} BusInfo;

BusInfo nearby_buses[NUM_NEARBY_BUSES];

//function decs
void window_unload(Window *window);
void window_load(Window *window);


static Window *window;

//for testing purposes, do this the right way later
BusInfo* b_info;

// This is a menu layer
// You have more control than with a simple menu layer
Layer *window_layer;
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
		
		char title[70] = "\0";

		strcat(title, nearby_buses[arr_pos].bus_route);
		//strcat(title, " ");
		//strcat(title, nearby_buses[arr_pos].stop);
		strcat(title, "   ");
		strcat(title, nearby_buses[arr_pos].arrival_time);
		
    menu_cell_basic_draw(ctx, cell_layer, title, nearby_buses[arr_pos].stop, NULL);
  }
}

void get_buses_from_server(){

	int iterator;
	for(iterator=0;iterator<NUM_NEARBY_BUSES;iterator++){
		BusInfo* b_info = malloc(sizeof(BusInfo));
		b_info->bus_route = "JFX";
		b_info->stop = "South Columbia at Sitterson";
		b_info->direction = "to jones ferry road";
		b_info->arrival_time =	"33";
		
		nearby_buses[iterator] = *b_info;
		free(b_info);
	}
	
}

// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
	
	//need to get the bus options, then load them into the nearby_menu_layer
	//this is possible b/c it gets drawn only after it is added to the window
	get_buses_from_server();
	
	//hide the main menu layer
	layer_set_hidden((Layer *)menu_layer, true);
	
	//register the click handler with the bus menu
	menu_layer_set_click_config_onto_window(nearby_menu_layer, window);

	//show the nearby buses
	layer_add_child(window_layer, menu_layer_get_layer(nearby_menu_layer));

}

//Show all of the Bus info on the screen
void load_bus_info(int index){
	
	BusInfo* b_info = &(nearby_buses[index]);
	
	//hide nearby menu list
	layer_set_hidden((Layer *)nearby_menu_layer, true);

	route_text_layer = text_layer_create(GRect(0,0,144,50));
	text_layer_set_background_color(route_text_layer, GColorClear);
  text_layer_set_text_color(route_text_layer, GColorBlack);
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
	arrival_text_layer = text_layer_create(GRect(0, 90, 144, 20));
	text_layer_set_font(arrival_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text(arrival_text_layer, b_info->arrival_time);																							 
		
	
	//add all layers to screen
	layer_add_child(window_layer, text_layer_get_layer(route_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(stop_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(direction_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(arrival_text_layer));
//	layer_add_child(window_layer, text_layer_get_layer(second_arrival_text_layer));

}

void nearby_menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
	load_bus_info(cell_index->row);
}

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
  nearby_menu_layer = menu_layer_create(bounds);
	

  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });
	menu_layer_set_callbacks(nearby_menu_layer, NULL, (MenuLayerCallbacks){
		.get_num_sections = nearby_menu_get_num_sections_callback,
    .get_num_rows = nearby_menu_get_num_rows_callback,
    .get_header_height = nearby_menu_get_header_height_callback,
    .draw_header = nearby_menu_draw_header_callback,
    .draw_row = nearby_menu_draw_row_callback,
    .select_click = nearby_menu_select_callback,
	});

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(menu_layer, window);
	

  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

void window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(menu_layer);
	menu_layer_destroy(nearby_menu_layer);
	
	text_layer_destroy(route_text_layer);
	text_layer_destroy(stop_text_layer);
	text_layer_destroy(direction_text_layer);
	text_layer_destroy(arrival_text_layer);
	//text_layer_destroy(second_arrival_text_layer);
}


int main(void) {
  window = window_create();

  // Setup the window handlers
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(window, true);

  app_event_loop();

  window_destroy(window);
}
