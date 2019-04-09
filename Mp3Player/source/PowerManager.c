#include "PowerManager.h"
#include "fsl_smc.h"
#include "fsl_llwu.h"
#include "fsl_pmc.h"

#define PM_WAKEUP_PIN 10u	// SW2: PC5

static int callbackCount = 0;
static PM_deinitCallback PM_callbackArray[PM_ATTACHED_MODULE_COUNT];

static void PM_DeinitAttachModules(void);

void PM_EnterLowPowerMode()
{
	PM_DeinitAttachModules();
	// Setup SMC for VLLS power mode
	SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlls);
	// Setup wakeup pin on LLWU
	LLWU_ClearExternalWakeupPinFlag(LLWU, PM_WAKEUP_PIN);
	LLWU_SetExternalWakeupPinMode(LLWU,  PM_WAKEUP_PIN, kLLWU_ExternalPinFallingEdge);

	smc_power_mode_vlls_config_t powerConfig;
	powerConfig.subMode = kSMC_StopSub3;
	powerConfig.enablePorDetectInVlls0=true;

	SMC_PreEnterStopModes();
	SMC_SetPowerModeVlls(SMC, &powerConfig);
	SMC_PostExitStopModes();
}

_Bool PM_Recover()
{
	bool retVal = false;
	if(LLWU_GetExternalWakeupPinFlag(LLWU, PM_WAKEUP_PIN) == true)
	{
		LLWU_ClearExternalWakeupPinFlag(LLWU, PM_WAKEUP_PIN);
		retVal = true;
	}
	if(PMC_GetPeriphIOIsolationFlag(PMC))
		PMC_ClearPeriphIOIsolationFlag(PMC);

	return retVal;
}

void PM_AttachDeinitFunction(PM_deinitCallback fun)
{
	if(callbackCount < PM_ATTACHED_MODULE_COUNT)
	{
		PM_callbackArray[callbackCount] = fun;
		callbackCount++;
	}
}

static void PM_DeinitAttachModules(void)
{
	for(int i = 0 ; i<callbackCount ; i++)
		PM_callbackArray[i]();
}

