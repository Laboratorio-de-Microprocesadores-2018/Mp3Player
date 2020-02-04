/**
 * @file GILI9341.c
 * @brief
 *
 *
 */

#include "GILI9341.h"
#include "ILI9341.h"
#include "fsl_common.h"

static void GILI9341_SetWindow(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

void GILI9341_Init(void)
{
	ILI9341_Init();
}

void GILI9341_Deinit(void)
{
	ILI9341_Deinit();
}
//void GILI9341_Flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_map)
void GILI9341_Flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
	assert(ILI9341_IsBusy()==false);

	/*Return if the area is out the screen*/
	/*if(x2 < 0) return;
	if(y2 < 0) return;
	if(x1 > ILI9341_TFTWIDTH - 1) return;
	if(y1 > ILI9341_TFTHEIGHT - 1) return;*/

	GILI9341_SetWindow(area->x1,area->y1,area->x2,area->y2);

	/* Memory write */
	ILI9341_SendCommand(ILI9341_RAMWR);

	uint32_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);

	ILI9341_SendData((void*)color_p, sizeof(lv_color_t)*size); // Each color is 16bit

	//lv_disp_flush_ready(disp_drv);

}

//void GILI9341_Fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color)
//{
//	assert(ILI9341_IsBusy()==false);
//
//	GILI9341_SetWindow(x1,y1,x2,y2);
//
//	/* Memory write */
//	ILI9341_SendCommand(ILI9341_RAMWR);
//
//	uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);
//
//	//assert(size<LV_VDB_SIZE*2);
//
//	//static uint16_t colors[LV_VDB_SIZE*2];
//	static uint16_t colors[2*2];
//	for(int i=0; i<size;i++)
//		colors[i]=color.full;
//
//	ILI9341_SendData((uint8_t*)colors,sizeof(lv_color_t)*size);
//
//
//	/* ESTO NO ANDA (todavia) Es para evitar hacer un arreglo y llenarlo del mismo color
//	static lv_color_t c;
//	c = color;
//	ILI9341_SendRepeatedData((uint8_t)&color,sizeof(lv_color_t),size);
//	return;
//	*/
//}
//
//void GILI9341_Map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
//{
//	assert(ILI9341_IsBusy()==false);
//
//
//	/*Return if the area is out the screen*/
//	/*if(x2 < 0) return;
//	if(y2 < 0) return;
//	if(x1 > ILI9341_TFTWIDTH - 1) return;
//	if(y1 > ILI9341_TFTHEIGHT - 1) return;*/
//
//
//	GILI9341_SetWindow(x1,y1,x2,y2);
//
//	/* Memory write */
//	ILI9341_SendCommand(ILI9341_RAMWR);
//
//	uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);
//
//
//	ILI9341_SendData((uint8_t*)color_p, size * sizeof(lv_color_t));
//
//}


void GILI9341_SetWindow(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{

	/*Column address set*/
	ILI9341_SendCommand(ILI9341_CASET);
	ILI9341_SendByte(DATA,(x1 >> 8) & 0xFF);
	ILI9341_SendByte(DATA,(x1 >> 0) & 0xFF);
	ILI9341_SendByte(DATA,(x2 >> 8) & 0xFF);
	ILI9341_SendByte(DATA,(x2 >> 0) & 0xFF);

	/*Page address set*/
	ILI9341_SendCommand(ILI9341_PASET);
	ILI9341_SendByte(DATA,(y1 >> 8) & 0xFF);
	ILI9341_SendByte(DATA,(y1 >> 0) & 0xFF);
	ILI9341_SendByte(DATA,(y2 >> 8) & 0xFF);
	ILI9341_SendByte(DATA,(y2 >> 0) & 0xFF);
}
