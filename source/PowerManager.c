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

#define PM_WAKEUP_PIN 	6 //BTN_SELECT_PIN	// SW2: PC5


void PM_EnterLowPowerMode()
{

	// Setup wakeup pin on LLWU
    PORT_SetPinMux(BTN_SELECT_PORT, BTN_SELECT_PIN, kPORT_MuxAsGpio);
	PORT_SetPinInterruptConfig(BTN_SELECT_PORT,BTN_SELECT_PIN, kPORT_InterruptFallingEdge);

	LLWU_ClearExternalWakeupPinFlag(LLWU, PM_WAKEUP_PIN);
	LLWU_SetExternalWakeupPinMode(LLWU,  PM_WAKEUP_PIN, kLLWU_ExternalPinFallingEdge);
	EnableIRQ(LLWU_IRQn);

	smc_power_mode_vlls_config_t powerConfig;
	powerConfig.subMode = kSMC_StopSub3;

	SMC_PreEnterStopModes();
	SMC_SetPowerModeVlls(SMC, &powerConfig);
	SMC_PostExitStopModes();
}

bool PM_Recover()
{
	SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeAll);

	uint32_t resetCause = RCM_GetPreviousResetSources(RCM);
	if (resetCause & kRCM_SourceWakeup) /* Wakeup from VLLS. */
	{
		PMC_ClearPeriphIOIsolationFlag(PMC);
		NVIC_ClearPendingIRQ(LLWU_IRQn);

		if(LLWU_GetExternalWakeupPinFlag(LLWU,6)==true)
		{
			//PRINTF("Wake up with button!\n");
			LLWU_ClearExternalWakeupPinFlag(LLWU, PM_WAKEUP_PIN);
		}
		/** RTC Alarm! */
		else if(LLWU_GetInternalWakeupModuleFlag(LLWU, 5) == true)
		{
			//PRINTF("Wake up with RTC alarm!\n");
		}
		/** RTC Seconds */
//		else if(LLWU_GetInternalWakeupModuleFlag(LLWU, 7) == true)
//		{
//
//		}
	}
	else if ( resetCause & (kRCM_SourcePor|kRCM_SourceJtag) )
	{
		//PRINTF("Power on reset!\n");
	}


}
