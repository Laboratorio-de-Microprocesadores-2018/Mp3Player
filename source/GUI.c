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

 /////////////////////////////////////////////////////////////////////////////////
 //                             Included header files                           //
 /////////////////////////////////////////////////////////////////////////////////

#include "GUI.h"
#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "MP3Player.h"
#include "FileExplorer.h"
#include "Calendar.h"


#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>
#include <assert.h>
#include "lv_drivers/indev/keyboard.h"
#include "lv_drivers/indev/mousewheel.h"
#include "lv_drivers/display/monitor.h"
#include "lv_settings.h"
#define PRINTF printf

#else
#include "fsl_debug_console.h"
#include "GILI9341.h"
#include "Input.h"
#include "CPUTimeMeasurement.h"
#include "lv_settings.h"
#endif



///////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                     //
///////////////////////////////////////////////////////////////////////////////


/** General sizes and positions */
#define GUI_TOP_HEADER_WIDTH	(LV_HOR_RES)
#define GUI_TOP_HEADER_HEIGHT	(LV_VER_RES * 0.07)

#define GUI_SCREEN_WIDTH		(LV_HOR_RES)
#define GUI_SCREEN_HEIGHT		(LV_VER_RES - GUI_TOP_HEADER_HEIGHT)


/** Music screen sizes and positions */
#define BAR_WIDTH				(GUI_SCREEN_WIDTH  * 0.8)
#define BAR_HEIGHT				(GUI_SCREEN_HEIGHT * 0.05)
#define BAR_SPACING				(GUI_SCREEN_HEIGHT * 0.10)

#define ALBUM_ART_SIZE			(200)
#define ALBUM_ART_SPACING		(10)

/** Video buffer size. */
#define LV_VDB_BUFF_SIZE       	(10*1024) //(uint32_t)255


#define GUI_TIME_STRING_BUFFER_LENGTH	9

/**
 * Abstraction for HAL init
 */
#if defined(_WIN64) || defined(_WIN32)
#define GET_TICK			SDL_GetTicks()
#define MONITOR_INIT		monitor_init()
#define MONITOR_DEINIT		
#define MONITOR_FLUSH_CB	monitor_flush
#define KEYPAD_READ_CB		GUI_KeyPadRead
#define ENCODER_READ_CB		mousewheel_read
#else
#define GET_TICK			lv_tick_get()
#define MONITOR_INIT		GILI9341_Init()
#define MONITOR_DEINIT		GILI9341_Deinit()
#define MONITOR_FLUSH_CB	GILI9341_Flush
#define KEYPAD_READ_CB		GUI_KeyPadRead
#define ENCODER_READ_CB		GUI_EncoderRead
#endif


/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
typedef enum {
	MUSIC_TAB,
	EXPLORER_TAB,
	SETTINGS_TAB,
	POWER_TAB,
	TAB_NUM
}TabID_t;

/**
 *
 */
typedef enum {
	ENTRY_FOLDER,
	ENTRY_SONG,
	ENTRY_FILE,
}EntryType_t;

/**
 *
 */
typedef struct
{
	EntryType_t type;
	uint32_t index;
}DirEntry_t;

/**
 * @brief Base screen definition.
 *
 * Every screen has a parent object and a group of objects where to focus input
 * events.
 */
typedef struct
{
	lv_obj_t* parent;
	lv_group_t* group;
}Screen_t;

/** Declare a sceen inside struct GUI_t. */
#define GUI_SCREEN_DECLARE(screenName,objects)  		\
							struct						\
							{							\
								struct					\
								{						\
									lv_obj_t* parent;	\
									lv_group_t* group;	\
								};						\
								objects					\
							}screenName;				\

typedef struct {
	lv_obj_t* container;
	lv_obj_t* song;
	lv_obj_t* artist;
	lv_obj_t* album;
	lv_obj_t* date;
	lv_obj_t* genre;
}GUI_Metadata_t;
/**
 * @brief GUI structure definition
 */
typedef struct
{
	/* Display driver */
	lv_disp_drv_t dispDrv;

	/* Display object*/
	lv_disp_t* disp;

	/* Input devices: encoder and keypad*/
	lv_indev_t* encoder;
	lv_indev_t* keypad;

	/* GUI theme*/
	lv_theme_t* theme;

	/* GUI header, time and battery*/
	struct
	{
		lv_obj_t* parent;
		lv_obj_t* time;
		lv_obj_t * battery;
	}header;

	/* Menu screen. */
	GUI_SCREEN_DECLARE(
	menuScreen,
	lv_obj_t* tabs[TAB_NUM];)
	
	/* Music screen. */
	GUI_SCREEN_DECLARE(
	musicScreen,
	lv_obj_t* progressBar;
	lv_obj_t* elapsedTime;
	lv_obj_t* remainingTime;
	lv_obj_t* Title;
	/*lv_obj_t* Album;*/
	lv_obj_t* Artist;
	lv_obj_t* queueProgress;
	/*lv_obj_t* Year;*/
	lv_obj_t* albumArt;
	GUI_Metadata_t metadata;
	lv_obj_t* volumeBar;)

	/* Browser screen. */
	GUI_SCREEN_DECLARE(
	browserScreen, 
	lv_obj_t* list;
	uint32_t songsIndex[MAX_FILES_PER_DIR];
	uint32_t nSongs;)

	/* Settings screen. */
	GUI_SCREEN_DECLARE(
	settingsScreen,
	lv_obj_t* settingsBtn;
	lv_settings_item_t settingsRootItem;)
	
	/* Power screen. */
	GUI_SCREEN_DECLARE(
	powerScreen, )
	
	/* Pointers to base screens to make screen switching easier. */
	Screen_t* screens[SCREEN_NUM];

	/* Pointer to current screen. */
	Screen_t* currentScreen;

	/* File browser path and depth. */
	char browserPath[255];
	uint8_t browserDepth;

	/* Task to make volume bar hide automatically. */
	lv_task_t* volumeBarTask;

	/* */
	bool powerOffRequest;
}GUI_t;

static lv_settings_item_t mainMenuItems[] =
{
	   {.type = LV_SETTINGS_TYPE_LIST_BTN,.name = "Equalizer",.value = "Low, mid, and high bands."},
	   {.type = LV_SETTINGS_TYPE_LIST_BTN,.name = "Time/Date",.value = "Set time and date."},
	  // {.type = LV_SETTINGS_TYPE_LIST_BTN,.name = "Display",.value = ""},
	  // {.type = LV_SETTINGS_TYPE_LIST_BTN,.name = "Sleep mode",.value = ""},
	  // {.type = LV_SETTINGS_TYPE_LIST_BTN,.name = "Alarm",.value = ""},
	   {.type = LV_SETTINGS_TYPE_INV},     /*Mark the last item*/
};

///////////////////////////////////////////////////////////////////////////////
//                   Local function prototypes ('static')                    //
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Creation and definition of GUI structure.
 */
static void GUI_HalInit(void);

/**
 * @brief Screen creation functions.
 */
static void GUI_CreateHeader(void);
static void GUI_CreateMenuScreen(void);
static void GUI_CreateMusicScreen(void);
static void GUI_CreateBrowserScreen(void);
static void GUI_CreateSettingsScreen(void);
static void GUI_CreatePowerScreen(void);

/**
 * @brief Event handler callbacks
 */
static void GUI_MenuEventHandler(lv_obj_t* obj, lv_event_t event);

static void GUI_MusicScreenEventHandler(lv_obj_t* obj, lv_event_t event);
static void GUI_VolumeBarEventHandler(lv_obj_t* slider, lv_event_t event);

static void GUI_BrowserScreenEventHandler(lv_obj_t* obj, lv_event_t event);

static void GUI_SettingsScreenEventHandler(lv_obj_t* obj, lv_event_t event);
static void GUI_SettingsScreenRootEventCallback(lv_obj_t* btn, lv_event_t event);
static void GUI_SettingsScreenTimeEventCallback(lv_obj_t* obj, lv_event_t event);
static void GUI_SettingsScreenEqualizerEventCallback(lv_obj_t* obj, lv_event_t event);
static void GUI_SettingsScreenDisplayEventCallback(lv_obj_t* btn, lv_event_t event);
static void submenu_event_cb(lv_obj_t* btn, lv_event_t event);

static void GUI_PowerScreenEventHandler(lv_obj_t* obj, lv_event_t event);



/**
 * @brief Hide current screen and show a new one.
 * @param[in] screen ID of the screen to be shown.
 * @note All screens must be created before calling this function.
 */
static void GUI_ShowScreen(ScreenID_t screen);

/**
 *
 */
static void GUI_OpenFolder(char* folder);

/**
 *
 */
static bool GUI_ListFolderContents(char * folder);

/**
 *
 */
static void GUI_SetTrackInfo(char* fileName);

/**
 *
 */
static void GUI_UpdateProgressBar(lv_task_t* task);

/**
 *
 */
static void GUI_UpdateHeader(lv_task_t* task);

/**
 *
 */
static void GUI_ShowVolumeBar();

/**
 *	@brief Style definition callback, set no border
 */
static void GUI_BorderlessStyleModCB(struct _lv_group_t * objGroup, lv_style_t * focusStyle);

/**
 * 
 */
static char * GUI_GetTimeString(void);

/**
 *
 */
static bool GUI_KeyPadRead(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

/**
 *
 */
static bool GUI_EncoderRead(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);


///////////////////////////////////////////////////////////////////////////////
//                   Local variable definitions ('static')                   //
///////////////////////////////////////////////////////////////////////////////

/* GUI structure to store information and lvgl objects. */
static GUI_t gui;

/* Default album art image for tracks without embedded albumart*/
LV_IMG_DECLARE(defaultAlbumArt);

/* Display buffer for lvgl. */
static lv_disp_buf_t dispBuffer;

static lv_color_t vdb_buf1[LV_VDB_BUFF_SIZE] LV_ATTRIBUTE_MEM_ALIGN;
static lv_color_t vdb_buf2[LV_VDB_BUFF_SIZE] LV_ATTRIBUTE_MEM_ALIGN;

///////////////////////////////////////////////////////////////////////////////
//                                  API                                      //
///////////////////////////////////////////////////////////////////////////////

/**
* @ Brief Initialize the GUI
*/
void GUI_Init(void)
{

	/*Initialize LittlevGL*/
	lv_init();

	GUI_HalInit();

	/** Set callback to update track information every time a new
	song starts playing. */
	MP3_SetTrackChangedCB(GUI_SetTrackInfo);

	/** Set callback to update drive status. */
	// FE_SetDriveChangedCB(GUI_UpdateDriveStatus); TODO!
}

/**
* @ Brief Deinitialize the GUI
*/
void GUI_Deinit(void)
{
	MONITOR_DEINIT;
	lv_deinit();
}

/**
* Create a mp3 graphic user interface
*/
void GUI_Create(void)
{
	/* Create theme for the GUI. */
	gui.theme = lv_theme_default_init(2, NULL);
	gui.theme->style.cont->body.radius = 0;
	lv_theme_set_current(gui.theme);

	/* Create header bar. */
	GUI_CreateHeader();

	/* Create different screens. */
	GUI_CreateMenuScreen();
	GUI_CreateMusicScreen();
	GUI_CreateBrowserScreen();
	GUI_CreateSettingsScreen();
	GUI_CreatePowerScreen();

	/* Store screen pointers. */
	gui.screens[MUSIC_SCREEN] = (Screen_t*)& gui.musicScreen;
	gui.screens[BROWSER_SCREEN] = (Screen_t*)& gui.browserScreen;
	gui.screens[SETTINGS_SCREEN] = (Screen_t*)& gui.settingsScreen;
	gui.screens[POWER_SCREEN] = (Screen_t*)& gui.powerScreen;
	gui.screens[MENU_SCREEN] = (Screen_t*)& gui.menuScreen;

	/* Show menu screen on startup. */
	GUI_ShowScreen(MENU_SCREEN);
}

/**
 * @brief GUI task function.
 *		  Must be called periodically
 */
void GUI_Task()
{
	static uint32_t lastTickTime;

	uint32_t currTime = GET_TICK; 

	if(currTime-lastTickTime >= 4)
	{
#if defined(_WIN64) || defined(_WIN32)

#else
		SET_DBG_PIN(2);
#endif
		lv_tick_inc(currTime-lastTickTime);
		lastTickTime = currTime;


		lv_task_handler();
#if defined(_WIN64) || defined(_WIN32)
#else
		CLEAR_DBG_PIN(2);
#endif

		

	}
}

/**
 * @brief Notifies GUI that screen has been flushed.
 */
void GUI_FlushReady(void)
{
	lv_disp_flush_ready(&gui.dispDrv);
}

/**
 * @brief Returns if the user wants to power off the player or not.
 */
bool GUI_PowerOffRequest()
{
	return gui.powerOffRequest;
}



///////////////////////////////////////////////////////////////////////////////
//                   Local function definition ('static')                    //
///////////////////////////////////////////////////////////////////////////////


/**
* Initialize Hardware abstraction layer for GUI implementation
*/
static void GUI_HalInit(void)
{

	lv_disp_buf_init(&dispBuffer, vdb_buf1, vdb_buf2, LV_VDB_BUFF_SIZE);

	MONITOR_INIT;

	lv_disp_drv_init(&gui.dispDrv);
	gui.dispDrv.hor_res = LV_HOR_RES;
	gui.dispDrv.ver_res = LV_VER_RES;
	gui.dispDrv.buffer = &dispBuffer;
	gui.dispDrv.flush_cb = MONITOR_FLUSH_CB;
	gui.disp = lv_disp_drv_register(&gui.dispDrv);

	lv_indev_drv_t keyPadDrv;
	lv_indev_drv_init(&keyPadDrv);
	keyPadDrv.type = LV_INDEV_TYPE_KEYPAD;
	keyPadDrv.read_cb = KEYPAD_READ_CB;
	gui.keypad = lv_indev_drv_register(&keyPadDrv);

	lv_indev_drv_t encDrv;
	lv_indev_drv_init(&encDrv);
	encDrv.type = LV_INDEV_TYPE_ENCODER;
	encDrv.read_cb = ENCODER_READ_CB;
	gui.encoder = lv_indev_drv_register(&encDrv);
}

static void GUI_CreateHeader(void)
{
	/* GUI header is a container */
	gui.header.parent = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(gui.header.parent, GUI_TOP_HEADER_WIDTH, GUI_TOP_HEADER_HEIGHT);
	lv_obj_align(gui.header.parent, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

	/*Add battery information to header. */
	gui.header.battery = lv_label_create(gui.header.parent, gui.header.time);
	lv_label_set_text(gui.header.battery, LV_SYMBOL_BATTERY_FULL);
	lv_obj_align(gui.header.battery, gui.header.parent, LV_ALIGN_IN_RIGHT_MID, -15, 0);

	/* Add digital clock to header. */
	gui.header.time = lv_label_create(gui.header.parent, NULL);
	lv_obj_align(gui.header.time, gui.header.parent, LV_ALIGN_IN_LEFT_MID, 15, 0);

	/* Create task to update header information every one second. */
	lv_task_t* headerUpdateTask = lv_task_create(GUI_UpdateHeader, 1000, LV_TASK_PRIO_LOW, NULL);
	lv_task_create(GUI_UpdateProgressBar, 250, LV_TASK_PRIO_LOW, NULL);
	lv_task_ready(headerUpdateTask);
}

static void GUI_CreateMenuScreen(void)
{
	/*Create the tab view for the main menu*/
	lv_obj_t* tabView = lv_tabview_create(lv_scr_act(), NULL);
	lv_obj_set_size(tabView, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
	lv_obj_align(tabView, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);	/*Align the tab view*/
	lv_tabview_set_btns_hidden(tabView, true);

	gui.menuScreen.group = lv_group_create();
	lv_group_set_style_mod_cb(gui.menuScreen.group, GUI_BorderlessStyleModCB);
	lv_group_set_wrap(gui.menuScreen.group, false);

	//lv_group_add_obj(gui.menuScreen.group, tabView);

	char menuLabels[TAB_NUM][4] = { LV_SYMBOL_AUDIO ,
								LV_SYMBOL_DIRECTORY,
								LV_SYMBOL_SETTINGS,
								LV_SYMBOL_POWER };

	static lv_style_t style;
	memcpy(&style, gui.theme->style.label.prim, sizeof(lv_style_t));
	style.text.font = &lv_font_roboto_28;

	for (int i = 0; i < TAB_NUM; i++)
	{
		lv_obj_t* tab = lv_tabview_add_tab(tabView, menuLabels[i]);

		tab->user_data = lv_mem_alloc(sizeof(TabID_t));

		if (tab->user_data != NULL)
			* ((TabID_t*)tab->user_data) = i; //Poner assert

		lv_obj_t* label = lv_label_create(tab, NULL);
		lv_label_set_text(label, menuLabels[i]);
		lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style);
		lv_obj_align(label, tab, LV_ALIGN_CENTER, 0, 0);

		if (i > 0)
		{
			lv_obj_t* label = lv_label_create(tab, NULL);
			lv_label_set_text(label, LV_SYMBOL_LEFT);
			lv_obj_align(label, tab, LV_ALIGN_IN_LEFT_MID, 10, 0);
		}

		if (i < TAB_NUM - 1)
		{
			lv_obj_t* label = lv_label_create(tab, NULL);
			lv_label_set_text(label, LV_SYMBOL_RIGHT);
			lv_obj_align(label, tab, LV_ALIGN_IN_RIGHT_MID, -10, 0);
		}
		
		lv_group_add_obj(gui.menuScreen.group, tab);
		lv_obj_set_event_cb(tab, GUI_MenuEventHandler);

		gui.menuScreen.tabs[i] = tab;
	}

	gui.menuScreen.parent = tabView;

	lv_obj_set_hidden(gui.menuScreen.parent, true);
}

static void GUI_CreateMusicScreen()
{
	/* Parent container. */
	lv_obj_t* cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(cont, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
	lv_obj_align(cont, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

	/* Add container to a group to handle input */
	gui.musicScreen.group = lv_group_create();
	lv_group_set_style_mod_cb(gui.musicScreen.group, GUI_BorderlessStyleModCB);
	lv_group_set_style_mod_edit_cb(gui.musicScreen.group, GUI_BorderlessStyleModCB);
	//lv_group_set_wrap(gui.menuScreen.group, false);
	lv_group_add_obj(gui.musicScreen.group, cont);
	lv_obj_set_event_cb(cont, GUI_MusicScreenEventHandler);


	/* Add a bar to show track progress*/
	lv_obj_t* bar = lv_bar_create(cont, NULL);
	lv_obj_set_size(bar, BAR_WIDTH, BAR_HEIGHT);
	lv_bar_set_range(bar, 0, BAR_WIDTH);
	lv_obj_align(bar, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -BAR_SPACING);
	gui.musicScreen.progressBar = bar;

	/* Add text labels for elaped and remaining time. */
	lv_obj_t* elapsed = lv_label_create(cont, NULL);
	lv_label_set_text(elapsed, "--:--");
	lv_obj_align(elapsed, bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
	gui.musicScreen.elapsedTime = elapsed;

	lv_obj_t* remaining = lv_label_create(cont, NULL);
	lv_label_set_text(remaining, "--:--");
	lv_obj_align(remaining, bar, LV_ALIGN_OUT_BOTTOM_RIGHT, -10, 5);
	gui.musicScreen.remainingTime = remaining;

	/* Add album art image. */
	lv_obj_t* albumArt = lv_img_create(cont, NULL);
	lv_obj_set_size(albumArt, ALBUM_ART_SIZE, ALBUM_ART_SIZE);
	lv_obj_align(albumArt, cont, LV_ALIGN_IN_TOP_MID, 0, ALBUM_ART_SPACING);
	lv_img_set_src(albumArt, &defaultAlbumArt);
	gui.musicScreen.albumArt = albumArt;

	lv_style_t* labelStyle = gui.theme->style.label.prim;
	//labelStyle->text.font = &lv_font_roboto_28;

	/* Add song title */
	lv_obj_t* Title = lv_label_create(cont, NULL);
	lv_label_set_text(Title, "");
	lv_label_set_style(Title, LV_LABEL_STYLE_MAIN, gui.theme->style.label.prim);
	lv_label_set_long_mode(Title, LV_LABEL_LONG_SROLL_CIRC);
	lv_obj_set_size(Title, ALBUM_ART_SIZE, 20);
	lv_label_set_align(Title, LV_LABEL_ALIGN_LEFT);
	lv_obj_align(Title, albumArt, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
	gui.musicScreen.Title = Title;

	lv_obj_t* Artist = lv_label_create(cont, Title);
	lv_label_set_style(Artist, LV_LABEL_STYLE_MAIN, gui.theme->style.label.sec);
	lv_obj_align(Artist, Title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
	gui.musicScreen.Artist = Artist;

	lv_obj_t* queueProgress = lv_label_create(cont, Title);
	lv_label_set_style(queueProgress, LV_LABEL_STYLE_MAIN, gui.theme->style.label.sec);
	lv_label_set_align(queueProgress, LV_LABEL_ALIGN_RIGHT);
	lv_obj_align(queueProgress, Title, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 0);
	gui.musicScreen.queueProgress = queueProgress;

	// Create metadata display
	lv_obj_t* metadata = lv_cont_create(cont, NULL);
	lv_obj_set_size(metadata, ALBUM_ART_SIZE, ALBUM_ART_SIZE);
	lv_obj_align(metadata, cont, LV_ALIGN_IN_TOP_MID, 0, ALBUM_ART_SPACING);
	lv_obj_set_protect(metadata, LV_SIGNAL_CHILD_CHG);
	lv_obj_set_hidden(metadata, true);
	gui.musicScreen.metadata.container = metadata;

	/* Volume bar */
	lv_obj_t* volumeBar = lv_slider_create(cont, NULL);
	lv_obj_set_size(volumeBar, BAR_WIDTH, BAR_HEIGHT);
	lv_obj_align(volumeBar, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(volumeBar, GUI_VolumeBarEventHandler);
	lv_slider_set_range(volumeBar, 0, MP3_GetMaxVolume());
	lv_slider_set_value(volumeBar, MP3_GetVolume(), false);
	lv_group_add_obj(gui.musicScreen.group, volumeBar);
	lv_obj_set_hidden(volumeBar,true);
	gui.musicScreen.volumeBar = volumeBar;

	// Hide screen for default
	lv_obj_set_hidden(cont, true);

	gui.musicScreen.parent = cont;

}

static void GUI_CreateBrowserScreen(void)
{

	/* Create a container for this screen. */
	lv_obj_t* cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(cont, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
	lv_obj_align(cont, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

	lv_obj_set_event_cb(cont, GUI_BrowserScreenEventHandler);

	/* Create a group for this screen.  */
	lv_obj_t* group = lv_group_create();
	lv_group_set_wrap(group, false);
	lv_group_set_style_mod_cb(group, GUI_BorderlessStyleModCB);
	lv_group_set_style_mod_edit_cb(group, GUI_BorderlessStyleModCB);

	/* Create a list to add songs. */
	lv_obj_t* list = lv_list_create(cont, NULL);
	lv_obj_set_size(list, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
	lv_list_set_edge_flash(list, true);
	lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
	lv_group_add_obj(group, list);

	gui.browserScreen.list = list;
	gui.browserScreen.group = group;
	gui.browserScreen.parent = cont;

	GUI_ListFolderContents("\0");

	// Hide screen for default
	lv_obj_set_hidden(cont, true);
}

static void GUI_CreateSettingsScreen(void)
{

	/* Create a container for this screen. */
	lv_obj_t* cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(cont, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
	lv_obj_align(cont, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
	//lv_cont_set_layout(cont, LV_LAYOUT_PRETTY);

	lv_obj_set_event_cb(cont, GUI_SettingsScreenEventHandler);

	/* Create a group for this screen.  */
	lv_group_t* group = lv_group_create();
	lv_group_set_wrap(group, false);
	lv_group_set_style_mod_cb(group, GUI_BorderlessStyleModCB);
	lv_group_set_style_mod_edit_cb(group, GUI_BorderlessStyleModCB);

	/*Declare items*/
	gui.settingsScreen.settingsRootItem.name = "Settings";
	gui.settingsScreen.settingsRootItem.value = "";
	gui.settingsScreen.settingsRootItem.cont = cont;

	lv_obj_t * settingsBtn;
	lv_settings_set_parent(cont);// gui.menuScreen.tabs[SETTINGS_TAB]);
	lv_settings_set_group(group);
	settingsBtn = lv_settings_create(&gui.settingsScreen.settingsRootItem, GUI_SettingsScreenEventHandler);
	

	//lv_btn_set_style(settingsBtn, LV_BTN_STYLE_REL, &lv_style_transp);
	//lv_obj_set_hidden(settingsBtn, true);


	//gui.settingsScreen.settingsLabel = settingsLabel;
	gui.settingsScreen.settingsBtn = settingsBtn;
	gui.settingsScreen.parent = cont;
	gui.settingsScreen.group = group;
	
	// Hide screen for default
	lv_obj_set_hidden(cont, true);
}


/**
 * @brief Event handler callbacks
 */
static void GUI_CreatePowerScreen(void)
{

	/* Create a group for this screen.  */
	lv_obj_t* group = lv_group_create();
	lv_group_set_wrap(group, false);
	lv_group_set_style_mod_cb(group, GUI_BorderlessStyleModCB);
	lv_group_set_style_mod_edit_cb(group, GUI_BorderlessStyleModCB);


	lv_obj_t* cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(cont, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
	lv_obj_align(cont, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

	//lv_obj_set_event_cb(cont, GUI_PowerScreenEventHandler);

	lv_group_add_obj(group, cont);

	// Hide screen for default
	lv_obj_set_hidden(cont, true);

	gui.powerScreen.parent = cont;
	gui.powerScreen.group = group;
}





static void GUI_MetadataShow()
{
	// Create metadata display
	//lv_obj_t* metadata = lv_cont_create(gui.musicScreen.parent, NULL);
	//lv_obj_set_size(metadata, ALBUM_ART_SIZE, ALBUM_ART_SIZE);
	//lv_obj_align(metadata, gui.musicScreen.parent, LV_ALIGN_IN_TOP_MID, 0, ALBUM_ART_SPACING);
	//gui.musicScreen.metadata = metadata;

	lv_obj_set_hidden(gui.musicScreen.metadata.container, false);
}

static void GUI_MetadataHide()
{
	//lv_obj_del(gui.musicScreen.metadata);
	//gui.musicScreen.metadata = NULL;
	
	lv_obj_set_hidden(gui.musicScreen.metadata.container, true);
}




static void GUI_MenuEventHandler(lv_obj_t* obj, lv_event_t event)
{
	TabID_t tabId = 0;

	if (obj->user_data != NULL)
		tabId = *(TabID_t*)(obj->user_data);

	lv_key_t pressedKey = LV_KEY_HOME;

	switch (event)
	{
	case LV_EVENT_FOCUSED:
		lv_tabview_set_tab_act(gui.menuScreen.parent, tabId, true);
		break;

	case LV_EVENT_LONG_PRESSED:
		switch (pressedKey)
		{
		case LV_KEY_ENTER:
			break;
		case LV_KEY_UP:
			break;
		}
		break;
	case LV_EVENT_KEY:
		pressedKey = gui.keypad->proc.types.keypad.last_key;
		switch (pressedKey)
		{
		case LV_KEY_ENTER:
			if (tabId == POWER_TAB)
				gui.powerOffRequest = true;
			else
				GUI_ShowScreen(tabId);
			break;

		//case LV_KEY_RIGHT:
		//	if(tabId <)
		//	lv_tabview_set_tab_act(gui.menuScreen.parent, tabId, true);
		//	break;
		//case LV_KEY_LEFT:
		//	lv_tabview_set_tab_act(gui.menuScreen.parent, tabId, true);
		//	break;
		}

		break;
	}
}

static void GUI_MusicScreenEventHandler(lv_obj_t* obj, lv_event_t event)
{

	lv_key_t pressedKey = gui.keypad->proc.types.keypad.last_key;

	switch (event)
	{

	case LV_EVENT_FOCUSED:
	{
	}
		break;
	case LV_EVENT_KEY_RELEASED:
		switch (pressedKey)
		{
		case LV_KEY_RIGHT:
			if (MP3_GetStatus() == MP3_FASTFORWARD)
			{
				MP3_Resume();
				printf("Fastforward stop!\n");
			}
			break;
		case LV_KEY_LEFT:
			if (MP3_GetStatus() == MP3_REWIND)
			{
				MP3_Resume();
				printf("Rewind stop!\n");
			}
			break;
		case LV_KEY_DOWN:
			if (gui.musicScreen.metadata.container != NULL)
			{
				GUI_MetadataHide();
			}
			break;
		}
		break;
	case LV_EVENT_KEY_CLICKED:
		switch (pressedKey)
		{
		case LV_KEY_UP:
			GUI_ShowScreen(MENU_SCREEN);
		break;
		case LV_KEY_RIGHT:
			if (MP3_GetStatus() == MP3_FASTFORWARD)
			{
				MP3_Resume();
				printf("Fastforward stop!\n");
			}
			else
			{
				MP3_Next();
			}
			
			break;
		case LV_KEY_LEFT:
			if (MP3_GetStatus() == MP3_REWIND)
			{
				MP3_Resume();
				printf("Rewind stop!\n");
			}
			else
			{
				MP3_Prev();
			}
			break;
		case LV_KEY_DOWN:
			GUI_ShowVolumeBar();
			break;
		}
		break;
	case LV_EVENT_KEY_LONG_PRESS:
		switch (pressedKey)
		{
		case LV_KEY_ENTER:
			gui.powerOffRequest = true;
			break;
			break;
		case LV_KEY_RIGHT:
			MP3_Fastforward();
			printf("Fastforward!\n");
			break;
		case LV_KEY_LEFT:
			MP3_Rewind();
			printf("Rewind!\n");
			break;
		case LV_KEY_DOWN:
			GUI_MetadataShow();
		}
		break;
	case LV_EVENT_KEY:
		switch (pressedKey)
		{
		case LV_KEY_ENTER:
			MP3_Resume();
			break;
		}
		break;
	}
}

static void GUI_BrowserScreenEventHandler(lv_obj_t* obj, lv_event_t event)
{
	lv_key_t pressedKey = gui.keypad->proc.types.keypad.last_key;

	switch (event)
	{

	case LV_EVENT_PRESSED:
	{
		DirEntry_t* direntry = (DirEntry_t*)(obj->user_data);

		if (direntry == NULL)
		{
			GUI_ShowScreen(MENU_SCREEN);
			return;
		}

		switch (direntry->type)
		{
			case ENTRY_FOLDER:
			{
				GUI_OpenFolder(lv_list_get_btn_text(obj));
				break;
			}
			case ENTRY_SONG:
			{
				//char filePath[255];
				//sprintf(filePath, "%s\\%s.mp3", gui.browserPath, lv_list_get_btn_text(obj));
				MP3_SetSongsQueue(gui.browserScreen.songsIndex, gui.browserScreen.nSongs);
				MP3_Play(gui.browserPath, direntry->index);

				/* Show music screen */
				GUI_ShowScreen(MUSIC_TAB);
				break;
			}
			case ENTRY_FILE:
			{
			}
			break;
		}
	}
	break;

	case LV_EVENT_LONG_PRESSED:
		switch (pressedKey)
		{
		case LV_KEY_UP:
			break;
		}
		break;

	case LV_EVENT_DELETE:
		lv_mem_free(obj->user_data);
		break;

	}
}

static void GUI_SettingsScreenEventHandler(lv_obj_t* obj, lv_event_t event)
{
	lv_key_t pressedKey = LV_KEY_HOME;
	lv_settings_item_t * act_item = NULL;

	switch (event)
	{

	case LV_EVENT_FOCUSED:
		break;

	case LV_EVENT_LONG_PRESSED:
		pressedKey = gui.keypad->proc.types.keypad.last_key;
		switch (pressedKey)
		{
		case LV_KEY_ENTER:
			break;
		case LV_KEY_UP:
			break;
		}
		break;
	case LV_EVENT_KEY:
	case LV_EVENT_PRESSED:
		pressedKey = gui.keypad->proc.types.keypad.last_key;
		switch (pressedKey)
		{
		case LV_KEY_ENTER:
		case LV_KEY_UP:
			//lv_obj_set_hidden(gui.settingsScreen.settingsBtn, true);
			GUI_ShowScreen(MENU_SCREEN);
			break;
		
		case LV_KEY_NEXT:
		case LV_KEY_RIGHT:
			break;

		case LV_KEY_PREV:
		case LV_KEY_LEFT:
			break;

		default : 
			break;
		}
		break;
	case LV_EVENT_CLICKED:
		/*Get the caller item*/
		act_item = (lv_settings_item_t *)lv_event_get_data();

		/*Create a new page in the menu*/
		lv_settings_open_page(act_item, GUI_SettingsScreenRootEventCallback);
		break;
	}
}

//static void GUI_PowerScreenEventHandler(lv_obj_t* obj, lv_event_t event)
//{
//	lv_key_t pressedKey = LV_KEY_HOME;
//
//	switch (event)
//	{
//
//	case LV_EVENT_FOCUSED:
//
//		break;
//
//	case LV_EVENT_LONG_PRESSED:
//		switch (pressedKey)
//		{
//		case LV_KEY_ENTER:
//			break;
//		case LV_KEY_UP:
//			break;
//		}
//		break;
//	case LV_EVENT_KEY:
//		pressedKey = gui.keypad->proc.types.keypad.last_key;
//		switch (pressedKey)
//		{
//		case LV_KEY_ENTER:
//
//			break;
//
//			//case LV_KEY_NEXT:
//		case LV_KEY_UP:
//			GUI_ShowScreen(MENU_SCREEN);
//			break;
//		case LV_KEY_RIGHT:
//
//			break;
//
//			//case LV_KEY_PREV:
//		case LV_KEY_LEFT:
//
//			break;
//		}
//		break;
//	}
//}

/**
 * @brief Settings menu Event handler callbacks
 */
static void GUI_SettingsScreenRootEventCallback(lv_obj_t * btn, lv_event_t e)
{
	(void)btn;  /*Unused*/
	//int a = gui.header.time.
	/*Get the caller item*/
	lv_settings_item_t * act_item = (lv_settings_item_t *)lv_event_get_data();

	/*Add the items*/

	if (e == LV_EVENT_REFRESH) {

		uint32_t i;
		for (i = 0; mainMenuItems[i].type != LV_SETTINGS_TYPE_INV; i++) {
			lv_settings_add(&mainMenuItems[i]);
		}
	}
	/* Open submenus.
	 * The submenus will be very simple so use a common event callback for them.
	 * `act_item` is the clicked list button in the main menu.
	 * It's name will be the title of the new page*/
	else if (e == LV_EVENT_CLICKED) {
		if (&mainMenuItems[0] == act_item) {
			lv_settings_open_page(act_item, GUI_SettingsScreenEqualizerEventCallback);
		}
		else if (&mainMenuItems[1] == act_item) {
			lv_settings_open_page(act_item, GUI_SettingsScreenTimeEventCallback);
		}
		else if (&mainMenuItems[2] == act_item) {
			lv_settings_open_page(act_item, GUI_SettingsScreenDisplayEventCallback);
		}
	}
}

static void GUI_SettingsScreenEqualizerEventCallback(lv_obj_t * btn, lv_event_t e)
{
	(void)btn;  /*Unused*/

	static char enable_str[6];

	static char equalizer_str[MP3_NUM_BANDS][6];


	static lv_settings_item_t ecualizadorItems[] = {
		{.type = LV_SETTINGS_TYPE_SW,    .name = "Enable",		.value = enable_str,},
		{.type = LV_SETTINGS_TYPE_SLIDER,.name = "Low",			.value = equalizer_str[0]},
		{.type = LV_SETTINGS_TYPE_SLIDER,.name = "Mid",			.value = equalizer_str[1]},
		{.type = LV_SETTINGS_TYPE_SLIDER,.name = "High",		.value = equalizer_str[2]},
		{.type = LV_SETTINGS_TYPE_INV},     /*Mark the last item*/
	};


	/*Add items to the submenus*/
	if (e == LV_EVENT_REFRESH) {

		MP3_EqLevels_t eqLevles;
		MP3_GetEqualizerLevels(&eqLevles);

		ecualizadorItems[0].state = MP3_IsEqualizerEnable();
		if (ecualizadorItems[0].state == 1)
			sprintf(ecualizadorItems[0].value, "true");
		else
			sprintf(ecualizadorItems[0].value, "false");

		int8_t eqMin, eqMax;
		MP3_GetEqualizerLevelLimits(&eqMin,&eqMax);

		for(int i=0; i<MP3_NUM_BANDS; i++)
		{
			ecualizadorItems[1+i].rangeMin = eqMin;
			ecualizadorItems[1+i].rangeMax = eqMax;
			snprintf(equalizer_str[i], 	6, "%d dB", eqLevles.band[i]);
			ecualizadorItems[1+i].state = eqLevles.band[i];
		}

		uint32_t i;
		for (i = 0; ecualizadorItems[i].type != LV_SETTINGS_TYPE_INV; i++) {
			lv_settings_add(&ecualizadorItems[i]);
		}
	}
	/*Handle the items' events*/
	else if (e == LV_EVENT_VALUE_CHANGED)
	{
		/*Get the caller item*/
		lv_settings_item_t* act_item = (lv_settings_item_t*)lv_event_get_data();

		if (act_item == &ecualizadorItems[0])
		{
			if (act_item->state == 1)
				sprintf(act_item->value, "true");
			else
				sprintf(act_item->value, "false");
			MP3_EqualizerEnable(act_item->state);
		}
		else
		{
			int i = act_item - ecualizadorItems;
			MP3_SetEqualizerLevel(i-1, act_item->state );
			sprintf(act_item->value, "%d", act_item->state);
		}

		lv_settings_refr(act_item);
	}
}

static void GUI_SettingsScreenTimeEventCallback(lv_obj_t * obj, lv_event_t e)
{
	(void)obj;  /*Unused*/

	/*Get the caller act_item*/
	lv_settings_item_t * act_item = (lv_settings_item_t *)lv_event_get_data();

	static char hours_value[3];
	static char mins_value[3];
	static char day_value[3];
	static char month_value[3];
	static char year_value[5];

	static lv_settings_item_t timeItems[] =
	{
			{.type = LV_SETTINGS_TYPE_LIST_BTN, .name = "Set", .value = ""},
			{.type = LV_SETTINGS_TYPE_SLIDER,.name = "Hours",.value = hours_value,.rangeMin = 0,.rangeMax = 23},
			{.type = LV_SETTINGS_TYPE_SLIDER,.name = "Minutes",.value = mins_value,.rangeMin = 0,.rangeMax = 59},
			{.type = LV_SETTINGS_TYPE_SLIDER,.name = "Day",.value = day_value,.rangeMin = 1,.rangeMax = 31},
			{.type = LV_SETTINGS_TYPE_SLIDER,.name = "Month",.value = month_value,.rangeMin = 1,.rangeMax = 12},
			{.type = LV_SETTINGS_TYPE_SLIDER,.name = "Year",.value = year_value,.rangeMin = 0,.rangeMax = 30},
			{.type = LV_SETTINGS_TYPE_INV},    /*Mark the last item*/
	};

	if (e == LV_EVENT_REFRESH) {
		/*Add the items*/

		CalendarDate_t date;
		Calendar_GetDate(&date);

		strftime(hours_value, 3, "%H", &date);
		strftime(mins_value, 3, "%M", &date);
		strftime(day_value, 3, "%d", &date);
		strftime(month_value, 3, "%m", &date);
		strftime(year_value, 5, "%Y", &date);

		timeItems[1].state = date.tm_hour;
		timeItems[2].state = date.tm_min;
		timeItems[3].state = date.tm_mday;
		timeItems[4].state = date.tm_mon;
		timeItems[5].state = date.tm_year;


		uint32_t i;
		for (i = 0; timeItems[i].type != LV_SETTINGS_TYPE_INV; i++) {
			lv_settings_add(&timeItems[i]);
		}
	}
	else if (e == LV_EVENT_VALUE_CHANGED) {

		int i = act_item - &timeItems[0];

		if(i==5)
			sprintf(act_item->value, "%d", 2000+act_item->state);
		else
			sprintf(act_item->value, "%d", act_item->state);

		lv_settings_refr(act_item);

	}
	else if (e == LV_EVENT_CLICKED) {
		if (&timeItems[0] == act_item) {

			CalendarDate_t date;
			date.tm_sec = 0;
			date.tm_hour = timeItems[1].state;
			date.tm_min = timeItems[2].state;
			date.tm_mday = timeItems[3].state;
			date.tm_mon = timeItems[4].state;
			date.tm_year = 2000 + timeItems[5].state;
			date.tm_wday = 0;
			date.tm_yday = 0;
			date.tm_isdst = 0;

			char buffer[80];
			strftime(buffer, 80, "%H:%M - %d/%m/%Y", &date);

			printf("Set date: %s\n",buffer);

			Calendar_SetDate(&date);

		}
	}
}

static void GUI_SettingsScreenDisplayEventCallback(lv_obj_t * btn, lv_event_t e)
{
	(void)btn;  /*Unused*/

	static char brightness_str[5] = { "0%" };
	static lv_settings_item_t items[] =
	{
				{.type = LV_SETTINGS_TYPE_SLIDER,.name = "Backlight brightness",.value = brightness_str,.state = 0 },
				{.type = LV_SETTINGS_TYPE_DDLIST,.name = "Backlight timeout",.value = "1\n5\n10\n15\n30\n60",.state = 0 },
				{.type = LV_SETTINGS_TYPE_INV}
	};

	/*Add items to the submenus*/
	if (e == LV_EVENT_REFRESH) {

		uint32_t i;
		for (i = 0; items[i].type != LV_SETTINGS_TYPE_INV; i++) {
			lv_settings_add(&items[i]);
		}
	}
	/*Handle the items' events*/
	else if (e == LV_EVENT_VALUE_CHANGED) {
		/*Get the caller item*/
		lv_settings_item_t* act_item = (lv_settings_item_t*)lv_event_get_data();

		if (act_item == &items[0])
		{
			act_item->state *= 4;
			snprintf(act_item->value, 5, "%d%", (act_item->state*100)/255);
			lv_settings_refr(act_item);
		}
		else if (act_item == &items[1])
		{
			static int timeoutList[] = { 1,5,10,15,30,60 };
			int timeout = timeoutList[act_item->state];
			PRINTF("Backlight timeout: %d\n", timeout);
		}

	}
}

static void submenu_event_cb(lv_obj_t * btn, lv_event_t e)
{
	(void)btn;  /*Unused*/

	/*Get the caller item*/
	lv_settings_item_t * act_item = (lv_settings_item_t *)lv_event_get_data();

	/*Add items to the submenus*/
	if (e == LV_EVENT_REFRESH) {
		if (strcmp("Button", act_item->name) == 0) {
			static lv_settings_item_t item = {
					.type = LV_SETTINGS_TYPE_BTN,
					.name = "System test",
					.value = "Start" };
			lv_settings_add(&item);
		}
		else if (strcmp("Slider", act_item->name) == 0) {

			static char value[16] = { "100 V" };
			static lv_settings_item_t item = {
					.type = LV_SETTINGS_TYPE_SLIDER,
					.name = "Voltage",
					.value = value,
					.state = 0 };

			lv_settings_add(&item);
		}
		else if (strcmp("Switch", act_item->name) == 0) {

			static lv_settings_item_t item = {
					.type = LV_SETTINGS_TYPE_SW,
					.name = "Valve",
					.value = "Closed",
					.state = 0 };

			lv_settings_add(&item);
		}
		else if (strcmp("Number set", act_item->name) == 0) {
			static char value[16] = { "12" };
			static lv_settings_item_t item = {
					.type = LV_SETTINGS_TYPE_NUMSET,
					.name = "Age",
					.value = value,
					.state = 12 };

			lv_settings_add(&item);
		}
		else if (strcmp("Drop down list", act_item->name) == 0) {
			static lv_settings_item_t item = {
					.type = LV_SETTINGS_TYPE_DDLIST,
					.name = "Size",
					.value = "S\nM\nL\nXL",
					.state = 0 };

			lv_settings_add(&item);
		}
	}
	/*Handle the items' events*/
	else if (e == LV_EVENT_VALUE_CHANGED) {
		if (strcmp("Voltage", act_item->name) == 0) {
			sprintf(act_item->value, "%d V", act_item->state);
			lv_settings_refr(act_item);
		}
		else if (strcmp("Age", act_item->name) == 0) {
			if (act_item->state < 6) act_item->state = 6;
			if (act_item->state > 99) act_item->state = 99;

			sprintf(act_item->value, "%d", act_item->state);
			lv_settings_refr(act_item);
		}
		else if (strcmp("Valve", act_item->name) == 0) {
			if (act_item->state) act_item->value = "Open";
			else act_item->value = "Close";

			lv_settings_refr(act_item);
		}
		else if (strcmp("Size", act_item->name) == 0) {
			printf("Size: %d\n", act_item->state);
		}
	}
	else if (e == LV_EVENT_CLICKED) {
		if (strcmp("System test", act_item->name) == 0) {
			if (strcmp(act_item->value, "Start") == 0) {
				act_item->value = "Stop";
			}
			else {
				act_item->value = "Start";
			}
			lv_settings_refr(act_item);
		}
	}
}


/**
 * Open a new folder, relative to gui.browserPath.
 */
static void GUI_ShowScreen(ScreenID_t ID)
{
	/* Hide current screen. */
	if (gui.currentScreen != NULL)
		lv_obj_set_hidden(gui.currentScreen->parent, true);

	/* Get new screen pointer. */
	Screen_t* newScreen = gui.screens[ID];

	lv_obj_set_hidden(newScreen->parent, false);

	/* Redirect input to screens group of objects. */
	lv_indev_set_group(gui.encoder, newScreen->group);
	lv_indev_set_group(gui.keypad, newScreen->group);

	/* Store current screen. */
	gui.currentScreen = newScreen;

}

/**
 * Open a new folder, relative to gui.browserPath.
 */
static void GUI_OpenFolder(char* folder)
{
	if (strcmp(folder, ".") == 0)
		return;


	// Now build the new path
	char newPath[255];
	strcpy(newPath, gui.browserPath);
	int depthInc = 0;

	// Go up one level
	if (strcmp(folder, "..") == 0)
	{
		depthInc = -1;

		if (gui.browserDepth == 1)
			newPath[0] = '\0';
		else
			*(strrchr(newPath, '\\')) = '\0';
	}
	// Go into a sub-folder
	else
	{
		depthInc = 1;

		/* Redirect SD path. */
		if (strcmp(folder, "SD") == 0)
#if defined(_WIN64) || defined(_WIN32)
			strcat(newPath, "Music");
#else
			strcat(newPath, "0:/");
#endif
		/* Redirect USB path. */
		else if (strcmp(folder, "USB") == 0)
#if defined(_WIN64) || defined(_WIN32)
			strcat(newPath, "Music");
#else
			strcat(newPath, "1:/");
#endif
		/* Folow folder path. */
		else
		{
			if (gui.browserDepth != 0)
#if defined(_WIN64) || defined(_WIN32)
				strcat(newPath, "\\");
#else
			strcat(newPath, "/");
#endif
			strcat(newPath, folder);
		}

	}


	bool status = GUI_ListFolderContents(newPath);

	/* If listing succeeded update browserPath and browserDepth*/
	if (status == true)
	{
		strcpy(gui.browserPath, newPath);
		gui.browserDepth = gui.browserDepth + depthInc;
	}
	else
	{
		/* If listing failed keep on current path.*/
		status = GUI_ListFolderContents(gui.browserPath);
		assert(status==true);
	}

	//lv_group_focus_obj(gui.browserScreen.list);
	lv_group_set_editing(gui.browserScreen.group, true);
}

/**
 * List folder contents in browser screen
 */
static bool GUI_ListFolderContents(char* path)
{
	lv_obj_t* button = NULL;

	/* Delete all previous objects from the list. */
	lv_list_clean(gui.browserScreen.list);

	/* If root folder, only show drives in list. */
	if (*path == '\0')
	{
		/* Add button to go back to menu screen. */
		button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_NEW_LINE, "Back");
		button->user_data = NULL;
		lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);

		button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_SD_CARD, "SD");
		button->user_data = lv_mem_alloc(sizeof(EntryType_t));
		*(EntryType_t*)button->user_data = ENTRY_FOLDER;
		lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);

		if (FE_DriveStatus(FE_SD) == false)
			lv_btn_set_state(button, LV_BTN_STATE_INA);

		button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_USB, "USB");
		button->user_data = lv_mem_alloc(sizeof(EntryType_t));
		*(EntryType_t*)button->user_data = ENTRY_FOLDER;
		lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);

		if (FE_DriveStatus(FE_USB) == false)
			lv_btn_set_state(button, LV_BTN_STATE_INA);

		return true;
	}

	/* Sort folder.*/
	uint32_t folderIndex[MAX_FILES_PER_DIR];
	int32_t nFiles = FE_Sort(SORT_ALPHABETIC, path, folderIndex);
	int32_t nSongs = 0;
	if (nFiles > 0)
	{
#if defined(_WIN64) || defined(_WIN32)
		// El mismo sistema de archivos tiene el directorio ".." !!
#else
		///* If succeeded, add button to go up one level in folder structure. */
		button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_DIRECTORY, "..");
		button->user_data = lv_mem_alloc(sizeof(EntryType_t));
		*(EntryType_t*)button->user_data = ENTRY_FOLDER;
		//lv_group_add_obj(gui.browserScreen.group, button);
		lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);
#endif


		FILINFO de;
		for (int i = 0; i < nFiles; i++)
		{
			FE_GetFileN(path, folderIndex[i], &de);
			if(FE_IS_HIDDEN(&de) == false)
			{
				if (FE_IS_FOLDER(&de))
				{
					PRINTF("Directory: %s\\%s\n", path, FE_ENTRY_NAME(&de));

					button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_DIRECTORY, FE_ENTRY_NAME(&de));

					DirEntry_t* direntry = (DirEntry_t*)lv_mem_alloc(sizeof(DirEntry_t));
					direntry->type = ENTRY_FOLDER;
					direntry->index = folderIndex[i];
					button->user_data = direntry;
				}
				else if (FE_ENTRY_NAME(&de) != NULL)
				{
					char name[150];

					strcpy(name, FE_ENTRY_NAME(&de));

					char* ext = strrchr(name, '.');
					if(ext!=NULL)
					{
						*ext++ = '\0';
						// Lowercase extension
						char* p = ext;
						for (; *p; ++p)
							*p = tolower(*p);
					}
					if(ext!=NULL && strcmp(ext, "mp3") == 0)
					{
						button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_AUDIO, name);

						DirEntry_t* direntry = (DirEntry_t*)lv_mem_alloc(sizeof(DirEntry_t));
						direntry->type = ENTRY_SONG;
						direntry->index = nSongs;
						gui.browserScreen.songsIndex[nSongs++] = folderIndex[i];
						button->user_data = direntry;
	#if defined(_WIN32) || defined(_WIN64)
						PRINTF("Song: %s\\%s\n", path, FE_ENTRY_NAME(&de));
	#endif
					}
					else
					{
						button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_FILE, FE_ENTRY_NAME(&de));
						lv_obj_set_click(button, false);
						lv_btn_set_state(button, LV_BTN_STATE_INA);

						DirEntry_t* direntry = (DirEntry_t*)lv_mem_alloc(sizeof(DirEntry_t));
						direntry->type = ENTRY_FILE;
						direntry->index = -1;
						button->user_data = direntry;
	#if defined(_WIN32) || defined(_WIN64)
						PRINTF("File: %s\\%s\n", path, FE_ENTRY_NAME(&de));
	#endif
					}
				}

				//lv_group_add_obj(gui.browserScreen.group, button);
				lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);
			}

		}
		gui.browserScreen.nSongs = nSongs;
		return true;
	}
	else
		return false;


	// VERSION SIN SORTEAR
#if 0
	/* Try to open directory. */
	DIR * dr;
	FRESULT res = FE_OpenDir(&dr, path);
	if (res != 0)
	{
		PRINTF("Path not found: [%s]\n", path);
		return false;
	}

	/* If succeeded, add button to go up one level in folder structure. */
	button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_DIRECTORY, "..");
	button->user_data = lv_mem_alloc(sizeof(EntryType_t));
	*(EntryType_t*)button->user_data = ENTRY_FOLDER;
	//lv_group_add_obj(gui.browserScreen.group, button);
	lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);

	/* Now iterate through folder adding files and subfolders entries. */
	FILINFO* de;
	int dirIndex = 0;
	while (FE_ReadDir(dr, &de) == 0)
	{
		if (strcmp(FE_ENTRY_NAME(de), ".") != 0
			&& strcmp(FE_ENTRY_NAME(de), "..") != 0)
		{
			//Is the entity a File or Folder?
			if (FE_IS_FOLDER(de))
			{
				PRINTF("Directory: %s\\%s\n", path, FE_ENTRY_NAME(de));

				//ListDirectoryContents(.....); //Recursion, I love it!
				button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_DIRECTORY, FE_ENTRY_NAME(de));

				DirEntry_t* direntry = (DirEntry_t*)lv_mem_alloc(sizeof(DirEntry_t));
				direntry->type = ENTRY_FOLDER;
				direntry->index = dirIndex;
				button->user_data = direntry;
			}
			else if (FE_ENTRY_NAME(de) != NULL)
			{
				char name[150];
				char ext[5];

				char* entryName = FE_ENTRY_NAME(de);
				char* dot = strrchr(entryName, '.');

				*dot = '\0'; // OJO, ACA MODIFICO LA ESTRUCTURA FILINFO * de

				strcpy(ext, dot + 1);
				strcpy(name, entryName);

				// Lowercase extension
				char* p = ext;
				for (; *p; ++p)* p = tolower(*p);

				if (strcmp(ext, "mp3") == 0)
				{
					button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_AUDIO, name);

					DirEntry_t* direntry = (DirEntry_t*)lv_mem_alloc(sizeof(DirEntry_t));
					direntry->type = ENTRY_SONG;
					direntry->index = dirIndex++;
					button->user_data = direntry;
					PRINTF("Song: %s\\%s\n", path, FE_ENTRY_NAME(de));
				}
				else
				{
					button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_FILE, FE_ENTRY_NAME(de));
					lv_obj_set_click(button, false);
					lv_btn_set_state(button, LV_BTN_STATE_INA);

					DirEntry_t* direntry = (DirEntry_t*)lv_mem_alloc(sizeof(DirEntry_t));
					direntry->type = ENTRY_FILE;
					direntry->index = -1;
					button->user_data = direntry;
					PRINTF("File: %s\\%s\n", path, FE_ENTRY_NAME(de));
				}
			}

			//lv_group_add_obj(gui.browserScreen.group, button);
			lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);
		}
	}


	FE_CloseDir(dr);
#endif // VERSION SIN SORTEAR
	return true;

	//	/* Iterate through folder adding files and subfolders entries. */
	//#if defined(_WIN32) || defined(_WIN64)
	//
	//	WIN32_FIND_DATA fdFile;
	//	HANDLE hFind = NULL;
	//
	//	char wildcard[255];
	//	sprintf(wildcard, "%s\\%s", path, "*.*");
	//	if ((hFind = FindFirstFile(wildcard, &fdFile)) == INVALID_HANDLE_VALUE)
	//	{
	//		PRINTF("Path not found: [%s]\n", path);
	//		return false;
	//	}
	//
	//	/* Add button to go up one level in folder structure. */
	//	button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_DIRECTORY, "..");
	//	button->user_data = lv_mem_alloc(sizeof(EntryType_t));
	//	*(EntryType_t*)button->user_data = ENTRY_FOLDER;
	//	//lv_group_add_obj(gui.browserScreen.group, button);
	//	lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);
	//
	//	do
	//	{
	//
	//		//Find first file will always return "."
	//		//    and ".." as the first two directories.
	//		if (strcmp(fdFile.cFileName, ".") != 0
	//			&& strcmp(fdFile.cFileName, "..") != 0)
	//		{
	//			//Is the entity a File or Folder?
	//			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	//			{
	//				PRINTF("Directory: %s\\%s\n", path, fdFile.cFileName);
	//
	//				//ListDirectoryContents(.....); //Recursion, I love it!
	//				button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_DIRECTORY, fdFile.cFileName);
	//				button->user_data = lv_mem_alloc(sizeof(EntryType_t));
	//				*(EntryType_t*)button->user_data = ENTRY_FOLDER;
	//			}
	//			else if (fdFile.cFileName)
	//			{
	//				char name[250];
	//				char ext[5];
	//				_splitpath(fdFile.cFileName, NULL, NULL, name, ext);
	//
	//				if (strcmp(ext, "mp3"))
	//				{
	//					button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_AUDIO, name);
	//					button->user_data = lv_mem_alloc(sizeof(EntryType_t));
	//					*(EntryType_t*)button->user_data = ENTRY_SONG;
	//					PRINTF("Song: %s\\%s\n", path, fdFile.cFileName);
	//				}
	//				else
	//				{
	//					button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_FILE, fdFile.cFileName);
	//					lv_obj_set_click(button, false);
	//					lv_btn_set_state(button, LV_BTN_STATE_INA);
	//					button->user_data = lv_mem_alloc(sizeof(EntryType_t));
	//					*(EntryType_t*)button->user_data = ENTRY_FILE;
	//					PRINTF("File: %s\\%s\n", path, fdFile.cFileName);
	//				}
	//			}
	//
	//			//lv_group_add_obj(gui.browserScreen.group, button);
	//			lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);
	//		}
	//	} while (FindNextFile(hFind, &fdFile)); //Find the next file.
	//
	//	FindClose(hFind); //Always, Always, clean things up!
	//
	//	return true;
	//#else
	//
	//#endif
}



/**
 * @brief Update header information
 * Called periodically within a task, updates time information
 * battery level, etc.
 */
static void GUI_UpdateHeader(lv_task_t * task)
{
	(void)task;

	lv_label_set_static_text(gui.header.time, GUI_GetTimeString());
}

static void GUI_VolumeBarEventHandler(lv_obj_t* slider, lv_event_t event)
{
	if (event == LV_EVENT_VALUE_CHANGED)
	{
		/* Reset timeout. */
		lv_task_reset(gui.volumeBarTask);

		/* Set device volume. */
		//PRINTF("Volume: %u\n", lv_slider_get_value(slider));
#if defined(_WIN32) || defined(_WIN64)
		MP3_SetVolume(lv_slider_get_value(slider)*4);
#else
		MP3_SetVolume(lv_slider_get_value(slider));
#endif
	}
}

static void GUI_VolumeBarTimeoutTask(lv_task_t* task)
{
	(void)task;
	lv_obj_set_hidden(gui.musicScreen.volumeBar,true);
	lv_group_focus_obj(gui.musicScreen.parent);
	//lv_group_set_editing(gui.musicScreen.group, false);

}

static void GUI_ShowVolumeBar()
{
	lv_obj_set_hidden(gui.musicScreen.volumeBar,false);

	/* Focus the slider and set to editing mode. */
	lv_group_focus_obj(gui.musicScreen.volumeBar);
	//lv_group_set_editing(gui.musicScreen.group, true);


	/* Create task to hide volume bar automatically */
	gui.volumeBarTask = lv_task_create(GUI_VolumeBarTimeoutTask, 1400, LV_TASK_PRIO_LOW, NULL);
	lv_task_once(gui.volumeBarTask);

}


/**
 * @brief Display current track information in music tab.
 * TODO: Read Id3 tags from track!
 */
static void GUI_SetTrackInfo(char * _fileName)
{
	// Set elapesed time to 0
	lv_label_set_text(gui.musicScreen.elapsedTime, "0:00");

	// Set remaining time to song duration
	uint32_t seconds = MP3_GetTrackDuration();
	char duration[7];
	sprintf(duration, "%d,%d", seconds / 60, seconds % 60);
	lv_label_set_text(gui.musicScreen.remainingTime, duration);

	// Read track metadata
	//if (hasMetadata)
	//{
		//lv_label_set_text(gui.musicScreen.Title, "Californication");
		//lv_label_set_text(gui.musicScreen.Album, "Red Hot Chili Peppers");
		//lv_label_set_text(gui.musicScreen.Artist, "Stadium Arcadium");
		//lv_label_set_text(gui.musicScreen.Year, "2013");
	//}
	//else
	{
		char fileName[255];
		strcpy(fileName, _fileName);

		/* Delete extension. */
		char* dot = strrchr(fileName, '.');
		if(dot != NULL)
			*dot = '\0';

		/** Try to identify artist from filename looking for
		a '-' separation token */
		char* tok = strchr(fileName, '-');
		if (tok != NULL)
		{
			// Separate artist and song title
			*tok = '\0';
			char* title = tok + 1;
			// Delete leading spaces in title
			while (*title == ' ') title++;
			// Delete trailing spaces in artist
			tok--;
			while (*tok == ' ') *tok-- = '\0';

			lv_label_set_text(gui.musicScreen.Title, title);
			lv_label_set_text(gui.musicScreen.Artist, fileName);
		}
		else
		{
			lv_label_set_text(gui.musicScreen.Title, fileName);
			lv_label_set_text(gui.musicScreen.Artist, "");
		}
	}

	// Check for albumart
	//if (hasAlbumArt)
	{
	}
	//else
	{
		lv_img_set_src(gui.musicScreen.albumArt, &defaultAlbumArt);
	}
	//if(MP3_GetPlayMode()==MP3_RepeatAll)
	{
		char text[7];
		sprintf(text, "%d/%d",MP3_GetSongNumber(),MP3_GetQueueLength());
		lv_label_set_text(gui.musicScreen.queueProgress, text);
	}
}

/**
 * @brief Periodically update track progress
 */
static void GUI_UpdateProgressBar(lv_task_t* task)
{
	(void)task;

	if(MP3_GetStatus()==PLAYING)
	{
		uint32_t elapsed = MP3_GetPlaybackTime();
		uint32_t remaining = MP3_GetTrackDuration() - elapsed;

		// 
		char text[7];
		sprintf(text, "%d:%02d", elapsed / 60, elapsed % 60);
		lv_label_set_text(gui.musicScreen.elapsedTime, text);

		//
		sprintf(text, "%d:%02d", remaining / 60, remaining % 60);
		lv_label_set_text(gui.musicScreen.remainingTime, text);

		lv_bar_set_value(gui.musicScreen.progressBar, elapsed*BAR_WIDTH/(remaining+elapsed), true);
	}
}

/**
 * @brief Set the status of a drive
 * @param[in] drive Drive number: SD=0, USB=1
 * @param[in] status true: Drive is mounted false: Drive is unavailable
 */
void GUI_UpdateDriveStatus(uint8_t drive, bool status)
{
	/* If root folder with drives is currently shown, update it. */
	if (gui.currentScreen == (Screen_t*)& gui.browserScreen)
	{
		if (gui.browserDepth == 0)
		{
			GUI_ListFolderContents("\0");
		}
	}
	else if(gui.browserPath[0]==('0'+drive) && status == false)
	{
		gui.browserPath[0] = '\0';
		gui.browserDepth = 0;
		GUI_ListFolderContents("\0");
	}
}

/**
* 
*/
static void GUI_BorderlessStyleModCB(struct _lv_group_t * objGroup, lv_style_t * focusStyle)
{
	//focusStyle->body.border.color = LV_COLOR_SILVER;
	//focusStyle->body.border.opa = 0;
	focusStyle->body.border.width = 0;

	//focusStyle->body.grad_color = LV_COLOR_WHITE;
	//focusStyle->body.main_color = LV_COLOR_WHITE;
	//focusStyle->body.opa = 0;
}

/**
* Get time string 
*/
static char * GUI_GetTimeString(void)
{
	static char guiTimeTxtBuffer[GUI_TIME_STRING_BUFFER_LENGTH];
	static char guiTimeStrng[GUI_TIME_STRING_BUFFER_LENGTH];


#if defined(_WIN64) || defined(_WIN32)
	
	GetTimeFormatA(LOCALE_CUSTOM_DEFAULT, TIME_NOSECONDS, NULL, NULL, guiTimeStrng, GUI_TIME_STRING_BUFFER_LENGTH);

#else

	CalendarDate_t time;

	Calendar_GetDate(&time);

	const char * format[] ={"%H:%M","%H %M"};

	int i = time.tm_sec%2==0;

	strftime(guiTimeStrng, GUI_TIME_STRING_BUFFER_LENGTH, format[i],&time);

	//strftime(guiTimeStrng, GUI_TIME_STRING_BUFFER_LENGTH, "%H:%M - %d/%m/%Y", &date);
#endif
	return guiTimeStrng;
}


/**
 * Get the last pressed or released character from the keyboard
 * @param indev_drv pointer to the related input device driver
 * @param data store the read data here
 * @return false: because the points are not buffered, so no more data to be read
 */
static bool GUI_KeyPadRead(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    (void) indev_drv;      /*Unused*/

#if defined(_WIN64) || defined(_WIN32)
	uint32_t lastKey = keyboard_get_last_key();
#endif

#if defined(_WIN64) || defined(_WIN32)
	if (lastKey == SDLK_RIGHT)
#else
    if(Input_ReadNextButton() == 0)
#endif
    {
    	data->state = LV_INDEV_STATE_PR;
    	if(gui.currentScreen == gui.screens[MUSIC_SCREEN])
    		data->key = LV_KEY_RIGHT;
		else if (gui.currentScreen == gui.screens[SETTINGS_SCREEN])
			data->key = LV_KEY_UP;
    	else
    		data->key = LV_KEY_NEXT;
    }
#if defined(_WIN64) || defined(_WIN32)
	if (lastKey == SDLK_LEFT)
#else
    else if(Input_ReadPrevButton() == 0)
#endif
    {
    	data->state = LV_INDEV_STATE_PR;
    	if(gui.currentScreen == gui.screens[MUSIC_SCREEN])
    		data->key = LV_KEY_LEFT;
		else if (gui.currentScreen == gui.screens[SETTINGS_SCREEN])
			data->key = LV_KEY_DOWN;
    	else
    		data->key = LV_KEY_PREV;
    }
#if defined(_WIN64) || defined(_WIN32)
	if (lastKey == SDLK_UP)
#else
	else if (Input_ReadMenuButton() == 0)
#endif
	{
		data->state = LV_INDEV_STATE_PR;
		if (gui.currentScreen == gui.screens[SETTINGS_SCREEN])
			data->key = LV_KEY_PREV;
		else
			data->key = LV_KEY_UP;
	}
#if defined(_WIN64) || defined(_WIN32)
	if (lastKey == SDLK_DOWN)
#else
	else if (Input_ReadPlayButton() == 0)
#endif
	{
		data->state = LV_INDEV_STATE_PR;
		if (gui.currentScreen == gui.screens[SETTINGS_SCREEN])
			data->key = LV_KEY_NEXT;
		else
			data->key = LV_KEY_DOWN;
	}
#if defined(_WIN64) || defined(_WIN32)
	if(lastKey == SDLK_KP_ENTER || lastKey == '\r')
#else
	else if(Input_ReadSelectButton() == 0)
#endif
       {
       	data->state = LV_INDEV_STATE_PR;
       	data->key = LV_KEY_ENTER;
       }
#if defined(_WIN64) || defined(_WIN32)
	data->state = keyboard_get_state();
#endif
    return false;
}

static bool GUI_EncoderRead(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
	(void) indev_drv;      /*Unused*/

	static uint8_t btnState = 1;

	if(Input_ReadEncoderButton() != btnState)
	{
		// This line is intentionally before flipping btnState
		// because indev driver is logic high.
		data->state = btnState;
		data->key = LV_KEY_ENTER;
		btnState ^= 1;
	}

	static int8_t encoderPrevCount;
	int8_t encoderCount = Input_ReadEncoderCount();

	data->enc_diff = (encoderPrevCount - encoderCount)/4;

	if(data->enc_diff != 0)
	{
		PRINTF("Enc diff: %d\n",data->enc_diff);
	}
	encoderPrevCount = encoderCount;

	return false;       /*No more data to read so return false*/
}

