#ifndef LED_MATRIX_H_
#define LED_MATRIX_H_


/////////////////////////////////////////////////////////////////////////////////
//                             Included header files                           //
/////////////////////////////////////////////////////////////////////////////////

#include "stdint.h"
#include "stdbool.h"

/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8

/////////////////////////////////////////////////////////////////////////////////
//                    Enumerations, structures and typedefs                    //
/////////////////////////////////////////////////////////////////////////////////


#define RED_MASK	0xFF000000
#define RED_SHIFT	24U
#define GREEN_MASK	0x00FF0000
#define GREEN_SHIFT 16U
#define BLUE_MASK	0x0000FF00
#define BLUE_SHIFT	8U
#define ALPHA_MASK	0x000000FF
#define ALPHA_SHIFT	0U
/*
typedef uint32_t Color;
inline uint8_t getRed(Color c) 	{ return (uint8_t)((c>>RED_SHIFT) & 0xFF); };
inline uint8_t getGreen(Color c){ return (uint8_t)((c>>GREEN_SHIFT) & 0xFF); };
inline uint8_t getBlue(Color c) { return (uint8_t)((c>>BLUE_SHIFT) & 0xFF); };
inline Color RGB(uint8_t r,uint8_t g,uint8_t b)
{
	return 	(Color)(((r<<RED_SHIFT)&RED_MASK)|
			((g<<GREEN_SHIFT)&GREEN_MASK)	 |
			((b<<BLUE_SHIFT)&BLUE_MASK));
}
*/

typedef union
{
	uint32_t val;
	uint8_t RGB[4];
	struct {
		uint8_t R,G,B,A;
	};
}Color;



typedef enum{SCROLL_ONCE,SCROLL_CONTINUOUS}ScrollMode;

/////////////////////////////////////////////////////////////////////////////////
//                       		Global variables	                       	   //
/////////////////////////////////////////////////////////////////////////////////
/*
static const RED={10,0,0};
static const GREEN={};
static const BLUE={};
static const WHITE={};
static const BLACK={0,0,0};
static const RED={};
*/
/////////////////////////////////////////////////////////////////////////////////
//                           Global Function Declarations                      //
/////////////////////////////////////////////////////////////////////////////////

/**
 *
 */
void LedMatrix_Init();

/**
 *
 */
void LedMatrix_Clear();

/**
 *
 */
void LedMatrix_PrintLed(uint8_t row, uint8_t col, Color c);

/**
 *
 */
void LedMatrix_PrintScreen(Color * screen);

/**
 *
 */
void LedMatrix_PlainColor(Color c);

/**
 *
 */
void LedMatrix_ColorTest();
void LedMatrix_ShortColorTest();
/**
 *
 */
void LedMatrix_Print(char * c, uint8_t len,Color fontColor, Color backgroundColor, ScrollMode mode);

/**
 *
 */
void LedMatrix_StopScrolling();

/**
 *
 */
void LedMatrix_StartScrolling();

#endif /* LED_MATRIX_H_ */
