/**
 * @file PowerManager.h
 * @brief
 *
 *
 */


#ifndef POWERMANAGER_H_
#define POWERMANAGER_H_

#include <stdbool.h>

#define PM_ATTACHED_MODULE_COUNT 15

/**
 *
 */
typedef void (*PM_deinitCallback)(void);

/**
 *
 */
void PM_EnterLowPowerMode(void);

/**
 *
 * @return True if recovered from Low Power mode. False if normal reset.
 */
bool PM_Recover(void);

/**
 *
 */
void PM_AttachDeinitFunction(PM_deinitCallback fun);

#endif /* POWERMANAGER_H_ */
