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
#include "lv_conf.h"
#include "lvgl/lvgl.h"
#include "MP3Player.h"
#include "FileExplorer.h"

#include <time.h>

#if defined(_WIN64) || defined(_WIN32)
#include "lv_drivers/indev/keyboard.h"
#include "lv_drivers/indev/mousewheel.h"
#include "lv_drivers/display/monitor.h"
#include <windows.h>
#else
#include "Calendar.h"
#endif


 /*********************
 *      DEFINES
 *********************/
#define LV_VDB_BUFF_SIZE (uint32_t)(LV_HOR_RES_MAX*LV_VER_RES_MAX/20)

#define GUI_TOP_HEADER_WIDTH	(LV_HOR_RES)
#define GUI_TOP_HEADER_HEIGHT	(LV_VER_RES * 0.07)

#define GUI_SCREEN_WIDTH		(LV_HOR_RES)
#define GUI_SCREEN_HEIGHT		(LV_VER_RES - GUI_TOP_HEADER_HEIGHT)


 /******************************** MUSIC TAB ************************************/


#define BAR_WIDTH				(GUI_SCREEN_WIDTH  * 0.8)
#define BAR_HEIGHT				(GUI_SCREEN_HEIGHT * 0.05)
#define BAR_SPACING				(GUI_SCREEN_HEIGHT * 0.10)

#define ALBUM_ART_SIZE			(200)
#define ALBUM_ART_SPACING		(10)


#define GUI_SCREEN_DECLARE(screenName,objects)  \
							struct				\
							{					\
								Screen_t;		\
								objects			\
							}screenName;		\

#if defined(_WIN64) || defined(_WIN32)
#define MONITOR_INIT		monitor_init()
#define MONITOR_FLUSH_CB	monitor_flush
#define KEYPAD_READ_CB		keyboard_read
#define ENCODER_READ_CB		mousewheel_read
#else
#define MONITOR_INIT		GILI9341_Init()
#define MONITOR_FLUSH_CB	GILI9341_Flush
#define KEYPAD_READ_CB		GUI_KeyPadRead
#define ENCODER_READ_CB		GUI_EncoderRead
#endif


 /**********************
 *      TYPEDEFS
 **********************/

typedef enum {
	MUSIC_TAB,
	EXPLORER_TAB,
	SETTINGS_TAB,
	POWER_TAB,
	TAB_NUM
}TabID_t;

/* */
typedef enum {
	FOLDER,
	SONG,
	FILE,
}EntryType_t;

typedef struct
{
	lv_obj_t* parent;
	lv_group_t* group;
}Screen_t;

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
	lv_indev_t* encIndev;
	lv_indev_t* keyPadIndev;

	/* GUI theme*/
	lv_theme_t* theme;

	/* GUI header, time and battery*/
	struct
	{
		lv_obj_t* parent;
		lv_obj_t* time;
		lv_obj_t * battery;
	}header;

	struct
	{
		Screen_t;
		lv_obj_t* tabs[TAB_NUM];
	}menuScreen;

	/* Current tab*/
	Screen_t * currentScreen;
	
	GUI_SCREEN_DECLARE(
	musicScreen,
	lv_obj_t* progressBar;
	lv_obj_t* elapsedTime;
	lv_obj_t* remainingTime;
	lv_obj_t* Title;
	lv_obj_t* Album;
	lv_obj_t* Artist;
	lv_obj_t* Year;
	lv_obj_t* albumArt;
	lv_obj_t* volumeBar;)

	GUI_SCREEN_DECLARE(
	browserScreen, 
	lv_obj_t* list;
	lv_obj_t* SD;
	lv_obj_t* USB;)

	GUI_SCREEN_DECLARE(
	settingsScreen,)
	
	GUI_SCREEN_DECLARE(
	powerScreen, )
	
	/* Store pointers to base screens to make screen switching easier. */
	Screen_t* screens[SCREEN_NUM];

	/* Stotre file browser path and depth. */
	uint8_t browserPath[255];
	uint8_t browserDepth;

	/* Task to make volume bar hide automatically. */
	lv_task_t* volumeBarTask;

}GUI;


 /**********************
 *  STATIC PROTOTYPES
 **********************/
 /**
 * Creation and definition of GUI structure
 */
static void GUI_HalInit(void);

/**
 *
 */
static void GUI_CreateHeader(void);
static void GUI_CreateMenuScreen(void);
static void GUI_CreateMusicScreen(void);
static void GUI_CreateBrowserScreen(void);
static void GUI_CreateSettingsScreen(void);
static void GUI_CreatePowerScreen(void);

/**
 *
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
 *	Style definition callback, set no border
 */
static void GUI_BorderlessStyleModCB(struct _lv_group_t * objGroup, lv_style_t * focusStyle);


/**
* Event handler call backs
*/
static void GUI_MusicScreenEventHandler(lv_obj_t* obj, lv_event_t event);
static void GUI_BrowserScreenEventHandler(lv_obj_t* obj, lv_event_t event);
static void GUI_SettingsScreenEventHandler(lv_obj_t* obj, lv_event_t event);
static void GUI_PowerScreenEventHandler(lv_obj_t* obj, lv_event_t event);
static void GUI_MenuEventHandler(lv_obj_t* obj, lv_event_t event);


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


/**********************
 *  STATIC VARIABLES
 **********************/

/* GUI structure to store information and lvgl objects. */
static GUI gui;


/* Default album art image for tracks without embedded albumart*/
LV_IMG_DECLARE(defaultAlbumArt);

/* Display buffer for lvgl. */
static lv_disp_buf_t dispBuffer;

static LV_ATTRIBUTE_MEM_ALIGN lv_color_t vdb_buf1[LV_VDB_BUFF_SIZE];
static LV_ATTRIBUTE_MEM_ALIGN lv_color_t vdb_buf2[LV_VDB_BUFF_SIZE];


/**********************
*      MACROS
**********************/

#define GUI_TIME_STRING_BUFFER_LENGTH	9


/**********************
*   GLOBAL FUNCTIONS
**********************/
/**
 * @brief Set the status of a drive
 * @param[in] drive Drive number: SD=0, USB=1
 * @param[in] status true: Drive is mounted false: Drive is unavailable
 */
void GUI_UpdateDriveStatus()
{
	/* If root folder with drives is currently shown, update it. */
	if ( gui.currentScreen == (Screen_t*)&gui.browserScreen )
	{
		if (gui.browserDepth == 0)
		{
			GUI_ListFolderContents("\0");
		}
	}
}

/**
* @ Brief Initialize the GUI
*/
void GUI_Init(void)
{

	/*Initialize LittlevGL*/
	lv_init();

	GUI_HalInit();
}

/**
 * @brief GUI task function.
 *		  Must be called periodically
 */
void GUI_Task()
{

	lv_task_handler();

#if !(defined(_WIN64) || defined(_WIN32))
	static uint32_t lastTickTime;

	uint32_t currTime = lv_tick_get();

	if(currTime-lastTickTime >= 1)
	{
		lv_tick_inc(currTime-lastTickTime);
		lastTickTime = currTime;
	}
#endif
}

/**
 * @brief Notifies GUI that screen has been flushed.
 */
void GUI_FlushReady(void)
{
	lv_disp_flush_ready(&gui.dispDrv);
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
#if defined(_WIN32) || defined(_WIN64)
		printf("Volume: %u\n", lv_slider_get_value(slider));
#else
		MP3_SetVolume(lv_slider_get_value(slider));
#endif
	}
}

static void GUI_VolumeBarTimeoutTask(lv_task_t* task)
{
	(void)task;
	lv_obj_del(gui.musicScreen.volumeBar);
	gui.musicScreen.volumeBar = NULL;
}

static void GUI_ShowVolumeBar()
{
	lv_obj_t* bar = lv_slider_create(gui.musicScreen.parent, NULL);
	lv_obj_set_size(bar, BAR_WIDTH, BAR_HEIGHT);
	lv_obj_align(bar, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_event_cb(bar, GUI_VolumeBarEventHandler);
	lv_slider_set_range(bar, 0, 32);
	lv_group_add_obj(gui.musicScreen.group, bar);
	gui.musicScreen.volumeBar = bar;


	/* Focus the slider and set to editing mode. */
	lv_group_focus_obj(bar);
	lv_group_set_editing(gui.musicScreen.group, true);


	/* Create task to hide volume bar automatically */
	gui.volumeBarTask = lv_task_create(GUI_VolumeBarTimeoutTask, 1000, LV_TASK_PRIO_LOW, NULL);
	lv_task_once(gui.volumeBarTask);

}


/**
 * @brief Display current track information in music tab.
 * TODO: Read Id3 tags from track!
 */
static void GUI_SetTrackInfo(char * fileName)
{
	// Set elapesed time to 0 and compute track duration
	lv_label_set_text(gui.musicScreen.elapsedTime, "0:00");

	//
	lv_label_set_text(gui.musicScreen.remainingTime, "4:23");

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
		char artist[255];
		strcpy(artist, fileName);
		char* tok = strchr(artist, '-');
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
			lv_label_set_text(gui.musicScreen.Artist, artist);
		}
		else
			lv_label_set_text(gui.musicScreen.Title, fileName);
	}

	// Check for albumart
	//if (hasAlbumArt)
	{
	}
	//else
	{
		lv_img_set_src(gui.musicScreen.albumArt, &defaultAlbumArt);
	}

}

/**
 * @brief Periodically update track progress
 */
static void GUI_UpdateTrackInfo(lv_task_t* task)
{
	(void)task;

	//lv_label_set_text(gui., GUI_GetTimeString());

	//if(MP3_IsPlaing())
	{
		int elapsed = MP3_GetPlaytime();
		int remaining = MP3_GetDuration() - elapsed;

		// Set elapesed time to 0 and compute track duration
		char text[7];
		sprintf(text, "%d:%02d", elapsed / 60, elapsed % 60);
		lv_label_set_text(gui.musicScreen.elapsedTime, text);

		//
		sprintf(text, "%d:%02d", remaining / 60, remaining % 60);
		lv_label_set_text(gui.musicScreen.remainingTime, text);

		lv_bar_set_value(gui.musicScreen.progressBar, MP3_GetTrackProgress()*BAR_WIDTH/100, TRUE);
	}
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
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
	gui.keyPadIndev = lv_indev_drv_register(&keyPadDrv);

	lv_indev_drv_t encDrv;
	lv_indev_drv_init(&encDrv);
	encDrv.type = LV_INDEV_TYPE_ENCODER;
	encDrv.read_cb = ENCODER_READ_CB;
	gui.encIndev = lv_indev_drv_register(&encDrv);
}

 /**
 * Create a mp3 graphic user interface
 */
void GUI_Create(void)
{
	
	/*Create theme for the GUI. */
	gui.theme = lv_theme_alien_init(2,NULL);
	gui.theme->style.cont->body.radius = 0;
	lv_theme_set_current(gui.theme);

	/*Create header. */
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
	lv_task_t * headerUpdateTask = lv_task_create(GUI_UpdateHeader,1000,LV_TASK_PRIO_LOW,NULL);
	lv_task_create(GUI_UpdateTrackInfo, 1000, LV_TASK_PRIO_LOW, NULL);
	lv_task_ready(headerUpdateTask);
}


static void GUI_CreateMenuScreen(void)
{
	/*Create the tab view for the main menu*/
	lv_obj_t *tabView = lv_tabview_create(lv_scr_act(), NULL);
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
		lv_obj_t * tab = lv_tabview_add_tab(tabView, menuLabels[i]);

		tab->user_data = lv_mem_alloc(sizeof(TabID_t));

		if (tab->user_data != NULL)
			* ((TabID_t*)tab->user_data) = i; //Poner assert

		lv_obj_t * label = lv_label_create(tab, NULL);
		lv_label_set_text(label, menuLabels[i]);
		lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style);
		lv_obj_align(label, tab, LV_ALIGN_CENTER,0,0);
		
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
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
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
	lv_obj_t * albumArt = lv_img_create(cont, NULL);
	lv_obj_set_size(albumArt, ALBUM_ART_SIZE, ALBUM_ART_SIZE);
	lv_obj_align(albumArt, cont, LV_ALIGN_IN_TOP_MID, 0, ALBUM_ART_SPACING);
	lv_img_set_src(albumArt, &defaultAlbumArt);
	gui.musicScreen.albumArt = albumArt;
	
	lv_style_t* labelStyle = gui.theme->style.label.prim;
	//labelStyle->text.font = &lv_font_roboto_28;

	/* Add song title */
	lv_obj_t* Title = lv_label_create(cont, NULL);
	lv_label_set_text(Title, "");
	lv_label_set_style(Title, LV_LABEL_STYLE_MAIN, gui.theme->style.label.sec);
	lv_label_set_long_mode(Title, LV_LABEL_LONG_SROLL_CIRC);
	
	lv_obj_set_size(Title, ALBUM_ART_SIZE,20);
	lv_label_set_align(Title, LV_LABEL_ALIGN_CENTER);
	lv_obj_align(Title, albumArt, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
	gui.musicScreen.Title = Title;

	lv_obj_t* Artist = lv_label_create(cont, Title);
	lv_label_set_style(Title, LV_LABEL_STYLE_MAIN, gui.theme->style.label.prim);
	lv_obj_align(Artist, Title, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
	gui.musicScreen.Artist = Artist;

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
	/* Create a group for this screen.  */
	lv_obj_t* group = lv_group_create();
	lv_group_set_wrap(group, false);
	lv_group_set_style_mod_cb(group, GUI_BorderlessStyleModCB);
	lv_group_set_style_mod_edit_cb(group, GUI_BorderlessStyleModCB);


	lv_obj_t* cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(cont, GUI_SCREEN_WIDTH, GUI_SCREEN_HEIGHT);
	lv_obj_align(cont, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);  
	lv_group_add_obj(group, cont);
	lv_obj_set_event_cb(cont, GUI_SettingsScreenEventHandler);

	// Hide screen for default
	lv_obj_set_hidden(cont, true);

	gui.settingsScreen.parent = cont;
	gui.settingsScreen.group = group;
}



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

	lv_obj_set_event_cb(cont, GUI_PowerScreenEventHandler);

	lv_group_add_obj(group, cont);

	// Hide screen for default
	lv_obj_set_hidden(cont, true);

	gui.powerScreen.parent = cont;
	gui.powerScreen.group = group;
}



/**
 * Open a new folder, relative to gui.browserPath.
 */

static void GUI_OpenFolder(char* folder)
{
	/* Delete all previous objects from the list. */
	lv_list_clean(gui.browserScreen.list);
	
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
			strcat(newPath, "D:\\Music");
		/* Redirect USB path. */
		else if (strcmp(folder, "USB") == 0)
			strcat(newPath, "D:\\Music");
		/* Folow folder path. */
		else
		{
			if (gui.browserDepth != 0)
				strcat(newPath, "\\");
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
		GUI_ListFolderContents(gui.browserPath);
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

	/* If root folder, only show drives in list. */
	if (*path == '\0')
	{
		button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_SD_CARD, "SD");
		button->user_data = lv_mem_alloc(sizeof(EntryType_t));
		*(EntryType_t*)button->user_data = FOLDER;
		lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);
		// TODO: Ask file explorer if drive is mounted
		//lv_btn_set_state(button, LV_BTN_STATE_REL);

		button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_USB, "USB");
		button->user_data = lv_mem_alloc(sizeof(EntryType_t));
		*(EntryType_t*)button->user_data = FOLDER;
		lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);
		//lv_obj_set_click(button, false);
		lv_btn_set_state(button, LV_BTN_STATE_INA);
		return true;
	}

	/* Iterate through folder adding files and subfolders entries. */
#if defined(_WIN32) || defined(_WIN64)

	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char wildcard[255];
	sprintf(wildcard, "%s\\%s", path, "*.*");
	if ((hFind = FindFirstFile(wildcard, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		printf("Path not found: [%s]\n", path);
		return false;
	}

	/* Add button to go up one level in folder structure. */
	button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_DIRECTORY, "..");
	button->user_data = lv_mem_alloc(sizeof(EntryType_t));
	*(EntryType_t*)button->user_data = FOLDER;
	//lv_group_add_obj(gui.browserScreen.group, button);
	lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);

	do
	{

		//Find first file will always return "."
		//    and ".." as the first two directories.
		if (strcmp(fdFile.cFileName, ".") != 0
			&& strcmp(fdFile.cFileName, "..") != 0)
		{
			//Is the entity a File or Folder?
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				printf("Directory: %s\\%s\n", path, fdFile.cFileName);

				//ListDirectoryContents(.....); //Recursion, I love it!
				button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_DIRECTORY, fdFile.cFileName);
				button->user_data = lv_mem_alloc(sizeof(EntryType_t));
				*(EntryType_t*)button->user_data = FOLDER;
			}
			else if (fdFile.cFileName)
			{
				char name[250];
				char ext[5];
				_splitpath(fdFile.cFileName, NULL, NULL, name, ext);

				if (strcmp(ext, "mp3"))
				{
					button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_AUDIO, name);
					button->user_data = lv_mem_alloc(sizeof(EntryType_t));
					*(EntryType_t*)button->user_data = SONG;
					printf("Song: %s\\%s\n", path, fdFile.cFileName);
				}
				else
				{
					button = lv_list_add_btn(gui.browserScreen.list, LV_SYMBOL_FILE, fdFile.cFileName);
					lv_obj_set_click(button, false);
					lv_btn_set_state(button, LV_BTN_STATE_INA);
					button->user_data = lv_mem_alloc(sizeof(EntryType_t));
					*(EntryType_t*)button->user_data = FILE;
					printf("File: %s\\%s\n", path, fdFile.cFileName);
				}
			}

			//lv_group_add_obj(gui.browserScreen.group, button);
			lv_obj_set_event_cb(button, GUI_BrowserScreenEventHandler);
		}
	} while (FindNextFile(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!

	return true;
#else

#endif
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


static void GUI_MenuEventHandler(lv_obj_t* obj, lv_event_t event)
{
	TabID_t tabId = 0;

	if (obj->user_data != NULL)
		tabId = *(TabID_t*)(obj->user_data);

	lv_key_t pressedKey = LV_KEY_HOME;

	switch (event)
	{
	//case LV_EVENT_VALUE_CHANGED:
//		lv_group_focus_obj(obj);
			//break;
	case LV_EVENT_FOCUSED:

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
		pressedKey = gui.keyPadIndev->proc.types.keypad.last_key;
		switch (pressedKey)
		{
		case LV_KEY_ENTER:
			GUI_ShowScreen(tabId);
			break;

		//case LV_KEY_NEXT:
		case LV_KEY_RIGHT:
			if (tabId < TAB_NUM - 1)
			{
				lv_tabview_set_tab_act(gui.menuScreen.parent, tabId + 1, true);
				lv_group_focus_next(gui.menuScreen.group);
			}
			break;

		//case LV_KEY_PREV:
		case LV_KEY_LEFT:
			if (tabId > 0)
			{
				lv_tabview_set_tab_act(gui.menuScreen.parent, tabId - 1, true);
				lv_group_focus_prev(gui.menuScreen.group);
			}
			break;

		}
		break;
	}
}


static void GUI_MusicScreenEventHandler(lv_obj_t* obj, lv_event_t event)
{

	lv_key_t pressedKey = gui.keyPadIndev->proc.types.keypad.last_key;

	switch (event)
	{

	case LV_EVENT_FOCUSED:

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
		switch (pressedKey)
		{
		case LV_KEY_ENTER:
#if defined(_WIN64) || defined(_WIN32)
			printf("MP3_PlayPause()\n");
#else

#endif
			break;

			//case LV_KEY_NEXT:
		case LV_KEY_UP:
			GUI_ShowScreen(MENU_SCREEN);
			break;
		case LV_KEY_DOWN:
		{
			GUI_ShowVolumeBar();			
		}
			break;
		case LV_KEY_RIGHT:
			if (gui.musicScreen.volumeBar != NULL)
			{
#if defined(_WIN64) || defined(_WIN32)
				printf("Volume UP\n");
#else
				MP3_Next();
#endif
				lv_task_reset(gui.volumeBarTask);
			}
			else
			{
#if defined(_WIN64) || defined(_WIN32)
				printf("MP3_Next()\n");
#else
				MP3_Next();
#endif
			}
			break;

			//case LV_KEY_PREV:
		case LV_KEY_LEFT:
			if (gui.musicScreen.volumeBar != NULL)
			{
#if defined(_WIN64) || defined(_WIN32)
				printf("Volume Down\n");
#else
				MP3_Next();
#endif
				lv_task_reset(gui.volumeBarTask);
			}
			else
			{
#if defined(_WIN64) || defined(_WIN32)
				printf("MP3_Prev()\n");
#else
				MP3_Prev();
#endif
			}
			break;

		}
		break;
	}
}


static void GUI_BrowserScreenEventHandler(lv_obj_t* obj, lv_event_t event)
{
	lv_key_t pressedKey = gui.keyPadIndev->proc.types.keypad.last_key;

	switch (event)
	{

	case LV_EVENT_PRESSED:
		{
			switch (*(EntryType_t*)(obj->user_data))
			{
			case FOLDER:
				GUI_OpenFolder(lv_list_get_btn_text(obj));
				break;
			case SONG:

#if defined(_WIN64) || defined(_WIN32)
				printf("MP3_Play(%s\\%s)", gui.browserPath, lv_list_get_btn_text(obj));
				MP3_Play(gui.browserPath);
#else
				MP3_Play(gui.browserPath, 0/*INDICE??*/);
#endif
				/* Show music screen */
				GUI_SetTrackInfo(lv_list_get_btn_text(obj));

				GUI_ShowScreen(MUSIC_TAB);

				break;
			}
		}
		break;

	case LV_EVENT_LONG_PRESSED:
		switch (pressedKey)
		{
		case LV_KEY_UP:
			GUI_ShowScreen(MENU_SCREEN);
			break;
		}
		break;

	case LV_EVENT_DELETE:
		//lv_mem_free(obj->user_data);
		break;
	
	}
}



static void GUI_SettingsScreenEventHandler(lv_obj_t* obj, lv_event_t event)
{
	TabID_t tabId = 0;

	lv_key_t pressedKey = LV_KEY_HOME;

	switch (event)
	{

	case LV_EVENT_FOCUSED:

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
		pressedKey = gui.keyPadIndev->proc.types.keypad.last_key;
		switch (pressedKey)
		{
		case LV_KEY_ENTER:

			break;

			//case LV_KEY_NEXT:
		case LV_KEY_UP:
			GUI_ShowScreen(MENU_SCREEN);
			break;
		case LV_KEY_RIGHT:


			//case LV_KEY_PREV:
		case LV_KEY_LEFT:

			break;
		}
		break;
	}
}

static void GUI_PowerScreenEventHandler(lv_obj_t* obj, lv_event_t event)
{
	TabID_t tabId = 0;

	lv_key_t pressedKey = LV_KEY_HOME;

	switch (event)
	{

	case LV_EVENT_FOCUSED:

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
		pressedKey = gui.keyPadIndev->proc.types.keypad.last_key;
		switch (pressedKey)
		{
		case LV_KEY_ENTER:

			break;

			//case LV_KEY_NEXT:
		case LV_KEY_UP:
			GUI_ShowScreen(MENU_SCREEN);
			break;
		case LV_KEY_RIGHT:

			break;

			//case LV_KEY_PREV:
		case LV_KEY_LEFT:

			break;
		}
		break;
	}
}


static void GUI_ShowScreen(ScreenID_t ID)
{

	/* Hide current screen. */
	if (gui.currentScreen != NULL)
		lv_obj_set_hidden(gui.currentScreen->parent, true);

	/* Get new screen pointer. */
	Screen_t * newScreen = gui.screens[ID];

	/* Show selected screen. */
	lv_obj_set_hidden(newScreen->parent, false);
	
	/* Redirect input to screens group of objects. */
	lv_indev_set_group(gui.encIndev, newScreen->group);
	lv_indev_set_group(gui.keyPadIndev, newScreen->group);

	/* Store current screen. */
	gui.currentScreen = newScreen;

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

	TM_date guiInitialTime = Calendar_GetDate();

	guiTimeStrng[0] = '\n';
	uitoa(guiInitialTime.hour, guiTimeStrng, 10);

	strcat(guiTimeStrng, ":");
	strcat(guiTimeStrng, uitoa(guiInitialTime.minute, guiTimeTxtBuffer, 10));
	strcat(guiTimeStrng, ":");
	strcat(guiTimeStrng, uitoa(guiInitialTime.second, guiTimeTxtBuffer, 10));
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

    if(Input_ReadSelectButton() == 0)
    {
    	data->state = LV_INDEV_STATE_PR;
    	data->key = LV_KEY_ENTER;
    }
    else if(Input_ReadNextButton() == 0)
    {
    	data->state = LV_INDEV_STATE_PR;
    	data->key = LV_KEY_RIGHT;
    }
    else if(Input_ReadPrevButton() == 0)
    {
    	data->state = LV_INDEV_STATE_PR;
    	data->key = LV_KEY_LEFT;
    }
	else if (Input_ReadMenuButton() == 0)
	{
		data->state = LV_INDEV_STATE_PR;
		data->key = LV_KEY_UP;
	}
	else if (Input_ReadPlayPauseButton() == 0)
	{
		data->state = LV_INDEV_STATE_PR;
		data->key = LV_KEY_DOWN;
	}
    else
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

	static uint8_t encoderCount;
	//encoderCount = Input_ReadEncoderCount();
	encoderCount = 0;
	//if (encoderCount != 0)
		data->enc_diff = encoderCount;
	//enc_diff = 0;

	return false;       /*No more data to read so return false*/
}