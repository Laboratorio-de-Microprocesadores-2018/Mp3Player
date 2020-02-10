/**
 * @file PowerManager.c
 * @brief
 *
 *
 */

#include "PowerManager.h"
#include "pin_mux.h"
#include "fsl_smc.h"
#include "fsl_llwu.h"
#include "fsl_pmc.h"
#include "fsl_rcm.h"
#include "fsl_port.h"
#include "fsl_debug_console.h"



void PM_EnterLowPowerMode()
{
	// Setup wakeup pin on LLWU
//	PORT_SetPinMux(BTN_SELECT_PORT, BTN_SELECT_PIN, kPORT_MuxAsGpio);

	LLWU_ClearExternalWakeupPinFlag(LLWU, PM_WAKEUP_PIN);
	LLWU_SetExternalWakeupPinMode(LLWU,  PM_WAKEUP_PIN, kLLWU_ExternalPinFallingEdge);

	smc_power_mode_vlls_config_t powerConfig;
	powerConfig.subMode = kSMC_StopSub0;

	SMC_PreEnterStopModes();
	SMC_SetPowerModeVlls(SMC, &powerConfig);
	SMC_PostExitStopModes();
}

bool PM_Recover()
{
	SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeAll);

	bool fromLowPower = LLWU_GetExternalWakeupPinFlag(LLWU,PM_WAKEUP_PIN);
	if (fromLowPower) /* Wakeup from VLLS. */
	{
		LLWU_ClearExternalWakeupPinFlag(LLWU, PM_WAKEUP_PIN);
		PMC_ClearPeriphIOIsolationFlag(PMC);
	}
		/* RTC Alarm! */
	/*	else if(LLWU_GetInternalWakeupModuleFlag(LLWU, 5) == true)
		{
			//PRINTF("Wake up with RTC alarm!\n");
		}*/
		/** RTC Seconds */
//		else if(LLWU_GetInternalWakeupModuleFlag(LLWU, 7) == true)
//		{
//
//		}
	//}
	/*else if ( resetCause & (kRCM_SourcePor|kRCM_SourceJtag) )
	{
		//PRINTF("Power on reset!\n");
	}*/

	return fromLowPower;
}
