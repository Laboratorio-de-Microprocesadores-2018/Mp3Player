/**
 * @file Input.h
 * @brief
 *
 *
 */

#ifndef INPUT_H_
#define INPUT_H_
#include "stdint.h"
#include "Button.h"

typedef void (*InputCallback)(void*);

typedef enum {
	PREV,
	NEXT,
	MENU,
	PLAY,
	SELECT,
	NumberOfButtons
} ButtonID;



/**
 *  Initialize input module
 */
void Input_Init(void);
void Input_Deinit(void);

/**
 *  Read input
 */
uint8_t Input_ReadSelectButton(void);
uint8_t Input_ReadNextButton(void);
uint8_t Input_ReadPrevButton(void);
uint8_t Input_ReadMenuButton(void);
uint8_t Input_ReadPlayButton(void);

int8_t Input_ReadEncoderCount(void);
uint8_t Input_ReadEncoderDirection(void);
uint8_t Input_ReadEncoderButton(void);


/**
 *  Attach Callbacks
 */
void Input_Start(void);
void Input_Attach(ButtonID button, ButtonEvent ev, ButtonCallback);
void Input_AttachEncoderInc(InputCallback c);
void Input_AttachEncoderDec(InputCallback c);

void Input_GetEvent(ButtonID * button, ButtonEvent * ev);

void Input_Tick(void);

#endif /* INPUT_H_ */
