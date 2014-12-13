#include <pebble.h>
#include <string.h>
#include <stdio.h>
#include "pebble_fonts.h"
#define ACCEL_STEP_MS 50


static Window *window;
static TextLayer *display_text;
static TextLayer *connection_text;
static BitmapLayer *rewind_layer;
static BitmapLayer *image_layer;
Layer *window_layer;
static AppTimer *timer;
static char *rate_text;
GBitmap *image;
GBitmap *font_banner;
GBitmap *game_bg;
GBitmap *rewind_icon;
GBitmap *pause_icon;
GBitmap *nothing_icon;
const char* fonts[4] = {FONT_KEY_GOTHIC_28, FONT_KEY_GOTHIC_28_BOLD, FONT_KEY_ROBOTO_CONDENSED_21 };
GFont disp_font;
int font_id = 0;
enum {MENU,BOOK,SETTINGS} frame;
int text_x = -20;
int text_y = 70;
int speed = 40;
int con_timer = 0;
int hold = 5;
int push_x = 0;
bool started = false;
bool pause = true;
bool fast_rewind  = false;
bool end = false;
bool menu = true;
char* word;
int size_of_word = 256;
typedef struct Node {
	char* word;
	struct Node* next;
	struct Node* prev;
} Node;

Node* head;
Node* tail;
Node* temp;

void getNextWord(){
	char* theWord = temp->word;
	memcpy(word,theWord,strlen(theWord));
	if(temp->next){
		temp = temp->next;
	}
}
void getPreviousWord(){
	char* theWord = temp->word;
	memcpy(word,theWord,strlen(theWord));
	if(temp->prev){
		temp = temp->prev;
	}
}

void redraw_text(int type){
	if(type == 0){
  hold--;
	}
	if(hold<0 || type>0){
		GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
		for(int i=0; i<size_of_word; i++){
			word[i] = '\0';
		}
		
		if(type == 0 || type == 2){
			getNextWord();
		}else if(type == 1){
			getPreviousWord();
		}
		int wordLength=0;
		if(end){
			hold = 1000000;
		}else{
			wordLength = strlen(word);
			hold = (125/speed) + (wordLength*4/speed);
			
			
			int shift_x = 0;
			if(wordLength<=2){
				shift_x = 1;
			}else
				if(wordLength <= 5){
					shift_x = 2;
				}else if(wordLength <= 8){
					shift_x = 3;
				}else if(wordLength <= 12){
					shift_x = 4;
				}else if(wordLength <= 14){
					shift_x = 5;
				}else if(wordLength <= 16){
					shift_x = 6;
				}else if(wordLength <= 20){
					shift_x = 7;
				}
			/*
			 char crop[20] = "";
			 for(int i=0; i<shift_x; i++){
			 crop[i] = (*word)[i];
			 }*/
			
			GSize size = graphics_text_layout_get_content_size(word,disp_font,move_pos2,GTextOverflowModeTrailingEllipsis,GTextAlignmentCenter);
			int16_t size_x = size.w;
			
			push_x = size_x/2-shift_x;
			APP_LOG(APP_LOG_LEVEL_DEBUG,"word = %s push = %d",word,push_x);
			
		}
		//hold = 10;
		move_pos2 = (GRect) { .origin = { text_x+push_x-5, text_y }, .size = { 180, 180 } };
		text_layer_set_text(display_text, word);
		
		//text_layer_set_text(connection_text,*con_text);
		
		layer_set_frame(text_layer_get_layer(display_text),move_pos2);
	}
}







static void timer_callback(void *data) {
	if(!pause){
  redraw_text(0);
	}else{
		if(fast_rewind){
			redraw_text(1);
		}
	}
	
	if(con_timer>0){
		con_timer--;
		if(con_timer==49){
			layer_add_child(window_layer, text_layer_get_layer(connection_text));
		}
	}else{
		layer_remove_from_parent(text_layer_get_layer(connection_text));
	}
	
	//animation actionEvent
	timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}



static void change_to_menu(){
	frame = MENU;
	text_layer_set_text(connection_text,"Waiting for Device..");
	text_layer_set_text(display_text,"");
	
	GRect move_pos2 = (GRect) { .origin = { -15, 105 }, .size = { 180, 180 } };
	layer_set_frame(text_layer_get_layer(display_text),move_pos2);
	GRect move_pos3 = (GRect) { .origin = { -15, 130 }, .size = { 180, 180 } };
	layer_set_frame(text_layer_get_layer(connection_text),move_pos3);
	
	GRect move_pos4 = (GRect) { .origin = {-18, -15 }, .size = { 180, 180 } };
	layer_set_frame(bitmap_layer_get_layer(image_layer),move_pos4);
	
	bitmap_layer_set_bitmap(image_layer, image);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);
	
}

static void change_to_settings(){
	frame = SETTINGS;
	text_layer_set_text(connection_text,"");
	text_layer_set_text(display_text,"FONT");
	GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
	layer_set_frame(text_layer_get_layer(display_text),move_pos2);
	GRect move_pos4 = (GRect) { .origin = { -18, -80 }, .size = { 180, 180 } };
	layer_set_frame(bitmap_layer_get_layer(image_layer),move_pos4);
	
	bitmap_layer_set_compositing_mode(image_layer, GCompOpAssign);
	bitmap_layer_set_bitmap(image_layer, font_banner);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);
	
}

static void change_to_book(){
	frame = BOOK;
	started = true;
	if(font_id==0){
		text_x=-40;
		text_y=70;
	}else if(font_id ==1 ){
		text_x =-40;
		text_y=70;
	}else if(font_id==2){
		text_x =-40;
		text_y = 73;
	}
	// con_text = "WPM: 200";
	//layer_remove_from_parent(text_layer_get_layer(connection_text));
	//text_layer_set_text(connection_text,"");
	text_layer_set_text(display_text,"Starting..");
	
	GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
	layer_set_frame(text_layer_get_layer(display_text),move_pos2);
	/*
	 GRect move_pos3 = (GRect) { .origin = { -15, 130 }, .size = { 180, 180 } };
	 layer_set_frame(text_layer_get_layer(connection_text),move_pos3);
  */
	// text_layer_set_font(connection_text, disp_font);
	
	GRect move_pos4 = (GRect) { .origin = { -18, 0 }, .size = { 180, 180 } };
	layer_set_frame(bitmap_layer_get_layer(image_layer),move_pos4);
	
	bitmap_layer_set_compositing_mode(image_layer, GCompOpClear);
	
	bitmap_layer_set_bitmap(image_layer, game_bg);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);
	
	timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}



void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	if(frame == MENU){
		change_to_settings();
	}else
  if(frame == SETTINGS){
	  if(!started){
		  change_to_menu();
	  }else{
		  change_to_book();
	  }
  }
  else if(frame == BOOK){
	  if(pause){
		  //redraw_text(2);
	  }else{
		  if(speed<95){
			  speed+=5;
			  // hard_code();
			  con_timer = 50;
			  // APP_LOG(APP_LOG_LEVEL_DEBUG,"speed_text = %s",con_text);
		  }
	  }
  }
}

void middle_click_handler(ClickRecognizerRef recognizer, void *context) {
	if(frame==MENU){
		change_to_book();
	}
	if(frame == SETTINGS){
		font_id++;
		if(font_id>2){
			font_id = 0;
		}
		if(font_id==0){
			text_x=-20;
			text_y=70;
		}else if(font_id ==1 ){
			text_x =-20;
			text_y=70;
		}else if(font_id==2){
			text_x =-20;
			text_y = 73;
		}
		
		disp_font = fonts_get_system_font(fonts[font_id]);
		GRect move_pos2 = (GRect) { .origin = { text_x, text_y }, .size = { 180, 180 } };
		layer_set_frame(text_layer_get_layer(display_text),move_pos2);
		text_layer_set_font(display_text, disp_font);
	}else if(frame == BOOK){
		pause = !pause;
		if(pause == true){
			bitmap_layer_set_bitmap(rewind_layer, pause_icon);
		}else{
			bitmap_layer_set_bitmap(rewind_layer, nothing_icon);
		}
	}
}

void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	if(frame == MENU) {
		
	} else if(frame == BOOK) {
		if(pause){
			end = false;
			hold = 0;
			redraw_text(1);
			fast_rewind = true;
			// layer_add_child(window_layer, bitmap_layer_get_layer(rewind_layer));
			bitmap_layer_set_bitmap(rewind_layer, rewind_icon);
			
		} else {
			if (speed>20) {
				speed-=5;
				con_timer = 50;
				//   hard_code();
				
				//APP_LOG(APP_LOG_LEVEL_DEBUG,"speed_text = %s",*con_text);
			}
		}
	}
}


void down_up_click_handler(ClickRecognizerRef recognizer, void *context) {
	//nothing as of now
	fast_rewind = false;
	// layer_remove_from_parent(bitmap_layer_get_layer(rewind_layer));
	if(pause){
		bitmap_layer_set_bitmap(rewind_layer, pause_icon);
	}
}



void config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, middle_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
	window_long_click_subscribe(BUTTON_ID_DOWN, 50, down_click_handler, down_up_click_handler);
}

static void addWord(char *word) {
	if (!word)
		return;
	
	Node * newNode = calloc(1, sizeof(Node));
	newNode->word = word;
	tail->next = newNode;
	newNode->prev = tail;
	newNode->next = NULL;
	tail = newNode;
}

void app_message_received_handler(DictionaryIterator *iterator, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "In message received handler.");
	Tuple *wordPair = dict_read_first(iterator);
	if (!wordPair) // safety check
		return;
	do {
		// check the value's type
		if (wordPair->type != TUPLE_CSTRING) {
			APP_LOG(APP_LOG_LEVEL_ERROR, "Received non-string word.");
		} else {
			// it's a string
			uint16_t dataLength = wordPair->length;
			if (dataLength == 0)
				return; // no work to do
			// copy the string out of the dictionary and add it to the linked list
			char * receivedWord = calloc(dataLength, sizeof(char));
			strncpy(receivedWord, (const char *)wordPair->value, dataLength);
			addWord(receivedWord);
		}
	} while ((wordPair = dict_read_next(iterator)));
}

static void init() {
	window = window_create();
	window_set_fullscreen(window, true);
	window_stack_push(window, true /* Animated */);
	window_set_click_config_provider(window, config_provider);
	window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	
	display_text = text_layer_create(bounds);
	connection_text = text_layer_create(bounds);
	image_layer = bitmap_layer_create(bounds);
	rewind_layer = bitmap_layer_create(bounds);
	
	image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STELA_ICON);
	font_banner = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FONT_BANNER);
	game_bg = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GAME_PANE_BLACK);
	rewind_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_REWIND);
	pause_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);
	nothing_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NOTHING);
	
	disp_font = fonts_get_system_font(fonts[0]);
	text_layer_set_font(display_text, disp_font);
	
	bitmap_layer_set_bitmap(rewind_layer, nothing_icon);
	bitmap_layer_set_alignment(rewind_layer, GAlignCenter);
	GRect move_pos2 = (GRect) { .origin = { -20, -60 }, .size = { 180, 180 } };
	layer_set_frame(bitmap_layer_get_layer(rewind_layer),move_pos2);
	
	change_to_menu();
	
	
	text_layer_set_text_alignment(display_text, GTextAlignmentCenter);
	text_layer_set_text_alignment(connection_text, GTextAlignmentCenter);
	text_layer_set_overflow_mode(connection_text, GTextOverflowModeWordWrap);
	
	layer_add_child(window_layer, text_layer_get_layer(display_text));
	layer_add_child(window_layer, text_layer_get_layer(connection_text));
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
	layer_add_child(window_layer, bitmap_layer_get_layer(rewind_layer));
	
	//*body_text = " Using Stela, you can read articles from around the web quickly and easily, right on your wrist! Just surf to the article on your phone and hit the Stela button and you can start reading instantly from your Pebble smartwatch. You can read in the shower, and you can even turn your phone off to save that last 5% of your battery. Stela uses a simple, clean, and intuitive interface to show the text one word at a time. Not only does this allow you to read comfortably on such a small screen, but it can even help you to read faster. Words are flashed on the screen one at a time, and they are optically centered on the screen. This allows you to spend less time moving your eyes from word to word (called saccades) and more time reading the content.";
	word = calloc(1,size_of_word);
	head = calloc(1,sizeof(Node));
	tail = calloc(1,sizeof(Node));
	temp = calloc(1,sizeof(Node));
	head->word = "Begin";
	head->next = tail;
	head->prev = NULL;
	tail->word = "...";
	tail->next = NULL;
	tail->prev = head;
	temp = head;
	addWord("Paint");
	addWord("Me");
	addWord("Like");
	addWord("one");
	addWord("of");
	addWord("your");
	addWord("french");
	addWord("girls");
	
	
	app_message_register_inbox_received(app_message_received_handler);
}


static void deinit() {
	gbitmap_destroy(image);
	gbitmap_destroy(game_bg);
	gbitmap_destroy(font_banner);
	gbitmap_destroy(rewind_icon);
	gbitmap_destroy(pause_icon);
	gbitmap_destroy(nothing_icon);
	
	bitmap_layer_destroy(image_layer);
	bitmap_layer_destroy(rewind_layer);
	text_layer_destroy(display_text);
	text_layer_destroy(connection_text);
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}