/*
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2018 NXP
* All rights reserved.
*
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "pin_mux.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* The board name */
#define BOARD_NAME "Mp3 Player"



/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE 			kSerialPort_Uart
#define BOARD_DEBUG_UART_BASEADDR 		(uint32_t) UART0
#define BOARD_DEBUG_UART_INSTANCE 		0U
#define BOARD_DEBUG_UART_CLKSRC 		SYS_CLK
#define BOARD_DEBUG_UART_CLK_FREQ 		CLOCK_GetCoreSysClkFreq()
#define BOARD_UART_IRQ 					UART0_RX_TX_IRQn
#define BOARD_UART_IRQ_HANDLER 			UART0_RX_TX_IRQHandler

#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE 		115200
#endif /* BOARD_DEBUG_UART_BAUDRATE */

/* SDHC base address, clock and card detection pin */
#define BOARD_SDHC_BASEADDR 			SDHC
#define BOARD_SDHC_CLKSRC 				kCLOCK_Osc0ErClk
#define BOARD_SDHC_CLK_FREQ 			CLOCK_GetFreq(BOARD_SDHC_CLKSRC)
#define BOARD_SDHC_IRQ 					SDHC_IRQn
#define BOARD_SDHC_CD_GPIO_BASE 		SD_DETECT_GPIO

#ifndef BOARD_SDHC_CD_GPIO_PIN
#define BOARD_SDHC_CD_GPIO_PIN 			SD_DETECT_PIN
#endif

#define BOARD_SDHC_CD_PORT_BASE 		SD_DETECT_PORT
#define BOARD_SDHC_CD_PORT_IRQ 			PORTD_IRQn
#define BOARD_SDHC_CD_PORT_IRQ_HANDLER 	PORTD_IRQHandler
#define BOARD_SDHC_CARD_INSERT_CD_LEVEL (0U)

/* DAC base address */
#define BOARD_DAC_BASEADDR 				DAC0

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/
void BOARD_InitDebugConsole(void);
#if defined(SDK_I2C_BASED_COMPONENT_USED) && SDK_I2C_BASED_COMPONENT_USED
void BOARD_I2C_Init(I2C_Type *base, uint32_t clkSrc_Hz);
status_t BOARD_I2C_Send(I2C_Type *base,
                        uint8_t deviceAddress,
                        uint32_t subAddress,
                        uint8_t subaddressSize,
                        uint8_t *txBuff,
                        uint8_t txBuffSize);
status_t BOARD_I2C_Receive(I2C_Type *base,
                           uint8_t deviceAddress,
                           uint32_t subAddress,
                           uint8_t subaddressSize,
                           uint8_t *rxBuff,
                           uint8_t rxBuffSize);
void BOARD_Accel_I2C_Init(void);
status_t BOARD_Accel_I2C_Send(uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint32_t txBuff);
status_t BOARD_Accel_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);
#endif /* SDK_I2C_BASED_COMPONENT_USED */
#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
