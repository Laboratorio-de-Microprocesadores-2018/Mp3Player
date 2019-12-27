/**
 * @file ILI9341.h
 * @brief
 *
 *
 */

#ifndef ILI9341_H_
#define ILI9341_H_
#include <stdint.h>
#include <stdbool.h>

// Screen dimensions
#define ILI9341_TFTWIDTH    240
#define ILI9341_TFTHEIGHT   320

// Screen control codes
typedef enum
{
    ILI9341_NOP         = 0x00,    ///< No-op register
    ILI9341_SWRESET     = 0x01,    ///< Software reset register
    ILI9341_RDDID       = 0x04,    ///< Read display identification information
    ILI9341_RDDST       = 0x09,    ///< Read Display Status

    ILI9341_SLPIN       = 0x10,    ///< Enter Sleep Mode
    ILI9341_SLPOUT      = 0x11,    ///< Sleep Out
    ILI9341_PTLON       = 0x12,    ///< Partial Mode ON
    ILI9341_NORON       = 0x13,    ///< Normal Display Mode ON

    ILI9341_RDMODE      = 0x0A,    ///< Read Display Power Mode
    ILI9341_RDMADCTL    = 0x0B,    ///< Read Display MADCTL
    ILI9341_RDPIXFMT    = 0x0C,    ///< Read Display Pixel Format
    ILI9341_RDIMGFMT    = 0x0A,    ///< Read Display Image Format
    ILI9341_RDSELFDIAG  = 0x0F,    ///< Read Display Self-Diagnostic Result

    ILI9341_INVOFF      = 0x20,    ///< Display Inversion OFF
    ILI9341_INVON       = 0x21,    ///< Display Inversion ON
    ILI9341_GAMMASET    = 0x26,    ///< Gamma Set
    ILI9341_DISPOFF     = 0x28,    ///< Display OFF
    ILI9341_DISPON      = 0x29,    ///< Display ON

    ILI9341_CASET       = 0x2A,    ///< Column Address Set
    ILI9341_PASET       = 0x2B,    ///< Page Address Set
    ILI9341_RAMWR       = 0x2C,    ///< Memory Write
    ILI9341_RAMRD       = 0x2E,    ///< Memory Read

    ILI9341_PTLAR       = 0x30,    ///< Partial Area
    ILI9341_MADCTL      = 0x36,    ///< Memory Access Control
    ILI9341_IDLOFF      = 0x38,    ///< Vertical Scrolling Start Address
    ILI9341_PIXFMT      = 0x3A,    ///< COLMOD: Pixel Format Set

    ILI9341_FRMCTR1     = 0xB1,    ///< Frame Rate Control (In Normal Mode/Full Colors)
    ILI9341_FRMCTR2     = 0xB2,    ///< Frame Rate Control (In Idle Mode/8 colors)
    ILI9341_FRMCTR3     = 0xB3,    ///< Frame Rate control (In Partial Mode/Full Colors)
    ILI9341_INVCTR      = 0xB4,    ///< Display Inversion Control
    ILI9341_DFUNCTR     = 0xB6,    ///< Display Function Control

    ILI9341_PWCTR1      = 0xC0,    ///< Power Control 1
    ILI9341_PWCTR2      = 0xC1,    ///< Power Control 2
    ILI9341_PWCTR3      = 0xC2,    ///< Power Control 3
    ILI9341_PWCTR4      = 0xC3,    ///< Power Control 4
    ILI9341_PWCTR5      = 0xC4,    ///< Power Control 5
    ILI9341_VMCTR1      = 0xC5,    ///< VCOM Control 1
    ILI9341_VMCTR2      = 0xC7,    ///< VCOM Control 2

    ILI9341_RDID1       = 0xDA,    ///< Read ID 1
    ILI9341_RDID2       = 0xDB,    ///< Read ID 2
    ILI9341_RDID3       = 0xDC,    ///< Read ID 3
    ILI9341_RDID4       = 0xDD,    ///< Read ID 4

    ILI9341_GMCTRP1     = 0xE0,    ///< Positive Gamma Correction
    ILI9341_GMCTRN1     = 0xE1,    ///< Negative Gamma Correction

    ILI9341_PWCTR6      = 0xFC
}ILI9341_Command;


typedef enum {COMMAND,DATA}ILI9341_MsgType;

/**
 * 	Initialize TFT display
 */
void ILI9341_Init();


/**
 * Reset display
 */
void ILI9341_Reset();


/**
 * Send a byte to the display (data or command)
 */
void ILI9341_SendByte(ILI9341_MsgType type, uint8_t data);


/**
 * Send a command
 */
void ILI9341_SendCommand(uint16_t command);


/**
 * Send data using DMA (non-blocking)
 */
void ILI9341_SendData(uint8_t * data, uint32_t len);
void ILI9341_SendDataBlocking(uint8_t * data, uint32_t len);

/**
 * Send repeated data using DMA ¡¡¡¡¡NOT IMPLEMENTED!!!!
 */
void ILI9341_SendRepeatedData(uint8_t * data, uint8_t len, uint32_t n);


/**
 *  Return true if there is a transfer happening through DMA
 */
bool ILI9341_IsBusy();


#endif /* ILI9341_H_ */
