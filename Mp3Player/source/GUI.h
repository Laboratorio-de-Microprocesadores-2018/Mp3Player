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
#include "lvgl/lvgl.h"
#include "lv_conf.h"

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
typedef enum {HOME_ID = 0, SEARCH_ID, LIBRARY_ID} GUI_tab_id_t;
/*enum
{
	HOME_BTN,
	SEARCH_BTN,
	LIBRARY_BTN,
	SETTINGS_BTN,
};*/
typedef struct
{
	lv_indev_t * indev;
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
/*typedef uint8_t btn_clicked;
typedef void (*gui_display_init) (void);
typedef void(*gui_indev_read) (void);*/



/////////////////////////////////////////////////////////////////////////////////
//                         Global function prototypes                          //
/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Initialize the GUI, create the front end structure and upload it to the screen
 *
 * @param Hardware abstraction layer function pointer, to be executed by the GUI Initialization. This is send a parameter, because the "lv_indev_t obj" (The input devices to be registered by the GUI are defined inside the scope of GUI.c)
 */
void GUI_Init(void(*HalInit)(lv_indev_t ** indev));
void GUI_UpdateHeader(void);

void GUI_EncoderIncCallBack(void * paramPointer);
void GUI_EncoderDecCallBack(void * paramPointer);

void GUI_PrevBtnCallBack(void * paramPointer);
void GUI_NextBtnCallBack(void * paramPointer);
void GUI_MenuBtnCallBack(void * paramPointer);
void GUI_PlayBtnCallBack(void * paramPointer);
void GUI_SelectBtnCallBack(void * paramPointer);



/**********************
*      MACROS
**********************/

#endif /* GUI_H_ */
