#pragma once
/**
 * @file GUI.h
 * @brief
 *
 *
 */
#ifndef GUI_H_
#define GUI_H_

/////////////////////////////////////////////////////////////////////////////////
//                             Included header files                           //
/////////////////////////////////////////////////////////////////////////////////

#include "GILI9341.h"
#include "Input.h"


/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////
#define GUI_TOP_HEADER_HOR_SIZE	LV_HOR_RES
#define GUI_TOP_HEADER_VER_SIZE	LV_VER_RES * 0.1

#define GUI_TAB_VIEW_HOR_SIZE	LV_HOR_RES
#define GUI_TAB_VIEW_VER_SIZE	LV_VER_RES * 0.9

/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////
typedef void(*GUI_hal_init_cb_t)(lv_indev_t ** indev);

typedef enum {HOME_TAB_ID = 0, SEARCH_TAB_ID, LIBRARY_TAB_ID} GUI_tab_id_t;
typedef struct
{

	lv_disp_drv_t dispDrv;
	lv_disp_t * disp;
	lv_indev_t * encIndev, * keyPadIndev;
	lv_theme_t * theme;

	lv_obj_t * screen;

	lv_obj_t * header;
	lv_obj_t * headerTimeTxt, * headerBatteryTxt;

	lv_obj_t * tabView;
	lv_obj_t * homeTab, * searchTab, * libraryTab;

	lv_group_t * tabGroup;
	GUI_tab_id_t currTabId;
	uint16_t tabIdCount;

	lv_group_t * libraryGroup;
	lv_obj_t * libraryList;

}GUI;

/////////////////////////////////////////////////////////////////////////////////
//                         Global function prototypes                          //
/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Initialize the GUI, create the front end structure and upload it to the screen
 *
 * @param Hardware abstraction layer function pointer, to be executed by the GUI Initialization. This is send a parameter, because the "lv_indev_t obj" (The input devices to be registered by the GUI are defined inside the scope of GUI.c)
 */
void GUI_Init(void);
void GUI_UpdateHeader(void);
lv_disp_drv_t * GUI_GetDispDrv(void);

/**********************
*      MACROS
**********************/

#endif /* GUI_H_ */
