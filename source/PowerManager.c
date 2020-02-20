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
#include "fsl_port.h"
#include "fsl_debug_console.h"

void PM_EnterLowPowerMode()
{
	// Setup wakeup pin on LLWU
	PORT_SetPinMux(BTN_SELECT_PORT, BTN_SELECT_PIN, kPORT_MuxAsGpio);

	LLWU_ClearExternalWakeupPinFlag(LLWU, PM_WAKEUP_PIN);
	LLWU_SetExternalWakeupPinMode(LLWU,  PM_WAKEUP_PIN, kLLWU_ExternalPinFallingEdge);

	// Setup digital filter 1 (NOT AVAILABLE IN VLLS0)
	llwu_external_pin_filter_mode_t filterSt;
	filterSt.pinIndex = PM_WAKEUP_PIN;
	filterSt.filterMode = kLLWU_PinFilterFallingEdge;

	LLWU_SetPinFilterMode(LLWU, 1, filterSt);
	for(int i = 0 ;i<840000 ; i++);	// Wait at least 5ms (840000>=7ms @120MHz) for filter to initialize


	smc_power_mode_vlls_config_t powerConfig;
	powerConfig.subMode = kSMC_StopSub3;

	SMC_PreEnterStopModes();
	SMC_SetPowerModeVlls(SMC, &powerConfig);
	SMC_PostExitStopModes();
}

bool PM_Recover()
{
	SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlls);

	bool fromLowPower = LLWU_GetExternalWakeupPinFlag(LLWU,PM_WAKEUP_PIN);
	if (fromLowPower) /* Wakeup from VLLS. */
	{
		LLWU_ClearExternalWakeupPinFlag(LLWU, PM_WAKEUP_PIN);
		PMC_ClearPeriphIOIsolationFlag(PMC);
	}

	return fromLowPower;
}
