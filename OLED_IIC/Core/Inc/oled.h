/**
  ******************************************************************************
  * @file    oled.h
  * @brief   0.96 寸 OLED (SSD1306, 128x64) 软件 IIC 驱动
  *
  * 显示坐标系:
  *   x     : 0..127, 单位像素
  *   page  : 0..7,  单位页 (1 页 = 8 像素高)
  *           8x16 ASCII / 16x16 汉字都占用 page 和 page+1
  *
  * 所有绘图函数只修改内部显存, 调用 OLED_Refresh() 或
  * OLED_RefreshPages() 后才真正送到屏幕。
  ******************************************************************************
  */

#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "oledfont.h"

#define OLED_WIDTH          128U
#define OLED_HEIGHT         64U
#define OLED_PAGES          (OLED_HEIGHT / 8U)

/* 硬件水平滚动方向 */
#define OLED_SCROLL_LEFT    0U
#define OLED_SCROLL_RIGHT   1U

/* 初始化 GPIO、IIC 总线和 SSD1306, 初始化后清屏并点亮 */
void OLED_Init(void);

void OLED_Clear(void);
void OLED_ClearPages(uint8_t page_start, uint8_t page_end);
void OLED_Refresh(void);
void OLED_RefreshPages(uint8_t page_start, uint8_t page_end);

void OLED_DisplayOn(void);
void OLED_DisplayOff(void);
void OLED_SetContrast(uint8_t contrast);

/* 画点, color: 0 熄灭 / 非 0 点亮 */
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color);

/* 带裁剪的字符绘制, x 可以为负数 */
void OLED_ShowCharAt(int16_t x, uint8_t page, char ch);
void OLED_ShowCNAt(int16_t x, uint8_t page, uint8_t index);

/* 普通字符/汉字显示, page 取值 0..6 */
void OLED_ShowChar(uint8_t x, uint8_t page, char ch);
void OLED_ShowString(uint8_t x, uint8_t page, const char *str);
void OLED_ShowCN(uint8_t x, uint8_t page, uint8_t index);
void OLED_ShowCNString(uint8_t x, uint8_t page,
                       const uint8_t *indices, uint8_t count);

/* 显示无符号十进制数, len 为固定显示位数 (不足补 0) */
void OLED_ShowNum(uint8_t x, uint8_t page, uint32_t num, uint8_t len);

/* 中英文混合字符串的字形描述: type = 0 ASCII, 1 汉字, 0xFF 结束 */
typedef struct
{
    uint8_t type;
    uint8_t value;
} OLED_Glyph;

#define OLED_GLYPH_ASCII    0U
#define OLED_GLYPH_CN       1U
#define OLED_GLYPH_END      0xFFU

/* 字形序列的像素宽度 (ASCII 8 像素, 汉字 16 像素) */
int16_t OLED_GlyphsWidth(const OLED_Glyph *glyphs);

/* 软件横向滚动: 清空 page/page+1, 把字形序列画在 x = -offset 处 */
void OLED_ScrollGlyphs(const OLED_Glyph *glyphs, uint8_t page, int16_t offset);

/* SSD1306 硬件水平滚动 (0x26/0x27 + 0x2F), interval: 0..7, 越小越快 */
void OLED_HWScrollStart(uint8_t direction, uint8_t start_page,
                        uint8_t end_page, uint8_t interval);
void OLED_HWScrollStop(void);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H */
