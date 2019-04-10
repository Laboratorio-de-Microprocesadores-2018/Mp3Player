/**
 * @file Button.h
 * @brief
 *
 *
 */

#ifndef Button_H_
#define Button_H_

#include "stdint.h"
#include "string.h"


#define BUTTON_TICK_INTERVAL_MS  10 //ms
#define DEBOUNCE_TICKS    3	//MAX 8
#define SHORT_TICKS       (300 /BUTTON_TICK_INTERVAL_MS)
#define LONG_TICKS        (1000 /BUTTON_TICK_INTERVAL_MS)


typedef void (*ButtonCallback)(void*);

typedef enum {
	PRESS_DOWN = 0,
	PRESS_UP,
	PRESS_REPEAT,
	SINGLE_CLICK,
	DOUBLE_CLICK,
	LONG_PRESS_START,
	LONG_PRESS_HOLD,
	numberOfEvents,
	NONE_PRESS
}ButtonEvent;

typedef struct Button {
	uint16_t ticks;
	uint8_t ID;
	uint8_t  repeat : 4;
	uint8_t  event : 4;
	uint8_t  state : 3;
	uint8_t  debounceCount : 3;
	uint8_t  activeLevel : 1;
	uint8_t  buttonLevel : 1;
	uint8_t  (*halButtonLevel)(void);
	ButtonCallback  cb[numberOfEvents];
	struct Button* next;
}Button;


/**
  * @brief  Initializes the button struct handle.
  * @param  handle: the button handle strcut.
  * @param  pinLevel: read the HAL GPIO of the connet button level.
  * @param  active_level: pressed GPIO level.
  * @retval None
  */
void Button_Init(struct Button* handle, uint8_t(*pinLevel)(), uint8_t activeLevel, uint8_t ID);

/**
  * @brief  Attach the button event callback function.
  * @param  handle: the button handle strcut.
  * @param  event: trigger event type.
  * @param  cb: callback function.
  * @retval None
  */
void Button_Attach(struct Button* handle, ButtonEvent event, ButtonCallback cb);

/**
  * @brief  Inquire the button event happen.
  * @param  handle: the button handle struct.
  * @retval button event.
  */

ButtonEvent Button_GetEvent(struct Button* handle);

/**
  * @brief  Start the button work, add the handle into work list.
  * @param  handle: target handle strcut.
  * @retval 0: succeed. -1: already exist.
  */
int  Button_Start(struct Button* handle);

/**
  * @brief  Stop the button work, remove the handle off work list.
  * @param  handle: target handle strcut.
  * @retval None
  */
void Button_Stop(struct Button* handle);

/**
  * @brief  Invoke this function periodically.
  * @param  None.
  * @retval None
  */
void Button_Tick(void);


#endif /* Button_H_ */
