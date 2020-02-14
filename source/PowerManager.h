/**
 * @file PowerManager.h
 * @brief
 *
 *
 */


#ifndef POWERMANAGER_H_
#define POWERMANAGER_H_

#include <stdbool.h>
#include "board.h"


#define APP_WAKEUP_BUTTON_GPIO BOARD_SW2_GPIO
#define APP_WAKEUP_BUTTON_PORT BOARD_SW2_PORT
#define APP_WAKEUP_BUTTON_GPIO_PIN BOARD_SW2_GPIO_PIN
#define APP_WAKEUP_BUTTON_IRQ_HANDLER BOARD_SW2_IRQ_HANDLER
#define APP_WAKEUP_BUTTON_NAME BOARD_SW2_NAME
#define APP_WAKEUP_BUTTON_IRQ_TYPE kPORT_InterruptFallingEdge
#define APP_WAKEUP_BUTTON_IRQ BOARD_SW2_IRQ
//#define BTN_SELECT_PORT PORTC /*!<@brief PORT device name: PORTC */
//#define BTN_SELECT_PIN 6U     /*!<@brief PORTC pin index: 6 (SW2) */
#define PM_WAKEUP_PIN 6 	  /* SW2: PTC1*/

/**
 *
 */
void PM_EnterLowPowerMode(void);

/**
 *
 * @return True if recovered from Low Power mode. False if normal reset.
 */
bool PM_Recover(void);

#endif /* POWERMANAGER_H_ */
