#ifndef POWERMANAGER_H_
#define POWERMANAGER_H_

#define PM_ATTACHED_MODULE_COUNT 15

typedef void (*PM_deinitCallback)(void);

void PM_EnterLowPowerMode();
// Returns true if recovered from Low Power mode. False if normal reset.
_Bool PM_Recover();

void PM_AttachDeinitFunction(PM_deinitCallback fun);

#endif /* POWERMANAGER_H_ */
