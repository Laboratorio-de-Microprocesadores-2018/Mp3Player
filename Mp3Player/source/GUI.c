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

 /**********************
 *  STATIC PROTOTYPES
 **********************/

static void GUI_CreateGUI(void);

static void GUI_CreateHeader(void);

static void GUI_CreateTabView(void);
static void GUI_SetTabViewStyle(lv_obj_t * tabView);

static void GUI_CreateLibraryTab(void);
static void GUI_SetListStyle(lv_obj_t * list);

static void GUI_StyleModCallBack(lv_style_t * focusStyle);

static void GUI_LibraryFocusCallBack(lv_group_t * libraryGroup);
static void GUI_indevFeedback(lv_indev_t * indev, lv_signal_t signal);

static lv_res_t GUI_SongListBtnAction(lv_obj_t * obj);

static char * GUI_GetTimeString(void);

/**********************
 *  STATIC VARIABLES
 **********************/
GUI gui;

/**********************
*      MACROS
**********************/
#define GUI_TIME_NUM_BASE	10
#define GUI_TIME_STRING_BUFFER_LENGTH	9
/**********************
*   GLOBAL FUNCTIONS
**********************/
/**
* Initialize the GUI
*/
void GUI_Init(void(*HalInit)(lv_indev_t ** indev)) {

	/*Initialize LittlevGL*/
	lv_init();

	gui.indev = NULL;
	(*HalInit)(&gui.indev);

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
	gui.theme= lv_theme_night_init(250, NULL);

	/*Create the screen to be used*/
	gui.screen = lv_scr_act();

	/*Create header*/
	GUI_CreateHeader();

	/*Create tab view*/
	GUI_CreateTabView();
}

static void GUI_CreateHeader(void)
{

	gui.header = lv_cont_create(gui.screen, NULL);
	lv_obj_set_size(gui.header, GUI_TOP_HEADER_HOR_SIZE, GUI_TOP_HEADER_VER_SIZE);

	lv_obj_align(gui.header, gui.screen, LV_ALIGN_IN_TOP_MID, 0, 0);      /*Align the container*/

	gui.theme->cont->body.radius = 0;
	lv_cont_set_style(gui.header, gui.theme->cont);

	/*Add a time and battery to the container*/
	gui.headerTimeTxt = lv_label_create(gui.header, NULL);
	lv_label_set_style(gui.headerTimeTxt, gui.theme->label.prim);
	lv_label_set_text(gui.headerTimeTxt, GUI_GetTimeString());

	gui.headerBatteryTxt = lv_label_create(gui.header, gui.headerTimeTxt);
	lv_label_set_text(gui.headerBatteryTxt, SYMBOL_BATTERY_FULL);

	lv_obj_align(gui.headerTimeTxt, gui.header, LV_ALIGN_CENTER, 0, 0);      /*Align time text*/
	lv_obj_align(gui.headerBatteryTxt, gui.header, LV_ALIGN_IN_RIGHT_MID, -12, 0);      /*Align battery text*/
}

void GUI_UpdateHeader(void)
{

	lv_label_set_text(gui.headerTimeTxt, GUI_GetTimeString());
}

static void GUI_CreateTabView(void)
{

	/*Create the tab view for the different sections of the GUI*/

	gui.tabView = lv_tabview_create(gui.screen, NULL);
	lv_obj_set_size(gui.tabView, GUI_TAB_VIEW_HOR_SIZE, GUI_TAB_VIEW_VER_SIZE);

	lv_obj_align(gui.tabView, gui.screen, LV_ALIGN_IN_BOTTOM_MID, 0, 0);	/*Align the tab view*/

	GUI_SetTabViewStyle(gui.tabView);
	lv_tabview_set_btns_pos(gui.tabView, LV_TABVIEW_BTNS_POS_BOTTOM);


	gui.currTabId = 0; gui.tabIdCount = 0;
	gui.homeTab = lv_tabview_add_tab(gui.tabView, "  " SYMBOL_HOME "\nHome"); gui.tabIdCount++;
	gui.searchTab = lv_tabview_add_tab(gui.tabView, "  " SYMBOL_LIST "\nSearch"); gui.tabIdCount++;
	gui.libraryTab = lv_tabview_add_tab(gui.tabView, "  " SYMBOL_DIRECTORY "\nLibrary"); gui.tabIdCount++;

	/*Create an object group*/
	gui.tabGroup = lv_group_create();
	lv_group_set_style_mod_cb(gui.tabGroup, GUI_StyleModCallBack);

	/*Adding the input device(s) to the created group*/
	if (gui.indev) lv_indev_set_group(gui.indev, gui.tabGroup);
	lv_indev_set_feedback(gui.indev, GUI_indevFeedback);

	lv_group_add_obj(gui.tabGroup, gui.homeTab);
	lv_group_add_obj(gui.tabGroup, gui.searchTab);
	lv_group_add_obj(gui.tabGroup, gui.libraryTab);

	/*Configure different styles*/
	lv_page_set_style(gui.homeTab, LV_PAGE_STYLE_BG, gui.theme->page.bg);
	lv_page_set_style(gui.searchTab, LV_PAGE_STYLE_BG, gui.theme->page.bg);
	lv_page_set_style(gui.libraryTab, LV_PAGE_STYLE_BG, gui.theme->page.bg);

	GUI_CreateLibraryTab();

}
static void GUI_SetTabViewStyle(lv_obj_t * tabView)
{

	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BG, gui.theme->tabview.bg);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_INDIC, gui.theme->tabview.indic);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BTN_BG, gui.theme->tabview.btn.bg);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BTN_REL, gui.theme->tabview.btn.rel);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BTN_PR, gui.theme->tabview.btn.pr);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BTN_TGL_REL, gui.theme->tabview.btn.tgl_rel);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BTN_TGL_PR, gui.theme->tabview.btn.tgl_pr);
}

static void GUI_CreateLibraryTab(void)
{
	/*Create an object group*/
	gui.libraryGroup = lv_group_create();
	lv_group_set_focus_cb(gui.libraryGroup, GUI_LibraryFocusCallBack);

	/*Add a song list* in the Library group*/
	gui.libraryList = lv_list_create(gui.libraryTab, NULL);            /*Create a drop down list*/
	lv_obj_set_size(gui.libraryList, lv_page_get_fit_width(gui.libraryTab), lv_page_get_fit_height(gui.libraryTab));

	//lv_obj_align(gui.libraryList, gui.libraryTab, LV_ALIGN_CENTER, 0, 0);         /*Align next to the slider*/
	lv_obj_align(gui.libraryList, NULL, LV_ALIGN_CENTER, 0, 0);

	GUI_SetListStyle(gui.libraryList);

	char songs[20][20];
	uint8_t songsCount = sizeof(songs)/ sizeof(songs[0]);

	for (int i = 0; i < songsCount; i++) {
		sprintf(songs[i], "song %d", i);

		lv_obj_t * song;
		song = lv_list_add(gui.libraryList, NULL, songs[i], GUI_SongListBtnAction);
		lv_group_add_obj(gui.libraryGroup, song);

	}

	//lv_obj_set_free_ptr(ddlist, slider);                                   /*Save the pointer of the slider in the ddlist (used in 'ddlist_action()')*/
	//lv_obj_set_top(gui.libraryList, true);                                        /*Enable to be on the top when clicked*/
}
static void GUI_SetListStyle(lv_obj_t * list)
{
	lv_list_set_style(list, LV_LIST_STYLE_BG, gui.theme->list.bg);
	lv_list_set_style(list, LV_LIST_STYLE_SCRL, gui.theme->list.scrl);
	lv_list_set_style(list, LV_LIST_STYLE_SB, gui.theme->list.sb);
	lv_list_set_style(list, LV_LIST_STYLE_EDGE_FLASH, gui.theme->list.bg);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_REL, gui.theme->list.btn.rel);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_PR, gui.theme->list.btn.pr);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_TGL_REL, gui.theme->list.btn.tgl_rel);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_TGL_PR, gui.theme->list.btn.tgl_pr);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_INA, gui.theme->list.btn.ina);
}
static void GUI_StyleModCallBack(lv_style_t * focusStyle)
{

	focusStyle->body.border.color = LV_COLOR_SILVER;
	focusStyle->body.border.opa = 100;
	focusStyle->body.border.width = 5;

	focusStyle->body.grad_color = LV_COLOR_WHITE;
	focusStyle->body.main_color = LV_COLOR_WHITE;
	focusStyle->body.opa = 20;

}

static void GUI_LibraryFocusCallBack(lv_group_t * libraryGroup)
{
	lv_list_focus(*(libraryGroup->obj_focus), true);
}
static void GUI_indevFeedback(lv_indev_t * indev, lv_signal_t signal)
{

	lv_indev_data_t * data = ((lv_indev_data_t *)(indev->driver.user_data));

	switch (signal) {
		case LV_SIGNAL_FOCUS:
			if (indev->group == gui.tabGroup) {


				tab_id_t auxId = gui.currTabId + data->enc_diff;

				if (auxId < 0)
					gui.currTabId = 0;
				else if (auxId >= (gui.tabIdCount - 1))
					gui.currTabId = gui.tabIdCount - 1;
				else
					gui.currTabId = auxId;

				lv_tabview_set_tab_act(gui.tabView, gui.currTabId, true);
			}
			return;

		case LV_SIGNAL_CONTROLL:
			if (data->key == LV_GROUP_KEY_ENTER) {
				switch (gui.currTabId) {
				case HOME_ID:
					return;

				case SEARCH_ID:
					return;

				case LIBRARY_ID:
					/*Adding the input device(s) to the created group*/
					if (gui.indev) lv_indev_set_group(gui.indev, gui.libraryGroup);
					return;
				}
				return;
			}
			return;

		case LV_SIGNAL_LONG_PRESS:
			/*Adding the input device(s) to the created group*/
			if (gui.indev) lv_indev_set_group(gui.indev, gui.tabGroup);
			return;

		default:
			return;
	}
}

static lv_res_t GUI_SongListBtnAction(lv_obj_t * obj)
{
	return 1;

}

static char * GUI_GetTimeString(void){

	TM_date guiInitialTime = TM_GetDate();
	static char guiTimeTxtBuffer[GUI_TIME_STRING_BUFFER_LENGTH];
	static char guiTimeStrng[GUI_TIME_STRING_BUFFER_LENGTH];

	guiTimeStrng[0] = '\n';
	uitoa(guiInitialTime.hour, guiTimeStrng, GUI_TIME_NUM_BASE);
	strcat(guiTimeStrng, ":");
	strcat(guiTimeStrng, uitoa(guiInitialTime.minute, guiTimeTxtBuffer, GUI_TIME_NUM_BASE));
	strcat(guiTimeStrng, ":");
	strcat(guiTimeStrng, uitoa(guiInitialTime.second, guiTimeTxtBuffer, GUI_TIME_NUM_BASE));

	return guiTimeStrng;
}

