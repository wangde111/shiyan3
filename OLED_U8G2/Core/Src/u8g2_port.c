/**
  ******************************************************************************
  * @file    u8g2_port.c
  * @brief   U8g2 到 STM32F103 HAL 的移植
  *
  * U8g2 本身不直接操作 GPIO，而是通过两个回调完成硬件访问：
  *   u8x8_byte_soft_iic()   : IIC 起始/停止、发送字节
  *   u8x8_gpio_and_delay()  : 毫秒/微秒/纳秒延时
  *
  * 这里使用软件 IIC（PB6=SCL, PB7=SDA），方便在任意引脚上移植。
  ******************************************************************************
  */

#include "u8g2_port.h"
#include "soft_iic.h"
#include "main.h"

u8g2_t u8g2;

/**
  * @brief  U8g2 的字节传输回调
  * @note   START_TRANSFER 时发送从机地址，随后 U8g2 的 CAD 层发送控制字节
  *         0x00(命令) 或 0x40(数据)，最后 END_TRANSFER 发送停止条件。
  */
uint8_t U8G2_ByteCallback(u8x8_t *u8x8, uint8_t msg,
                          uint8_t arg_int, void *arg_ptr)
{
    uint8_t *data;
    uint8_t addr;

    switch (msg)
    {
        case U8X8_MSG_BYTE_INIT:
            SoftIIC_Init();
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            SoftIIC_Start();
            addr = u8x8_GetI2CAddress(u8x8);
            (void)SoftIIC_WriteByte(addr);
            break;

        case U8X8_MSG_BYTE_SEND:
            data = (uint8_t *)arg_ptr;
            while (arg_int-- != 0U)
            {
                (void)SoftIIC_WriteByte(*data++);
            }
            break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            SoftIIC_Stop();
            break;

        default:
            break;
    }

    return 1;
}

/**
  * @brief  U8g2 的 GPIO 和延时回调
  * @note   IIC 方式不需要复位/DC/CS 引脚，只需要处理延时消息。
  */
uint8_t U8G2_GPIOAndDelayCallback(u8x8_t *u8x8, uint8_t msg,
                                  uint8_t arg_int, void *arg_ptr)
{
    volatile uint32_t i;

    (void)u8x8;
    (void)arg_ptr;

    switch (msg)
    {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            break;

        case U8X8_MSG_DELAY_MILLI:
            HAL_Delay((uint32_t)arg_int);
            break;

        case U8X8_MSG_DELAY_10MICRO:
            /* 72 MHz 下约 72 次循环为 10 us，留一点余量 */
            for (i = 0U; i < ((uint32_t)arg_int * 80U); i++)
            {
                __NOP();
            }
            break;

        case U8X8_MSG_DELAY_100NANO:
        case U8X8_MSG_DELAY_NANO:
            __NOP();
            __NOP();
            break;

        default:
            break;
    }

    return 1;
}

void U8G2_Port_Init(void)
{
    /* SSD1306 128x64，全屏缓冲模式 (_f)，IIC 接口 */
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0,
                                           U8G2_ByteCallback,
                                           U8G2_GPIOAndDelayCallback);

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_ClearBuffer(&u8g2);
    u8g2_SendBuffer(&u8g2);
}
