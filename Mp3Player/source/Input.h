#ifndef INPUT_H_
#define INPUT_H_
#include "stdint.h"
#include "Button.h"

typedef void (*InputCallback)(void*);

typedef enum {PREV,NEXT,MENU,PLAY,SELECT,NumberOfButtons} ButtonID;
void Input_Init();

/**
 *  Read input
 */
uint8_t Input_ReadSelectButton();
uint8_t Input_ReadNextButton();
uint8_t Input_ReadPrevButton();
uint8_t Input_ReadMenuButton();
uint8_t Input_ReadPlayButton();
uint8_t Input_ReadEncoderCount();
uint8_t Input_ReadEncoderDirection();

/**
 *  Attach Callbacks
 */
void Input_Attach(ButtonID button, ButtonEvent ev, ButtonCallback);
void Input_AttachEncoderInc(InputCallback c);
void Input_AttachEncoderDec(InputCallback c);

void Input_GetEvent(ButtonID * button, ButtonEvent * ev);

#endif /* INPUT_H_ */
