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
 /**
 * Creation and definition of GUI structure
 */
static void GUI_HalInit(void);

static void GUI_CreateGUI(void);
static void GUI_CreateHeader(void);

static void GUI_CreateTabView(void);
static lv_obj_t *  GUI_CreateTab(lv_obj_t * tabView, const char * tabString, GUI_tab_id_t tabId, lv_event_cb_t tabEvenetCb, lv_group_t * tabGroup,
	lv_page_style_t tabStyleType, const lv_style_t * tabStyle);

static void GUI_CreateLibraryTab(void);

/**
* Style definition functions
*/
static void GUI_SetTabViewStyle(lv_obj_t * tabView);
static void GUI_TabGroupStyleModCB(struct _lv_group_t * objGroup, lv_style_t * focusStyle);
static void GUI_SetListStyle(lv_obj_t * list);

/**
* Event handler call backs
*/
static void GUI_TabEventhandlerCB(lv_obj_t * obj, lv_event_t event);
static void GUI_LibraryFocusCB(lv_group_t * libraryGroup);
static void GUI_LibraryEventHandlerCB(lv_obj_t * obj, lv_event_t event);

/**
* Auxliary local functions
*/
static char * GUI_GetTimeString(void);
static bool GUI_KeyPadRead(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static bool GUI_EncoderRead(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

static lv_res_t GUI_SongListSelectBtnAction(lv_obj_t * obj);

/**********************
 *  STATIC VARIABLES
 **********************/
static GUI gui;

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
void GUI_Init(void) {

	/*Initialize LittlevGL*/
	lv_init();

	gui.encIndev = NULL;
	gui.keyPadIndev = NULL;

	GUI_HalInit();

	GUI_CreateGUI();
}
void GUI_UpdateHeader(void)
{
	lv_label_set_text(gui.headerTimeTxt, GUI_GetTimeString());
}
lv_disp_drv_t * GUI_GetDispDrv(void)
{
	return &gui.dispDrv;
}
/**********************
 *   STATIC FUNCTIONS
 **********************/
 /**
  * Initialize Hardware abstraction layer for GUI implementation
  */
static void GUI_HalInit(void)
{
	Input_Init();

	GILI9341_Init();

	static lv_disp_buf_t dispBuffer;
	static lv_color_t drawingBuffer[LV_HOR_RES_MAX*20];
	lv_disp_buf_init(&dispBuffer, drawingBuffer, NULL, LV_HOR_RES_MAX*20);

	lv_disp_drv_init(&gui.dispDrv);            /*Basic initialization*/
	gui.dispDrv.buffer = &dispBuffer;
	gui.dispDrv.flush_cb = GILI9341_Flush;
	gui.disp = lv_disp_drv_register(&gui.dispDrv);

	lv_indev_drv_t keyPadDrv;
	lv_indev_drv_init(&keyPadDrv);
	keyPadDrv.type = LV_INDEV_TYPE_KEYPAD;
	keyPadDrv.read_cb = GUI_KeyPadRead;
	gui.keyPadIndev = lv_indev_drv_register(&keyPadDrv);	//assert keyPadIndev

	lv_indev_drv_t encDrv;
	lv_indev_drv_init(&encDrv);
	encDrv.type = LV_INDEV_TYPE_ENCODER;
	encDrv.read_cb = GUI_EncoderRead;
	gui.encIndev = lv_indev_drv_register(&encDrv); //assert encIndev
}

 /**
 * Create a mp3 graphic user interface
 */
static void GUI_CreateGUI(void)
{

	/*Create theme for the GUI*/
	gui.theme = lv_theme_night_init(250, NULL);
	lv_theme_set_current(gui.theme);

	/*Create the screen to be used*/
	gui.screen = lv_scr_act();
	
	/*Create header*/
	GUI_CreateHeader();

	/*Create tab view*/
	GUI_CreateTabView();

	/*Create Library Tab*/
	GUI_CreateLibraryTab();
}
static void GUI_CreateHeader(void)
{

	gui.header = lv_cont_create(gui.screen, NULL);
	lv_obj_set_size(gui.header, GUI_TOP_HEADER_HOR_SIZE, GUI_TOP_HEADER_VER_SIZE);

	lv_obj_align(gui.header, gui.screen, LV_ALIGN_IN_TOP_MID, 0, 0);      /*Align the container*/

	gui.theme->style.cont->body.radius = 0;
	lv_cont_set_style(gui.header, LV_CONT_STYLE_MAIN, gui.theme->style.cont);

	/*Add a time and battery to the container*/
	gui.headerTimeTxt = lv_label_create(gui.header, NULL);
	lv_label_set_style(gui.headerTimeTxt, LV_CONT_STYLE_MAIN, gui.theme->style.label.prim);
	GUI_UpdateHeader();

	gui.headerBatteryTxt = lv_label_create(gui.header, gui.headerTimeTxt);
	lv_label_set_text(gui.headerBatteryTxt, LV_SYMBOL_BATTERY_FULL);

	lv_obj_align(gui.headerTimeTxt, gui.header, LV_ALIGN_CENTER, 0, 0);      /*Align time text*/
	lv_obj_align(gui.headerBatteryTxt, gui.header, LV_ALIGN_IN_RIGHT_MID, -12, 0);      /*Align battery text*/
}


static void GUI_CreateTabView(void)
{

	/*Create an object group*/
	gui.tabGroup = lv_group_create();
	//lv_group_set_style_mod_cb(gui.tabGroup, GUI_TabGroupStyleModCB);
	//lv_group_set_click_focus(gui.tabGroup, false);
	lv_group_set_wrap(gui.tabGroup, false);

	/*Adding the input device(s) to the created group*/
	lv_indev_set_group(gui.encIndev, gui.tabGroup);
	lv_indev_set_group(gui.keyPadIndev, gui.tabGroup);

	/*Create the tab view for the different sections of the GUI*/
	gui.tabView = lv_tabview_create(gui.screen, NULL);
	lv_obj_set_size(gui.tabView, GUI_TAB_VIEW_HOR_SIZE, GUI_TAB_VIEW_VER_SIZE);

	lv_obj_align(gui.tabView, gui.screen, LV_ALIGN_IN_BOTTOM_MID, 0, 0);	/*Align the tab view*/

	//GUI_SetTabViewStyle(gui.tabView);
	lv_tabview_set_btns_pos(gui.tabView, LV_TABVIEW_BTNS_POS_BOTTOM);


	gui.currTabId = 0; gui.tabIdCount = 0;

	gui.homeTab = GUI_CreateTab(gui.tabView, "  " LV_SYMBOL_HOME "\nHome", HOME_TAB_ID, GUI_TabEventhandlerCB, gui.tabGroup, LV_PAGE_STYLE_BG, gui.theme->style.page.bg);
	gui.searchTab = GUI_CreateTab(gui.tabView, "  " LV_SYMBOL_LIST "\nSearch", SEARCH_TAB_ID, GUI_TabEventhandlerCB, gui.tabGroup, LV_PAGE_STYLE_BG, gui.theme->style.page.bg);
	gui.libraryTab = GUI_CreateTab(gui.tabView, "  " LV_SYMBOL_DIRECTORY "\nLibrary", LIBRARY_TAB_ID, GUI_TabEventhandlerCB, gui.tabGroup, LV_PAGE_STYLE_BG, gui.theme->style.page.bg);

}
static lv_obj_t *  GUI_CreateTab(lv_obj_t * tabView, const char * tabString, GUI_tab_id_t tabId, lv_event_cb_t tabEvenetCb, lv_group_t * tabGroup,
	lv_page_style_t tabStyleType, const lv_style_t * tabStyle)
{
	lv_obj_t * tabRet = lv_tabview_add_tab(tabView, tabString); gui.tabIdCount++;

	tabRet->user_data = lv_mem_alloc(sizeof(GUI_tab_id_t));

	if (tabRet->user_data != NULL) *((GUI_tab_id_t *)tabRet->user_data) = tabId; //Poner assert

	if (tabEvenetCb != NULL) lv_obj_set_event_cb(tabRet, tabEvenetCb);

	if (tabGroup != NULL) lv_group_add_obj(tabGroup, tabRet);

	lv_page_set_style(tabRet, tabStyleType, tabStyle);

	return tabRet;
}

static void GUI_CreateLibraryTab(void)
{
	/*Create an object group*/
	gui.libraryGroup = lv_group_create();
	lv_group_set_wrap(gui.libraryGroup, false);
	lv_group_set_focus_cb(gui.libraryGroup, GUI_LibraryFocusCB);

	/*Add a song list* in the Library group*/
	gui.libraryList = lv_list_create(gui.libraryTab, NULL);            /*Create a drop down list*/
	lv_obj_set_event_cb(gui.libraryList, GUI_LibraryEventHandlerCB);
	lv_obj_set_size(gui.libraryList, lv_page_get_fit_width(gui.libraryTab), lv_page_get_fit_height(gui.libraryTab));

	//lv_obj_align(gui.libraryList, gui.libraryTab, LV_ALIGN_CENTER, 0, 0);         /*Align next to the slider*/
	lv_obj_align(gui.libraryList, NULL, LV_ALIGN_CENTER, 0, 0);

	GUI_SetListStyle(gui.libraryList);

	char songs[20][20];
	uint8_t songsCount = sizeof(songs) / sizeof(songs[0]);

	for (int i = 0; i < songsCount; i++) {
		sprintf(songs[i], "song %d", i);

		lv_obj_t * song;
		song = lv_list_add_btn(gui.libraryList, LV_SYMBOL_AUDIO, songs[i]);
		lv_obj_set_event_cb(song, GUI_LibraryEventHandlerCB);
		//song = lv_list_add(gui.libraryList, NULL, songs[i], GUI_SongListSelectBtnAction);
		lv_group_add_obj(gui.libraryGroup, song);

	}

	//lv_obj_set_free_ptr(ddlist, slider);                                   /*Save the pointer of the slider in the ddlist (used in 'ddlist_action()')*/
	//lv_obj_set_top(gui.libraryList, true);                                        /*Enable to be on the top when clicked*/
}

/**
* Style definition functions
*/
static void GUI_SetTabViewStyle(lv_obj_t * tabView)
{

	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BG, gui.theme->style.tabview.bg);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_INDIC, gui.theme->style.tabview.indic);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BTN_BG, gui.theme->style.tabview.btn.bg);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BTN_REL, gui.theme->style.tabview.btn.rel);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BTN_PR, gui.theme->style.tabview.btn.pr);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BTN_TGL_REL, gui.theme->style.tabview.btn.tgl_rel);
	lv_tabview_set_style(tabView, LV_TABVIEW_STYLE_BTN_TGL_PR, gui.theme->style.tabview.btn.tgl_pr);
}
static void GUI_TabGroupStyleModCB(struct _lv_group_t * objGroup, lv_style_t * focusStyle)
{

	focusStyle->body.border.color = LV_COLOR_SILVER;
	focusStyle->body.border.opa = 100;
	focusStyle->body.border.width = 5;

	focusStyle->body.grad_color = LV_COLOR_WHITE;
	focusStyle->body.main_color = LV_COLOR_WHITE;
	focusStyle->body.opa = 0;

}
static void GUI_SetListStyle(lv_obj_t * list)
{
	lv_list_set_style(list, LV_LIST_STYLE_BG, gui.theme->style.list.bg);
	lv_list_set_style(list, LV_LIST_STYLE_SCRL, gui.theme->style.list.scrl);
	lv_list_set_style(list, LV_LIST_STYLE_SB, gui.theme->style.list.sb);
	lv_list_set_style(list, LV_LIST_STYLE_EDGE_FLASH, gui.theme->style.list.bg);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_REL, gui.theme->style.list.btn.rel);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_PR, gui.theme->style.list.btn.pr);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_TGL_REL, gui.theme->style.list.btn.tgl_rel);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_TGL_PR, gui.theme->style.list.btn.tgl_pr);
	lv_list_set_style(list, LV_LIST_STYLE_BTN_INA, gui.theme->style.list.btn.ina);
}

/**
* Event handler call backs
*/
static void GUI_TabEventhandlerCB(lv_obj_t * obj, lv_event_t event)
{
	GUI_tab_id_t * tabIdPtr = (GUI_tab_id_t *)(obj->user_data);
	lv_key_t pressedKey = LV_KEY_HOME;
	
	(void)obj; /*Unused*/

	switch (event)
	{
	case LV_EVENT_PRESSED:
		if (*tabIdPtr == LIBRARY_TAB_ID)
		{
			lv_indev_set_group(gui.encIndev, gui.libraryGroup); //assert
			lv_indev_set_group(gui.keyPadIndev, gui.libraryGroup); //assert
		}
		break;

	case LV_EVENT_KEY:
		pressedKey = gui.keyPadIndev->proc.types.keypad.last_key;
		switch (pressedKey)
		{
		case LV_KEY_ENTER:
			if (*tabIdPtr == LIBRARY_TAB_ID)
			{
				lv_indev_set_group(gui.encIndev, gui.libraryGroup); //assert
				lv_indev_set_group(gui.keyPadIndev, gui.libraryGroup); //assert
			}	
			break;

		case LV_KEY_NEXT:
			//lv_tabview_set_tab_act(gui.tabView, *tabIdPtr, true);
			lv_group_focus_next(gui.tabGroup);
			break;

		case LV_KEY_PREV:
			lv_group_focus_prev(gui.tabGroup);
			break;

		default:
			break;

		}
		break;

	case LV_EVENT_FOCUSED:
		lv_tabview_set_tab_act(gui.tabView, *tabIdPtr, true);
		break;

	default:
		break;
	}

}
static void GUI_LibraryFocusCB(lv_group_t * libraryGroup)
{
	lv_list_focus(*(libraryGroup->obj_focus), true);
}
static void GUI_LibraryEventHandlerCB(lv_obj_t * obj, lv_event_t event)
{
	switch (event)
	{
	case LV_EVENT_LONG_PRESSED:
		lv_indev_set_group(gui.encIndev, gui.tabGroup);	//assert
		lv_indev_set_group(gui.keyPadIndev, gui.tabGroup); //assert
		break;

	default:
		break;
	}
}

/**
* Auxliary local functions
*/
static char * GUI_GetTimeString(void)
{

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
static lv_res_t GUI_SongListSelectBtnAction(lv_obj_t * obj)
{
	return 1;
}

/**
 * Get the last pressed or released character from the PC's keyboard
 * @param indev_drv pointer to the related input device driver
 * @param data store the read data here
 * @return false: because the points are not buffered, so no more data to be read
 */
static bool GUI_KeyPadRead(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;      /*Unused*/
    //ButtonID buttonId = PREV;
    //for(buttonId = PREV; buttonId < NumberOfButtons; ++buttonId ){}
    if(!Input_ReadSelectButton())
    {
    	data->state = LV_INDEV_STATE_PR;
    	data->key = LV_KEY_ENTER;
    }else if(!Input_ReadNextButton())
    {
    	data->state = LV_INDEV_STATE_PR;
    	data->key = LV_KEY_NEXT;
    }else if(!Input_ReadPrevButton())
    {
    	data->state = LV_INDEV_STATE_PR;
    	data->key = LV_KEY_PREV;
    }else
    {
    	data->state = LV_INDEV_STATE_REL;
    	data->key = LV_KEY_HOME;
    }

    return false;
}

static bool GUI_EncoderRead(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
	(void) indev_drv;      /*Unused*/
	data->state = LV_INDEV_STATE_REL;
//	if (data->state == LV_INDEV_STATE_PR)
//		sprintf("Hola mundo");
	static uint8_t encoderCount;
	//encoderCount = Input_ReadEncoderCount();
	encoderCount = 0;
	//if (encoderCount != 0)
		data->enc_diff = encoderCount;
	//enc_diff = 0;

	return false;       /*No more data to read so return false*/
}
