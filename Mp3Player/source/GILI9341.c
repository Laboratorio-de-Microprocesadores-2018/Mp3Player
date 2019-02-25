#include "GILI9341.h"
#include "ILI9341.h"

static void ILI9341_SetWindow(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

void GILI9341_Init(void)
{
	ILI9341_Init();
}

void GILI9341_Flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_map)
{
	/*Return if the area is out the screen*/
	/*if(x2 < 0) return;
	if(y2 < 0) return;
	if(x1 > ILI9341_TFTWIDTH - 1) return;
	if(y1 > ILI9341_TFTHEIGHT - 1) return;*/


	ILI9341_SetWindow(x1,y1,x2,y2);

	/* Memory write */
	ILI9341_SendCommand(ILI9341_RAMWR);

	uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);

	ILI9341_SendData((void*)color_map, size * 2); // Each color is 16bit

	//lv_flush_ready();
}

void GILI9341_Fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
{
	static lv_color_t staticColor;

	ILI9341_SetWindow(x1,y1,x2,y2);

	/* Memory write */
	ILI9341_SendCommand(ILI9341_RAMWR);

	staticColor = color;
	uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);
	ILI9341_SendRepeatedData((uint8_t*)&staticColor,sizeof(lv_color_t),size);
}

void GILI9341_Map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
	/*Return if the area is out the screen*/
	/*if(x2 < 0) return;
	if(y2 < 0) return;
	if(x1 > ILI9341_TFTWIDTH - 1) return;
	if(y1 > ILI9341_TFTHEIGHT - 1) return;*/


	ILI9341_SetWindow(x1,y1,x2,y2);

	/* Memory write */
	ILI9341_SendCommand(ILI9341_RAMWR);

	uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);

	ILI9341_SendData((uint8_t*)color_p, size * 2); // Each color is 16bit

	//lv_flush_ready();
}


void ILI9341_SetWindow(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{

	/*Column address set*/
	ILI9341_SendCommand(ILI9341_CASET);
	ILI9341_SendByte(DATA,(x1 >> 8) & 0xFF);
	ILI9341_SendByte(DATA,(x1 >> 0) & 0xFF);
	ILI9341_SendByte(DATA,(y1 >> 8) & 0xFF);
	ILI9341_SendByte(DATA,(y1 >> 0) & 0xFF);

	/*Page address set*/
	ILI9341_SendCommand(ILI9341_PASET);
	ILI9341_SendByte(DATA,(x2 >> 8) & 0xFF);
	ILI9341_SendByte(DATA,(x2 >> 0) & 0xFF);
	ILI9341_SendByte(DATA,(y2 >> 8) & 0xFF);
	ILI9341_SendByte(DATA,(y2 >> 0) & 0xFF);
}
