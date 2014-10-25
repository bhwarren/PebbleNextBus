#include "pebble.h"

#define NUM_MENU_SECTIONS 2
#define NUM_FIRST_MENU_ITEMS 4
#define NUM_SECOND_MENU_ITEMS 1

#define NUM_NEARBY_BUSES 5
#define NUM_NEARBY_MENU_SECTIONS 1

static Window *window;

// This is a menu layer
// You have more control than with a simple menu layer
Layer *window_layer;
static MenuLayer *menu_layer;
static MenuLayer *nearby_menu_layer;


// You can draw arbitrary things in a menu item such as a background
static GBitmap *menu_background;

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
	switch (section_index) {
		case 0:
			return NUM_FIRST_MENU_ITEMS;

		case 1:
			return NUM_SECOND_MENU_ITEMS;

		default:
			return 0;
  }
}

static uint16_t nearby_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	switch(section_index){
		case 0:
			return NUM_NEARBY_BUSES;
		//only allow one "section"
		default:
			return 0;
	}
}



// A callback is used to specify the height of the section header
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	// This is a define provided in pebble.h that you may use for
	// the default height
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t nearby_menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

// Here we draw what each header is
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
	// Determine which section we're working with
	switch (section_index) {
		case 0:
			// Draw title text in the section header
			menu_cell_basic_header_draw(ctx, cell_layer, "PebbleNextbus");
			break;
	}
}

static void nearby_menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
	// Determine which section we're working with
	switch (section_index) {
		case 0:
			// Draw title text in the section header
			menu_cell_basic_header_draw(ctx, cell_layer, "Nearby Buses");
			break;
		default:
			break;
	}
}

// This is the menu item draw callback where you
// specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	// Determine which section we're going to draw in
	switch (cell_index->section) {
		case 0:
			// Use the row to specify which item we'll draw
			switch (cell_index->row) {
				case 0:
					// This is a basic menu item with a title and subtitle - ""
					menu_cell_basic_draw(ctx,
					                     cell_layer,
					                     "Nearby Buses", "", NULL);
					break;
				case 1:
					menu_cell_basic_draw(ctx,
					                     cell_layer,
					                     "Select Stop",
					                     "From a predefined list", NULL);
					break;
				case 2:
					menu_cell_basic_draw(ctx,
					                     cell_layer,
					                     "Favorite Stops", "", NULL);
					break;
				case 3:
					menu_cell_basic_draw(ctx,
					                     cell_layer,
					                     "Information", "", NULL);
					break;
			}
			break;
		default:
			break;
  }
}

static void nearby_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	// Determine which section we're going to draw in
	switch (cell_index->section) {
		case 0:
			// Use the row to specify which item we'll draw
			switch (cell_index->row) {
				case 0:
					// This is a basic menu item with a title and subtitle - ""
					menu_cell_basic_draw(ctx,
					                     cell_layer,
					                     "A Longview 33min",
					                     "Towards Family Practice", NULL);
					break;
				case 1:
					menu_cell_basic_draw(ctx,
					                     cell_layer,
					                     "N Longview 10min",
					                     "Towards Town Hall", NULL);
					break;
				case 2:
					menu_cell_basic_draw(ctx,
					                     cell_layer,
					                     "JFX Sitterson 2min",
					                     "Towards Jones Ferry Rd", NULL);
					break;
			}
			break;

		case 1:
			switch (cell_index->row) {
				default:
					break;
			}
	}
}


void load_nearby_menu(){
	menu_layer_set_click_config_onto_window(nearby_menu_layer, window);
	layer_add_child(window_layer, menu_layer_get_layer(nearby_menu_layer));
}

// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	// Use the row to specify which item will receive the select action
	switch (cell_index->row) {
		case 0:
			//make a new menu with list of buses close by
			load_nearby_menu();
			break;
		case 1:
			//make a new menu of stops available nearby
			break;
		case 2:
			//make a new menu to choose favorite stops already saved
			break;
		case 3:
			//show a text info screen
			break;
	}
}

void load_bus_info(int row){


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

	// Bind the menu layer's click config provider
	// to the window for interactivity
	menu_layer_set_click_config_onto_window(menu_layer, window);


	// Add it to the window for display
	layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

void window_unload(Window *window) {
	// Destroy the menu layer
	menu_layer_destroy(menu_layer);

	// And cleanup the background
	gbitmap_destroy(menu_background);
}

int main(void) {
	window = window_create();

	// Setup the window handlers
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});

	window_stack_push(window, true /* Animated */);

	app_event_loop();

	window_destroy(window);
}
