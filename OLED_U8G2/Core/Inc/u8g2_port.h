/**
  ******************************************************************************
  * @file    u8g2_port.h
  * @brief   U8g2 到 STM32 HAL 的移植接口
  ******************************************************************************
  */

#ifndef __U8G2_PORT_H
#define __U8G2_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "u8g2.h"

/* 全局 U8g2 实例，应用层直接使用 */
extern u8g2_t u8g2;

/* U8g2 回调，导出后可供虚拟显示缓冲实例复用 */
uint8_t U8G2_ByteCallback(u8x8_t *u8x8, uint8_t msg,
                          uint8_t arg_int, void *arg_ptr);
uint8_t U8G2_GPIOAndDelayCallback(u8x8_t *u8x8, uint8_t msg,
                                  uint8_t arg_int, void *arg_ptr);

/* 初始化 SSD1306 128x64 IIC OLED */
void U8G2_Port_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __U8G2_PORT_H */
