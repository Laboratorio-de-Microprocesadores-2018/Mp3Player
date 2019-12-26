/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

#define PORT_DFER_DFE_0_MASK 0x01u /*!<@brief Digital Filter Enable Mask for item 0. */

/*! @name PORTA5 (number 39), /I2S_CLK
  @{ */
#define BOARD_INITPINS_I2S_CLK_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_INITPINS_I2S_CLK_PIN 5U     /*!<@brief PORTA pin index: 5 */
                                          /* @} */

/*! @name PORTA12 (number 42), /I2S_SDI
  @{ */
#define BOARD_INITPINS_I2S_SDI_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_INITPINS_I2S_SDI_PIN 12U    /*!<@brief PORTA pin index: 12 */
                                          /* @} */

/*! @name PORTA13 (number 43), /I2S_WS
  @{ */
#define BOARD_INITPINS_I2S_WS_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_INITPINS_I2S_WS_PIN 13U    /*!<@brief PORTA pin index: 13 */
                                         /* @} */

/*! @name PORTA17 (number 47), /I2S_MCLK
  @{ */
#define BOARD_INITPINS_I2S_MCLK_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_INITPINS_I2S_MCLK_PIN 17U    /*!<@brief PORTA pin index: 17 */
                                           /* @} */

/*! @name PORTA14 (number 44), /HPR_SENSE
  @{ */
#define BOARD_INITPINS_HPR_SENSE_GPIO GPIOA /*!<@brief GPIO device name: GPIOA */
#define BOARD_INITPINS_HPR_SENSE_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_INITPINS_HPR_SENSE_PIN 14U    /*!<@brief PORTA pin index: 14 */
                                            /* @} */

/*! @name PORTA15 (number 45), /HPL_SENSE
  @{ */
#define BOARD_INITPINS_HPL_SENSE_GPIO GPIOA /*!<@brief GPIO device name: GPIOA */
#define BOARD_INITPINS_HPL_SENSE_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_INITPINS_HPL_SENSE_PIN 15U    /*!<@brief PORTA pin index: 15 */
                                            /* @} */

/*! @name PORTB0 (number 53), /ENC_A
  @{ */
#define BOARD_INITPINS_ENC_A_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_ENC_A_PIN 0U     /*!<@brief PORTB pin index: 0 */
                                        /* @} */

/*! @name PORTB1 (number 54), /ENC_B
  @{ */
#define BOARD_INITPINS_ENC_B_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_ENC_B_PIN 1U     /*!<@brief PORTB pin index: 1 */
                                        /* @} */

/*! @name PORTB2 (number 55), /AUD_I2C_SCL
  @{ */
#define BOARD_INITPINS_AUD_I2C_SCL_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_AUD_I2C_SCL_PIN 2U     /*!<@brief PORTB pin index: 2 */
                                              /* @} */

/*! @name PORTB3 (number 56), /AUD_I2C_SDA
  @{ */
#define BOARD_INITPINS_AUD_I2C_SDA_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_AUD_I2C_SDA_PIN 3U     /*!<@brief PORTB pin index: 3 */
                                              /* @} */

/*! @name PORTB10 (number 58), /LCD_CSn
  @{ */
#define BOARD_INITPINS_LCD_CSn_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_LCD_CSn_PIN 10U    /*!<@brief PORTB pin index: 10 */
                                          /* @} */

/*! @name PORTB11 (number 59), /LCD_SCK
  @{ */
#define BOARD_INITPINS_LCD_SCK_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_LCD_SCK_PIN 11U    /*!<@brief PORTB pin index: 11 */
                                          /* @} */

/*! @name PORTB17 (number 63), /LCD_MISO
  @{ */
#define BOARD_INITPINS_LCD_MISO_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_LCD_MISO_PIN 17U    /*!<@brief PORTB pin index: 17 */
                                           /* @} */

/*! @name PORTB16 (number 62), /LCD_MOSI
  @{ */
#define BOARD_INITPINS_LCD_MOSI_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_LCD_MOSI_PIN 16U    /*!<@brief PORTB pin index: 16 */
                                           /* @} */

/*! @name PORTB18 (number 64), /LCD_RESET
  @{ */
#define BOARD_INITPINS_LCD_RESET_GPIO GPIOB /*!<@brief GPIO device name: GPIOB */
#define BOARD_INITPINS_LCD_RESET_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_LCD_RESET_PIN 18U    /*!<@brief PORTB pin index: 18 */
                                            /* @} */

/*! @name PORTB19 (number 65), /LCD_DCRS
  @{ */
#define BOARD_INITPINS_LCD_DCRS_GPIO GPIOB /*!<@brief GPIO device name: GPIOB */
#define BOARD_INITPINS_LCD_DCRS_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_LCD_DCRS_PIN 19U    /*!<@brief PORTB pin index: 19 */
                                           /* @} */

/*! @name PORTB20 (number 66), /LCD_LED
  @{ */
#define BOARD_INITPINS_LCD_LED_GPIO GPIOB /*!<@brief GPIO device name: GPIOB */
#define BOARD_INITPINS_LCD_LED_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_LCD_LED_PIN 20U    /*!<@brief PORTB pin index: 20 */
                                          /* @} */

/*! @name PORTB22 (number 68), /LED_DATA
  @{ */
#define BOARD_INITPINS_LED_DATA_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_LED_DATA_PIN 22U    /*!<@brief PORTB pin index: 22 */
                                           /* @} */

/*! @name PORTC2 (number 72), /BTN_PLAY
  @{ */
#define BOARD_INITPINS_BTN_PLAY_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_INITPINS_BTN_PLAY_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_BTN_PLAY_PIN 2U     /*!<@brief PORTC pin index: 2 */
                                           /* @} */

/*! @name PORTC3 (number 73), /BTN_PREV
  @{ */
#define BOARD_INITPINS_BTN_PREV_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_INITPINS_BTN_PREV_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_BTN_PREV_PIN 3U     /*!<@brief PORTC pin index: 3 */
                                           /* @} */

/*! @name PORTC4 (number 76), /PWR_EN_5Vn
  @{ */
#define BOARD_INITPINS_PWR_EN_5Vn_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_INITPINS_PWR_EN_5Vn_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_PWR_EN_5Vn_PIN 4U     /*!<@brief PORTC pin index: 4 */
                                             /* @} */

/*! @name PORTC5 (number 77), /LED_SENSE_3_3V
  @{ */
#define BOARD_INITPINS_LED_SENSE_3_3V_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_INITPINS_LED_SENSE_3_3V_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_LED_SENSE_3_3V_PIN 5U     /*!<@brief PORTC pin index: 5 */
                                                 /* @} */

/*! @name PORTB23 (number 69), /BTN_MENU
  @{ */
#define BOARD_INITPINS_BTN_MENU_GPIO GPIOB /*!<@brief GPIO device name: GPIOB */
#define BOARD_INITPINS_BTN_MENU_PORT PORTB /*!<@brief PORT device name: PORTB */
#define BOARD_INITPINS_BTN_MENU_PIN 23U    /*!<@brief PORTB pin index: 23 */
                                           /* @} */

/*! @name PORTC10 (number 82), /BAT_I2C_SCL
  @{ */
#define BOARD_INITPINS_BAT_I2C_SCL_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_BAT_I2C_SCL_PIN 10U    /*!<@brief PORTC pin index: 10 */
                                              /* @} */

/*! @name PORTC11 (number 83), /BAT_I2C_SCL
  @{ */
#define BOARD_INITPINS_BAT_I2C_SDA_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_BAT_I2C_SDA_PIN 11U    /*!<@brief PORTC pin index: 11 */
                                              /* @} */

/*! @name PORTC13 (number 85), /LOW_BATn
  @{ */
#define BOARD_INITPINS_LOW_BATn_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_INITPINS_LOW_BATn_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_LOW_BATn_PIN 13U    /*!<@brief PORTC pin index: 13 */
                                           /* @} */

/*! @name PORTC14 (number 86), /PWR_EN_3V3
  @{ */
#define BOARD_INITPINS_PWR_EN_3V3_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_INITPINS_PWR_EN_3V3_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_PWR_EN_3V3_PIN 14U    /*!<@brief PORTC pin index: 14 */
                                             /* @} */

/*! @name PORTC16 (number 90), /DONEn
  @{ */
#define BOARD_INITPINS_DONEn_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_INITPINS_DONEn_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_DONEn_PIN 16U    /*!<@brief PORTC pin index: 16 */
                                        /* @} */

/*! @name PORTC17 (number 91), /DOK
  @{ */
#define BOARD_INITPINS_DOK_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_INITPINS_DOK_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_DOK_PIN 17U    /*!<@brief PORTC pin index: 17 */
                                      /* @} */

/*! @name PORTC18 (number 92), /CHG
  @{ */
#define BOARD_INITPINS_CHG_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_INITPINS_CHG_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_CHG_PIN 18U    /*!<@brief PORTC pin index: 18 */
                                      /* @} */

/*! @name PORTD0 (number 93), /DBG1
  @{ */
#define BOARD_INITPINS_DBG1_GPIO GPIOD /*!<@brief GPIO device name: GPIOD */
#define BOARD_INITPINS_DBG1_PORT PORTD /*!<@brief PORT device name: PORTD */
#define BOARD_INITPINS_DBG1_PIN 0U     /*!<@brief PORTD pin index: 0 */
                                       /* @} */

/*! @name PORTD1 (number 94), /DBG2
  @{ */
#define BOARD_INITPINS_DBG2_GPIO GPIOD /*!<@brief GPIO device name: GPIOD */
#define BOARD_INITPINS_DBG2_PORT PORTD /*!<@brief PORT device name: PORTD */
#define BOARD_INITPINS_DBG2_PIN 1U     /*!<@brief PORTD pin index: 1 */
                                       /* @} */

/*! @name PORTD2 (number 95), /DBG3
  @{ */
#define BOARD_INITPINS_DBG3_GPIO GPIOD /*!<@brief GPIO device name: GPIOD */
#define BOARD_INITPINS_DBG3_PORT PORTD /*!<@brief PORT device name: PORTD */
#define BOARD_INITPINS_DBG3_PIN 2U     /*!<@brief PORTD pin index: 2 */
                                       /* @} */

/*! @name PORTD3 (number 96), /DBG4
  @{ */
#define BOARD_INITPINS_DBG4_GPIO GPIOD /*!<@brief GPIO device name: GPIOD */
#define BOARD_INITPINS_DBG4_PORT PORTD /*!<@brief PORT device name: PORTD */
#define BOARD_INITPINS_DBG4_PIN 3U     /*!<@brief PORTD pin index: 3 */
                                       /* @} */

/*! @name PORTE2 (number 3), /SD_CLK
  @{ */
#define BOARD_INITPINS_SD_CLK_PORT PORTE /*!<@brief PORT device name: PORTE */
#define BOARD_INITPINS_SD_CLK_PIN 2U     /*!<@brief PORTE pin index: 2 */
                                         /* @} */

/*! @name PORTE4 (number 5), /SD_DAT3
  @{ */
#define BOARD_INITPINS_SD_DAT3_PORT PORTE /*!<@brief PORT device name: PORTE */
#define BOARD_INITPINS_SD_DAT3_PIN 4U     /*!<@brief PORTE pin index: 4 */
                                          /* @} */

/*! @name PORTE5 (number 6), /SD_DAT2
  @{ */
#define BOARD_INITPINS_SD_DAT2_PORT PORTE /*!<@brief PORT device name: PORTE */
#define BOARD_INITPINS_SD_DAT2_PIN 5U     /*!<@brief PORTE pin index: 5 */
                                          /* @} */

/*! @name PORTE1 (number 2), /SD_DAT0
  @{ */
#define BOARD_INITPINS_SD_DAT0_PORT PORTE /*!<@brief PORT device name: PORTE */
#define BOARD_INITPINS_SD_DAT0_PIN 1U     /*!<@brief PORTE pin index: 1 */
                                          /* @} */

/*! @name PORTE3 (number 4), /SD_CMD
  @{ */
#define BOARD_INITPINS_SD_CMD_PORT PORTE /*!<@brief PORT device name: PORTE */
#define BOARD_INITPINS_SD_CMD_PIN 3U     /*!<@brief PORTE pin index: 3 */
                                         /* @} */

/*! @name PORTE0 (number 1), /SD_DAT1
  @{ */
#define BOARD_INITPINS_SD_DAT1_PORT PORTE /*!<@brief PORT device name: PORTE */
#define BOARD_INITPINS_SD_DAT1_PIN 0U     /*!<@brief PORTE pin index: 0 */
                                          /* @} */

/*! @name PORTC1 (number 71), /BTN_SELECT
  @{ */
#define BOARD_INITPINS_BTN_SELECT_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_BTN_SELECT_PIN 1U     /*!<@brief PORTC pin index: 1 */
                                             /* @} */

/*! @name PORTA18 (number 50), /XTALN
  @{ */
#define BOARD_INITPINS_XTALP_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_INITPINS_XTALP_PIN 18U    /*!<@brief PORTA pin index: 18 */
                                        /* @} */

/*! @name PORTA19 (number 51), /XTALN
  @{ */
#define BOARD_INITPINS_XTALN_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_INITPINS_XTALN_PIN 19U    /*!<@brief PORTA pin index: 19 */
                                        /* @} */

/*! @name PORTA4 (number 38), /NMI
  @{ */
#define BOARD_INITPINS_NMI_PORT PORTA /*!<@brief PORT device name: PORTA */
#define BOARD_INITPINS_NMI_PIN 4U     /*!<@brief PORTA pin index: 4 */
                                      /* @} */

/*! @name PORTD7 (number 100), /SD_DETECT
  @{ */
#define BOARD_INITPINS_SD_DETECT_GPIO GPIOD /*!<@brief GPIO device name: GPIOD */
#define BOARD_INITPINS_SD_DETECT_PORT PORTD /*!<@brief PORT device name: PORTD */
#define BOARD_INITPINS_SD_DETECT_PIN 7U     /*!<@brief PORTD pin index: 7 */
                                            /* @} */

/*! @name DAC0_OUT (number 27), /DAC_AUDIO
  @{ */
/* @} */

/*! @name USB0_DM (number 11), /USB_N
  @{ */
/* @} */

/*! @name USB0_DP (number 10), /USB_P
  @{ */
/* @} */

/*! @name VOUT33 (number 12), /VOUT3V3
  @{ */
/* @} */

/*! @name VREGIN (number 13), /P3V3_MCU
  @{ */
/* @} */

/*! @name PORTC0 (number 70), /BTN_NEXT
  @{ */
#define BOARD_INITPINS_BTN_NEXT_GPIO GPIOC /*!<@brief GPIO device name: GPIOC */
#define BOARD_INITPINS_BTN_NEXT_PORT PORTC /*!<@brief PORT device name: PORTC */
#define BOARD_INITPINS_BTN_NEXT_PIN 0U     /*!<@brief PORTC pin index: 0 */
                                           /* @} */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
