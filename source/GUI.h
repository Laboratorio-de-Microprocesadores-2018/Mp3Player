#pragma once
/**
 * @file GUI.h
 * @brief
 *
 *
 */
#ifndef GUI_H_
#define GUI_H_

 /*********************
  *      INCLUDES
  *********************/


/////////////////////////////////////////////////////////////////////////////////
//                             Included header files                           //
/////////////////////////////////////////////////////////////////////////////////
#include <stdbool.h>
#include <stdint.h>
/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
typedef enum {
	MUSIC_SCREEN,
	BROWSER_SCREEN,
	SETTINGS_SCREEN,
	POWER_SCREEN,
	MENU_SCREEN,
	SCREEN_NUM
}ScreenID_t;


///////////////////////////////////////////////////////////////////////////////
//                                  API                                      //
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Initialize the GUI.
 *
 */
void GUI_Init(void);
void GUI_Deinit(void);
/**
 * @brief Create the front end structure and upload it to the screen
 */
void GUI_Create(void);

/** 
 * @brief Task function, must be called periodically in main loop.
 */
void GUI_Task(void);

/**
 * @brief Notify GUI module that display has been flushed.
 * TODO: To resolve circular dependencies, add an api to GILI9341
 * to register a callback in DMA finish interrupt, and register 
 * this function on init.
 */
void GUI_FlushReady(void);

/**
 * @brief Returns if the user wants to power off the player or not.
 */
bool GUI_PowerOffRequest(void);

void GUI_UpdateDriveStatus(uint8_t drive, bool status);
/**********************
*      MACROS
**********************/

#endif /* GUI_H_ */
