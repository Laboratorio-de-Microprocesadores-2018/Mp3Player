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
#define GUI_TOP_HEADER_VER_SIZE	LV_VER_RES*0.1

/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////
enum
{
	HOME_BTN,
	SEARCH_BTN,
	LIBRARY_BTN,
	SETTINGS_BTN,
};
typedef uint8_t btn_clicked;
typedef void (*gui_display_init) (void);
typedef void(*gui_indev_read) (void);

/////////////////////////////////////////////////////////////////////////////////
//                         Global function prototypes                          //
/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Initialize the GUI, create the front end structure and upload it to the screen
 *
 * @param Hardware abstraction layer function pointer, to be executed by the GUI Initialization. This is send a parameter, because the "lv_indev_t obj" (The input devices to be registered by the GUI are defined inside the scope of GUI.c)
 */
void GUI_Init(void(*hal_init)(lv_indev_t ** indev));
void update_header(void);

/**********************
*      MACROS
**********************/

#endif /* GUI_H_ */
