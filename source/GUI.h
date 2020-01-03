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
#if defined(_WIN64) || defined(_WIN32)

#else
#include "GILI9341.h"
#include "Input.h"
#endif

/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////


typedef enum {
	MUSIC_SCREEN,
	BROWSER_SCREEN,
	SETTINGS_SCREEN,
	POWER_SCREEN,
	MENU_SCREEN,
	SCREEN_NUM
}ScreenID_t;


/////////////////////////////////////////////////////////////////////////////////
//                         Global function prototypes                          //
/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Initialize the GUI, create the front end structure and upload it to the screen
 *
 * @param Hardware abstraction layer function pointer, to be executed by the GUI Initialization. This is send a parameter, because the "lv_indev_t obj" (The input devices to be registered by the GUI are defined inside the scope of GUI.c)
 */
void GUI_Init(void);

/**
 *
 */
void GUI_Create(void);

/*
 *
 */
void GUI_Task(void);

/*
 *
 */
void GUI_FlushReady(void);

/**********************
*      MACROS
**********************/

#endif /* GUI_H_ */
