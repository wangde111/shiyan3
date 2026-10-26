/**
  ******************************************************************************
  * @file    main.h
  * @brief   实验三：0.96 寸 OLED (SSD1306) IIC 显示
  *          引脚定义与公共声明
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* ---------------------------------------------------------------------------
 * OLED / AHT20 软件 IIC 引脚定义（两个设备共用一条总线）
 *
 *   OLED  VCC -> 3.3V
 *   OLED  GND -> GND
 *   OLED  SCL -> PB6
 *   OLED  SDA -> PB7
 *   AHT20 VCC/GND -> 3.3V/GND, SCL/SDA 同样接 PB6/PB7
 *
 * 采用开漏输出 + 内部上拉，模块自带上拉电阻时也可以正常工作。
 * 默认 SCL/SDA 同在 GPIOB；若改到其它端口，除下面 4 个宏外，
 * 还需要同步修改 OLED_GPIO_CLK_ENABLE()。
 * --------------------------------------------------------------------------- */
#define OLED_SCL_Pin        GPIO_PIN_6
#define OLED_SCL_GPIO_Port  GPIOB
#define OLED_SDA_Pin        GPIO_PIN_7
#define OLED_SDA_GPIO_Port  GPIOB
#define OLED_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()

/* SSD1306 的 8 位 IIC 从机地址: 7 位地址 0x3C 左移一位 */
#define OLED_I2C_ADDR       0x78U

/* 板载 LED: PC13, 低电平点亮, 用作程序运行指示灯 */
#define LED_Pin             GPIO_PIN_13
#define LED_GPIO_Port       GPIOC

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
