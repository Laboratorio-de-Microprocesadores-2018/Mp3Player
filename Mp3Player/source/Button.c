#include "Button.h"

// Modificado de:: https://github.com/0x1abin/MultiButton


#define EVENT_CALLBACK(ev)	if(handle->cb[ev]) handle->cb[ev]((Button*)handle)

// Button handle list head
static struct Button* head_handle = NULL;

/**
  * @brief  Button driver core function, driver state machine.
  * @param  handle: the button handle strcut.
  * @retval None
  */
static void Button_Handler(struct Button* handle);

void Button_Init(struct Button* handle, uint8_t(*pinLevel)(), uint8_t activeLevel)
{
	memset(handle, 0, sizeof(struct Button));
	handle->event = (uint8_t)NONE_PRESS;
	handle->halButtonLevel = pinLevel;
	handle->buttonLevel = handle->halButtonLevel();
	handle->activeLevel = activeLevel;
}


void Button_Attach(struct Button* handle, ButtonEvent event, ButtonCallback cb)
{
	handle->cb[event] = cb;
}


ButtonEvent Button_GetEvent(struct Button* handle)
{
	return (ButtonEvent)(handle->event);
}


int Button_Start(struct Button* handle)
{
	struct Button* target = head_handle;
	while(target) {
		if(target == handle) return -1;	//already exist.
		target = target->next;
	}
	handle->next = head_handle;
	head_handle = handle;
	return 0;
}


void Button_stop(struct Button* handle)
{
	struct Button** curr;
	for(curr = &head_handle; *curr; ) {
		struct Button* entry = *curr;
		if (entry == handle) {
			*curr = entry->next;
//			free(entry);
		} else
			curr = &entry->next;
	}
}


void Button_Tick()
{
	struct Button* target;
	// Iterate on list
	for(target=head_handle; target; target=target->next)
		Button_Handler(target);
}


void Button_Handler(struct Button* handle)
{
	uint8_t readLevel = handle->halButtonLevel();

	//ticks counter working..
	if((handle->state) > 0) handle->ticks++;

	/*------------button debounce handle---------------*/
	if(readLevel != handle->buttonLevel) { //not equal to prev one
		//continue read 3 times same new level change
		if(++(handle->debounceCount) >= DEBOUNCE_TICKS) {
			handle->buttonLevel = readLevel;
			handle->debounceCount = 0;
		}
	} else { //leved not change ,counter reset.
		handle->debounceCount = 0;
	}

	/*-----------------State machine-------------------*/
	switch (handle->state) {
	case 0:
		if(handle->buttonLevel == handle->activeLevel) {	//start press down
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CALLBACK(PRESS_DOWN);
			handle->ticks = 0;
			handle->repeat = 1;
			handle->state = 1;
		} else {
			handle->event = (uint8_t)NONE_PRESS;
		}
		break;

	case 1:
		if(handle->buttonLevel != handle->activeLevel) { //released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CALLBACK(PRESS_UP);
			handle->ticks = 0;
			handle->state = 2;

		} else if(handle->ticks > LONG_TICKS) {
			handle->event = (uint8_t)LONG_PRESS_START;
			EVENT_CALLBACK(LONG_PRESS_START);
			handle->state = 5;
		}
		break;

	case 2:
		if(handle->buttonLevel == handle->activeLevel) { //press down again
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CALLBACK(PRESS_DOWN);
			handle->repeat++;
			if(handle->repeat == 2) {
				EVENT_CALLBACK(DOUBLE_CLICK); // repeat hit
			}
			EVENT_CALLBACK(PRESS_REPEAT); // repeat hit
			handle->ticks = 0;
			handle->state = 3;
		} else if(handle->ticks > SHORT_TICKS) { //released timeout
			if(handle->repeat == 1) {
				handle->event = (uint8_t)SINGLE_CLICK;
				EVENT_CALLBACK(SINGLE_CLICK);
			} else if(handle->repeat == 2) {
				handle->event = (uint8_t)DOUBLE_CLICK;
			}
			handle->state = 0;
		}
		break;

	case 3:
		if(handle->buttonLevel != handle->activeLevel) { //released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CALLBACK(PRESS_UP);
			if(handle->ticks < SHORT_TICKS) {
				handle->ticks = 0;
				handle->state = 2; //repeat press
			} else {
				handle->state = 0;
			}
		}
		break;

	case 5:
		if(handle->buttonLevel == handle->activeLevel) {
			//continue hold trigger
			handle->event = (uint8_t)LONG_PRESS_HOLD;
			EVENT_CALLBACK(LONG_PRESS_HOLD);

		} else { //releasd
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CALLBACK(PRESS_UP);
			handle->state = 0; //reset
		}
		break;
	}
}
