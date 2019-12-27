/**
 * @file GILI9341.h
 * @brief
 *
 *
 */

#ifndef GILI9341_H_
#define GILI9341_H_
#include "../lvgl/src/lv_misc/lv_color.h"
#include "../lvgl/lvgl.h"
#include "../lv_conf.h"

#include "../lv_drv_conf.h"

void GILI9341_Init(void);
void GILI9341_Flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
//void GILI9341_Fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t  color);
//void GILI9341_Map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p);


#endif /* GILI9341_H_ */
