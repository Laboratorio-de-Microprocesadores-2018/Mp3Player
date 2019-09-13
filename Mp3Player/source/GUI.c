/**
 * @file GUI.c
 *
 */

 /*
 * -------------------------------------------
 * Create a mp3 graphic user interface
 * -------------------------------------------
 *	The best way to understand how it works is to run it and follow the code
 *
 */

 /*********************
 *      INCLUDES
 *********************/
#include "GUI.h"
#include "TimeManager.h"

 /*********************
 *      DEFINES
 *********************/

 /**********************
 *      TYPEDEFS
 **********************/
typedef enum {HOME_ID = 0, SEARCH_ID, LIBRARY_ID} tab_id_t;

 /**********************
 *  STATIC PROTOTYPES
 **********************/

static void GUI_CreateGUI(void);

static void GUI_CreateHeader(void);

static void GUI_CreateTabView(void);
static void set_tab_view_style(lv_obj_t * tab_view);

static void create_library_tab(void);
static void set_list_style(lv_obj_t * list);

static void style_mod_cb(lv_style_t *focus_style);

static void library_focus_cb(lv_group_t * my_group);
static void indev_feedback(lv_indev_t * indev, lv_signal_t signal);

static lv_res_t list_btn_action(lv_obj_t * obj);

//static lv_res_t gui_btn_click_action(lv_obj_t * btn);
//static lv_res_t gui_go_to_menu_action(lv_obj_t * btn);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * guiScreen, * gui_cont, * tab_view;
static lv_obj_t * home_tab, * search_tab, * library_tab;

static lv_group_t * tab_group;
static tab_id_t curr_tab_id = 0;
static uint16_t id_count = 0;

static lv_group_t * library_group;
lv_obj_t * headerTimeTxt;

lv_indev_t * gui_indev = NULL;

lv_theme_t * th;

/**********************
*      MACROS
**********************/

/**********************
*   GLOBAL FUNCTIONS
**********************/
/**
* Initialize the GUI
*/
void GUI_Init(void(*hal_init)(lv_indev_t ** indev)) {

	/*Initialize LittlevGL*/
	lv_init();

	(*hal_init)(&gui_indev);

	//lv_tutorial_objects();
	GUI_CreateGUI();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
 /**
 * Create a mp3 graphic user interface
 */
static void GUI_CreateGUI(void)
{
	/*Create theme for the GUI*/
	th = lv_theme_night_init(250, NULL);

	/*Create the screen to be used*/
	guiScreen = lv_scr_act();

	/*Create header*/
	GUI_CreateHeader();

	/*Create tab view*/
	GUI_CreateTabView();
}

static void GUI_CreateHeader(void) {

	lv_obj_t * guiHeader = lv_cont_create(guiScreen, NULL);
	lv_obj_set_size(guiHeader, GUI_TOP_HEADER_HOR_SIZE, GUI_TOP_HEADER_VER_SIZE);

	lv_obj_align(guiHeader, guiScreen, LV_ALIGN_IN_TOP_MID, 0, 0);      /*Align the container*/

	th->cont->body.radius = 0;
	lv_cont_set_style(guiHeader, th->cont);

	/*Add a time and battery to the container*/
	headerTimeTxt = lv_label_create(guiHeader, NULL);
	lv_label_set_style(headerTimeTxt, th->label.prim);

	TM_date a = TM_GetDate();
	char buffer[20];
	char time_string[50];
	uitoa(a.hour, time_string, 10);
	strcat(time_string, ":"); strcat(time_string, uitoa(a.minute, buffer, 10)); strcat(time_string, ":"); strcat(time_string, uitoa(a.second, buffer, 10));

	lv_label_set_text(headerTimeTxt, time_string);

	lv_obj_t * headerBatteryTxt = lv_label_create(guiHeader, headerTimeTxt);
	lv_label_set_text(headerBatteryTxt, SYMBOL_BATTERY_FULL);

	lv_obj_align(headerTimeTxt, guiHeader, LV_ALIGN_CENTER, 0, 0);      /*Align time text*/
	lv_obj_align(headerBatteryTxt, guiHeader, LV_ALIGN_IN_RIGHT_MID, -12, 0);      /*Align battery text*/
}

void update_header(void){

	TM_date a = TM_GetDate();
	char buffer[20];
	char time_string[50];
	uitoa(a.hour, time_string, 10);
	strcat(time_string, ":"); strcat(time_string, uitoa(a.minute, buffer, 10)); strcat(time_string, ":"); strcat(time_string, uitoa(a.second, buffer, 10));

	lv_label_set_text(headerTimeTxt, time_string);
}

static void GUI_CreateTabView(void) {

	/*Create the tab view for the different sections of the GUI*/

	tab_view = lv_tabview_create(guiScreen, NULL);

	lv_coord_t gui_screen_width = guiScreen->coords.x2 - guiScreen->coords.x1 + 1;
	lv_coord_t gui_screen_height = guiScreen->coords.y2 - guiScreen->coords.y1 + 1;
	lv_obj_set_size(tab_view, gui_screen_width, 9 * gui_screen_height / 10);

	lv_obj_align(tab_view, guiScreen, LV_ALIGN_IN_BOTTOM_MID, 0, 0);	/*Align the tab view*/

	set_tab_view_style(tab_view);
	lv_tabview_set_btns_pos(tab_view, LV_TABVIEW_BTNS_POS_BOTTOM);



	home_tab = lv_tabview_add_tab(tab_view, "  " SYMBOL_HOME "\nHome"); id_count++;
	search_tab = lv_tabview_add_tab(tab_view, "  " SYMBOL_LIST "\nSearch"); id_count++;
	library_tab = lv_tabview_add_tab(tab_view, "  " SYMBOL_DIRECTORY "\nLibrary"); id_count++;

	/*Create an object group*/
	tab_group = lv_group_create();
	lv_group_set_style_mod_cb(tab_group, style_mod_cb);

	/*Adding the input device(s) to the created group*/
	if (gui_indev) lv_indev_set_group(gui_indev, tab_group);
	lv_indev_set_feedback(gui_indev, indev_feedback);

	lv_group_add_obj(tab_group, home_tab);
	lv_group_add_obj(tab_group, search_tab);
	lv_group_add_obj(tab_group, library_tab);

	/*th->page.bg->body.main_color = LV_COLOR_MAKE(0x10, 0x10, 0x10);
	th->page.bg->body.grad_color = LV_COLOR_MAKE(0x10, 0x10, 0x10);*/

	/*Configure different styles*/
	lv_page_set_style(home_tab, LV_PAGE_STYLE_BG, th->page.bg);
	lv_page_set_style(search_tab, LV_PAGE_STYLE_BG, th->page.bg);
	lv_page_set_style(library_tab, LV_PAGE_STYLE_BG, th->page.bg);

	create_library_tab();

}
static void set_tab_view_style(lv_obj_t * tab_view) {

	/*tabview BG*/
	/*th->tabview.bg->body.main_color = LV_COLOR_WHITE;
	th->tabview.bg->body.grad_color = LV_COLOR_WHITE;
	lv_tabview_set_style(tab_view, LV_TABVIEW_STYLE_BG, th->tabview.bg);*/

	/*tabview INDIC*/

	/*tabview BTN_BG*/
	/*th->tabview.btn.bg->body.main_color = LV_COLOR_BLACK;
	th->tabview.btn.bg->body.grad_color = LV_COLOR_BLACK;*/

	/*tabview BTN_REL*/
	//th->tabview.btn.rel->body.main_color = LV_COLOR_YELLOW;

	/*tabview BTN_PR*/
	/*th->tabview.btn.pr->body.main_color = LV_COLOR_ORANGE;
	th->tabview.btn.pr->body.grad_color = LV_COLOR_ORANGE;
	th->tabview.btn.pr->body.opa = 255;*/

	/*tabview BTN_TGL_REL*/
	//th->tabview.btn.tgl_rel->body.main_color = LV_COLOR_GREEN;

	/*tabview BTN_TGL_PR*/
	//th->tabview.btn.tgl_pr->body.main_color = LV_COLOR_ORANGE;


	lv_tabview_set_style(tab_view, LV_TABVIEW_STYLE_BG, th->tabview.bg);
	lv_tabview_set_style(tab_view, LV_TABVIEW_STYLE_INDIC, th->tabview.indic);
	lv_tabview_set_style(tab_view, LV_TABVIEW_STYLE_BTN_BG, th->tabview.btn.bg);
	lv_tabview_set_style(tab_view, LV_TABVIEW_STYLE_BTN_REL, th->tabview.btn.rel);
	lv_tabview_set_style(tab_view, LV_TABVIEW_STYLE_BTN_PR, th->tabview.btn.pr);
	lv_tabview_set_style(tab_view, LV_TABVIEW_STYLE_BTN_TGL_REL, th->tabview.btn.tgl_rel);
	lv_tabview_set_style(tab_view, LV_TABVIEW_STYLE_BTN_TGL_PR, th->tabview.btn.tgl_pr);
}

static void create_library_tab(void) {
	/*Create an object group*/
	library_group = lv_group_create();
	lv_group_set_focus_cb(library_group, library_focus_cb);
	//lv_group_set_style_mod_cb(tab_group, style_mod_cb);



	/***********************
	 * ADD A SONG LIST
	 ************************/
	lv_obj_t * library_list = lv_list_create(library_tab, NULL);            /*Create a drop down list*/

	lv_coord_t tab_width = library_tab->coords.x2 - library_tab->coords.x1 + 1;
	lv_coord_t tab_height = library_tab->coords.y2 - library_tab->coords.y1 + 1;
	lv_obj_set_size(library_list, lv_page_get_fit_width(library_tab), lv_page_get_fit_height(library_tab));
	//lv_obj_set_size(library_list, tab_width, tab_height);
	//lv_obj_set_height(library_list, tab_height);

	//lv_obj_align(library_list, library_tab, LV_ALIGN_CENTER, 0, 0);         /*Align next to the slider*/
	lv_obj_align(library_list, NULL, LV_ALIGN_CENTER, 0, 0);

	set_list_style(library_list);

	char songs[20][20];
	uint8_t songs_count = sizeof(songs)/ sizeof(songs[0]);

	for (int i = 0; i < songs_count; i++) {
		sprintf(songs[i], "song %d", i);

		lv_obj_t * song;
		song = lv_list_add(library_list, NULL, songs[i], list_btn_action);
		lv_group_add_obj(library_group, song);

	}

	//lv_obj_set_free_ptr(ddlist, slider);                                   /*Save the pointer of the slider in the ddlist (used in 'ddlist_action()')*/
	//lv_obj_set_top(library_list, true);                                        /*Enable to be on the top when clicked*/
}
static void set_list_style(lv_obj_t * list) {
	lv_list_set_style(list, LV_LIST_STYLE_BG, th->list.bg);
	lv_list_set_style(list, LV_LIST_STYLE_SCRL, th->list.scrl);
	lv_list_set_style(list, LV_LIST_STYLE_SB, th->list.sb);
	lv_list_set_style(list, LV_LIST_STYLE_EDGE_FLASH, th->list.bg);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_REL, th->list.btn.rel);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_PR, th->list.btn.pr);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_TGL_REL, th->list.btn.tgl_rel);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_TGL_PR, th->list.btn.tgl_pr);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_INA, th->list.btn.ina);
}

static void style_mod_cb(lv_style_t *focus_style) {

	focus_style->body.border.color = LV_COLOR_SILVER;
	focus_style->body.border.opa = 100;
	focus_style->body.border.width = 5;

	focus_style->body.grad_color = LV_COLOR_WHITE;
	focus_style->body.main_color = LV_COLOR_WHITE;
	focus_style->body.opa = 20;


	/*focus_style->body.border.width = 5;
	focus_style->body.border.color = LV_COLOR_SILVER;
	focus_style->body.border.part = 0;

	focus_style->body.main_color = LV_COLOR_MAKE(0x35, 0x35, 0x35);
	focus_style->body.grad_color = LV_COLOR_MAKE(0x35, 0x35, 0x35);
	focus_style->body.;
	focus_style->body.padding.hor = 100;
	focus_style->body.padding.ver = 100;
	focus_style->body.padding.inner = 100;
	focus_style->body.opa = 255;
	focus_style->body.empty = 0;*/

	/*LV_COLOR_WHITE   LV_COLOR_MAKE(0xFF, 0xFF, 0xFF)
#define LV_COLOR_SILVER  LV_COLOR_MAKE(0xC0,0xC0,0xC0)
#define LV_COLOR_GRAY    LV_COLOR_MAKE(0x80,0x80,0x80)
#define LV_COLOR_BLACK   LV_COLOR_MAKE(0x00,0x00,0x00)
#define LV_COLOR_RED     LV_COLOR_MAKE(0xFF,0x00,0x00)
#define LV_COLOR_MAROON  LV_COLOR_MAKE(0x80,0x00,0x00)
#define LV_COLOR_YELLOW  LV_COLOR_MAKE(0xFF,0xFF,0x00)
#define LV_COLOR_OLIVE   LV_COLOR_MAKE(0x80,0x80,0x00)
#define LV_COLOR_LIME    LV_COLOR_MAKE(0x00,0xFF,0x00)
#define LV_COLOR_GREEN   LV_COLOR_MAKE(0x00,0x80,0x00)
#define LV_COLOR_CYAN    LV_COLOR_MAKE(0x00,0xFF,0xFF)
#define LV_COLOR_AQUA    LV_COLOR_CYAN
#define LV_COLOR_TEAL    LV_COLOR_MAKE(0x00,0x80,0x80)
#define LV_COLOR_BLUE    LV_COLOR_MAKE(0x00,0x00,0xFF)
#define LV_COLOR_NAVY    LV_COLOR_MAKE(0x00,0x00,0x80)
#define LV_COLOR_MAGENTA LV_COLOR_MAKE(0xFF,0x00,0xFF)
#define LV_COLOR_PURPLE  LV_COLOR_MAKE(0x80,0x00,0x80)
#define LV_COLOR_ORANGE*/
}


static void library_focus_cb(lv_group_t * my_group) {
	lv_list_focus(*(my_group->obj_focus), true);
}
static void indev_feedback(lv_indev_t * indev, lv_signal_t signal) {

	lv_indev_data_t * data = ((lv_indev_data_t *)(indev->driver.user_data));

	switch (signal) {
		case LV_SIGNAL_FOCUS:
			if (indev->group == tab_group) {


				tab_id_t aux_id = curr_tab_id + data->enc_diff;

				if (aux_id < 0)
					curr_tab_id = 0;
				else if (aux_id >= (id_count - 1))
					curr_tab_id = id_count - 1;
				else
					curr_tab_id = aux_id;

				lv_tabview_set_tab_act(tab_view, curr_tab_id, true);
			}
			return;

		case LV_SIGNAL_CONTROLL:
			if (data->key == LV_GROUP_KEY_ENTER) {
				switch (curr_tab_id) {
				case HOME_ID:
					return;

				case SEARCH_ID:
					return;

				case LIBRARY_ID:
					/*Adding the input device(s) to the created group*/
					if (gui_indev) lv_indev_set_group(gui_indev, library_group);
					return;
				}
				return;
			}
			return;

		case LV_SIGNAL_LONG_PRESS:
			/*Adding the input device(s) to the created group*/
			if (gui_indev) lv_indev_set_group(gui_indev, tab_group);
			return;

		default:
			return;
	}
}

static lv_res_t list_btn_action(lv_obj_t * obj) {

}

