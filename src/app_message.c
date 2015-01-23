#include "window_loaders.h"	
	
	
//temp var to store messages from the phone
char* msg;

// Key values for AppMessage Dictionary
enum {
	STATUS_KEY = 0,
	DATA_KEY,
	DEBUG_KEY
};


//-----------------------Message Callbacks ----------------------------------


void get_buses(){ 


	
	
	if(num_nearby_buses>0){
		free_bus_info(num_nearby_buses);
  }


	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	
	//hi is the only thing that works????
	dict_write_cstring(iter, 1, "hif");
	
	
	dict_write_end(iter);
  app_message_outbox_send();
	APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", "sent buses request, waiting for reply");
		
	//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", "Start querying buses");
	got_all_buses = false;
	num_nearby_buses = 0;
	
}

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

void add_to_routes(char* msg){
	
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "before adding route: %s\n", msg);

	
	int last_nl = 0;
	int section = 0;
	int i;
	for(i=0;i<(int)strlen(msg); i++){
		
		//when find a newline, grab stuff between nls and put it in the array
		if(msg[i]!='\n'){
			continue;
		}
		
		switch(section){
			//define route section
			case 0:
				nearby_buses[num_nearby_buses].bus_route = malloc((i-last_nl+1)*sizeof(char));
				strncpy(nearby_buses[num_nearby_buses].bus_route, msg+last_nl, i-last_nl);
				nearby_buses[num_nearby_buses].bus_route[(i-last_nl)] = '\0';
				break;
			//define direction section
			case 1:
				nearby_buses[num_nearby_buses].direction = malloc((i-last_nl+1)*sizeof(char));
				strncpy(nearby_buses[num_nearby_buses].direction, msg+last_nl+1, i-last_nl);
				nearby_buses[num_nearby_buses].direction[(i-last_nl)] = '\0';
				break;
			//define stop section
			case 2:
				nearby_buses[num_nearby_buses].stop = malloc((i-last_nl+1)*sizeof(char));
				strncpy(nearby_buses[num_nearby_buses].stop, msg+last_nl+1, i-last_nl);
				nearby_buses[num_nearby_buses].stop[(i-last_nl)] = '\0';
				break;
			//define arrival time section
			case 3:
				nearby_buses[num_nearby_buses].arrival_time = malloc((i-last_nl+1)*sizeof(char));
				strncpy(nearby_buses[num_nearby_buses].arrival_time, msg+last_nl+1, i-last_nl);
				nearby_buses[num_nearby_buses].arrival_time[(i-last_nl)] = '\0';
				break;

			
		}
		
		section++;
		last_nl=i;		
	}
	
	
	int str_sz = strlen(nearby_buses[num_nearby_buses].arrival_time) + 
								strlen(nearby_buses[num_nearby_buses].bus_route) + 5;
	
	nearby_buses[num_nearby_buses].title = malloc(sizeof(char)*str_sz);	
					//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n",nearby_buses[arr_pos].bus_route );
		
	strcpy(nearby_buses[num_nearby_buses].title, nearby_buses[num_nearby_buses].bus_route);
	strcat(nearby_buses[num_nearby_buses].title, "   ");
					//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n",nearby_buses[arr_pos].arrival_time);
	strcat(nearby_buses[num_nearby_buses].title, nearby_buses[num_nearby_buses].arrival_time);

	
	num_nearby_buses++;

			//APP_LOG(APP_LOG_LEVEL_DEBUG, "after adding route: %s\n", msg);

}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
	if(got_all_buses){
		send_ack_message();
		return;
	}
	
	Tuple* rec_debug = dict_find(received, DEBUG_KEY);
	Tuple* rec_data = dict_find(received, DATA_KEY);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", "got msg");
	

	
	//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", "haven't gotten all buses yet");
	if (rec_debug) {
			//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", "starting to copy recieved data");

		msg = malloc(strlen(rec_debug->value->cstring)*sizeof(char));
		strcpy(msg, rec_debug->value->cstring);
		
	} else if (rec_data) {
					//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", "starting to copy debug data");

		msg = malloc(strlen(rec_data->value->cstring)*sizeof(char));
		strcpy(msg, rec_data->value->cstring);
	}
	else{
		send_ack_message();
		return;
	}
				//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", "starting to print recieved/debug data");
	
	
	//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", msg);
	
	/*
	if(strcmp(msg, "done")==0){
		free(msg);
		got_all_buses = true;
		send_ack_message();
		
		//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", "i'm done.  setting layer to hidden");
		
		//remove the loading screen from user's view
		if(layer_get_hidden(text_layer_get_layer(loading_text_layer))==false
												&& window_stack_contains_window(nearby_buses_window)){
			layer_set_hidden(text_layer_get_layer(loading_text_layer), true);
			menu_layer_reload_data(nearby_menu_layer);
		}
		
		//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", "done setting layer to hidden");

		return;
	}*/
	//else 
	if(strcmp(msg,"Error connecting to server.")==0){
		send_ack_message();
		return;
	}
	

	//APP_LOG(APP_LOG_LEVEL_DEBUG, "%s\n", "before splitting on |");
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "msg%s\n", msg);

	
	int last_vert = 0;
	int i;
	for(i=0; i < (int)strlen(msg); i++){
		
		//when find a new vert bar, grab stuff inbetween and put it in the bus array
		//or if is first char, just ignore the bar, since nothing to copy 
		if(msg[i] != '|' || i==0){
			continue;
		} 
		
		char* one_msg = malloc((i-last_vert+1)*sizeof(char));
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "before copying: %s\n", one_msg);

		
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "numchars:%d\n", i-last_vert);

		strncpy(one_msg, msg+last_vert+1, i-last_vert);

		//APP_LOG(APP_LOG_LEVEL_DEBUG, "onemsg: %s\n", one_msg);

		one_msg[i-last_vert]='\0';
 
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "before adding: %s\n", one_msg);
		add_to_routes(one_msg);
		
		free(one_msg);
		
		last_vert = i;

		
	}
	
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Finished adding all routes %s\n", "f");

	got_all_buses = true;
		
		//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", "i'm done.  setting layer to hidden");
		
		//remove the loading screen from user's view
	if(layer_get_hidden(text_layer_get_layer(loading_text_layer))==false
											&& window_stack_contains_window(nearby_buses_window)){
		layer_set_hidden(text_layer_get_layer(loading_text_layer), true);
		menu_layer_reload_data(nearby_menu_layer);
	}
				
	
	
	free(msg);
	APP_LOG(APP_LOG_LEVEL_DEBUG,"%s%d\n", "after getting msg. num busses gotten: ",num_nearby_buses);
	
	//layer_add_child(window_layer, text_layer_get_layer(text_layer));
	send_ack_message();
	

	
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {
	send_nack_message();
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	get_buses();
	send_nack_message();
}





//----------------Startup and main/managerial stuff----------------------


void init(void) {

	got_all_buses = false;
	num_nearby_buses = 0;
	
	nearby_buses = malloc(sizeof(BusInfo)*MAX_SUPPORTED_BUSES);

	window = window_create();
	nearby_buses_window = window_create();
	bus_info_window = window_create();
	info_window = window_create();
	
	
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
	
	// initialize the TextLayer so can be referenced on multiple windows
	window_layer = window_get_root_layer(window);
	bounds = layer_get_frame(window_layer);

	//text_layer = text_layer_create((GRect){ .origin = { 0, 30 }, .size = bounds.size });
	nearby_menu_layer = menu_layer_create(bounds);
	loading_text_layer = text_layer_create(GRect(0, 50, 144, 20));


	
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
		
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	send_ack_message();

}

void deinit(void) {
	
	
	//check that we don't need to free individuals inside
			//APP_LOG(APP_LOG_LEVEL_DEBUG,"%s\n", "In de-init, after free");

	
	app_message_deregister_callbacks();
	
	//text_layer_destroy(text_layer);
	
	window_destroy(bus_info_window);
	window_destroy(nearby_buses_window);
	window_destroy(window);
	window_destroy(info_window);
	
	//if it is initialized, kill it
	if(nearby_menu_layer){
		menu_layer_destroy(nearby_menu_layer);
	}

	
	free_bus_info(num_nearby_buses);
	free(nearby_buses);
	
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}