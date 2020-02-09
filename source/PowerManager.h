/**
 * @file PowerManager.h
 * @brief
 *
 *
 */


#ifndef POWERMANAGER_H_
#define POWERMANAGER_H_

#include <stdbool.h>

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
