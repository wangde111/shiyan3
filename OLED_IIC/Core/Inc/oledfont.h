/**
  ******************************************************************************
  * @file    oledfont.h
  * @brief   SSD1306 点阵字库: 16x16 Chinese and 8x16 ASCII.
  ******************************************************************************
  */

#ifndef __OLEDFONT_H
#define __OLEDFONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 16x16 Chinese characters, order:
   学 号 姓 名 王 兴 温 度 湿 欢 迎 来 到 重 庆 交 通 大 物 联 网 实 训 室 ，！ */
#define OLED_CN_XUE        0U
#define OLED_CN_HAO        1U
#define OLED_CN_XING_LABEL 2U
#define OLED_CN_MING       3U
#define OLED_CN_WANG       4U
#define OLED_CN_XING       5U
#define OLED_CN_WEN        6U
#define OLED_CN_DU         7U
#define OLED_CN_SHI        8U
#define OLED_CN_HUAN       9U
#define OLED_CN_YING       10U
#define OLED_CN_LAI        11U
#define OLED_CN_DAO        12U
#define OLED_CN_CHONG      13U
#define OLED_CN_QING       14U
#define OLED_CN_JIAO       15U
#define OLED_CN_TONG       16U
#define OLED_CN_DA         17U
#define OLED_CN_WU         18U
#define OLED_CN_LIAN       19U
#define OLED_CN_NET        20U
#define OLED_CN_SHI2       21U
#define OLED_CN_XUN        22U
#define OLED_CN_ROOM       23U
#define OLED_CN_COMMA      24U
#define OLED_CN_EXCLAM     25U
#define OLED_CN_COUNT      26U

/* 8x16 ASCII 字库覆盖的字符范围 */
#define ASCII_FIRST        0x20U
#define ASCII_LAST         0x7EU

extern const uint8_t OLED_CN16x16[OLED_CN_COUNT][32];
extern const uint8_t OLED_ASCII8x16[(ASCII_LAST - ASCII_FIRST + 1) * 16];

#ifdef __cplusplus
}
#endif

#endif /* __OLEDFONT_H */
