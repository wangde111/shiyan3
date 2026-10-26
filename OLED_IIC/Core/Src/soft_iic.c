/**
  ******************************************************************************
  * @file    soft_iic.c
  * @brief   软件 IIC 总线实现
  *
  * SCL = PB6, SDA = PB7 (可在 main.h 中修改)。
  * SDA 配置为开漏输出; STM32F1 的开漏输出模式下输入施密特触发器仍然有效,
  * 因此可以直接读 IDR 判断从机 ACK 和数据位, 不需要来回切换输入/输出模式。
  ******************************************************************************
  */

#include "soft_iic.h"
#include "main.h"

/* 约 1 us 的半周期延时, 总线速率约 300~400 kHz */
static void SoftIIC_Delay(void)
{
    volatile uint32_t i = 25U;
    while (i-- != 0U)
    {
        __NOP();
    }
}

static void SCL_High(void)
{
    OLED_SCL_GPIO_Port->BSRR = (uint32_t)OLED_SCL_Pin;
}

static void SCL_Low(void)
{
    OLED_SCL_GPIO_Port->BSRR = (uint32_t)OLED_SCL_Pin << 16U;
}

static void SDA_High(void)
{
    OLED_SDA_GPIO_Port->BSRR = (uint32_t)OLED_SDA_Pin;
}

static void SDA_Low(void)
{
    OLED_SDA_GPIO_Port->BSRR = (uint32_t)OLED_SDA_Pin << 16U;
}

static uint8_t SDA_Read(void)
{
    return ((OLED_SDA_GPIO_Port->IDR & OLED_SDA_Pin) != 0U) ? 1U : 0U;
}

void SoftIIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    OLED_GPIO_CLK_ENABLE();

    /* 先释放总线, 再配置为开漏输出 */
    OLED_SCL_GPIO_Port->BSRR = (uint32_t)OLED_SCL_Pin;
    OLED_SDA_GPIO_Port->BSRR = (uint32_t)OLED_SDA_Pin;

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = OLED_SCL_Pin;
    HAL_GPIO_Init(OLED_SCL_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = OLED_SDA_Pin;
    HAL_GPIO_Init(OLED_SDA_GPIO_Port, &GPIO_InitStruct);

    SCL_High();
    SDA_High();
}

void SoftIIC_Start(void)
{
    SDA_High();
    SCL_High();
    SoftIIC_Delay();
    SDA_Low();              /* SCL 高电平期间 SDA 下降沿 */
    SoftIIC_Delay();
    SCL_Low();
    SoftIIC_Delay();
}

void SoftIIC_Stop(void)
{
    SCL_Low();
    SDA_Low();
    SoftIIC_Delay();
    SCL_High();
    SoftIIC_Delay();
    SDA_High();             /* SCL 高电平期间 SDA 上升沿 */
    SoftIIC_Delay();
}

uint8_t SoftIIC_WriteByte(uint8_t byte)
{
    uint8_t i;
    uint8_t ack;

    for (i = 0U; i < 8U; i++)
    {
        SCL_Low();
        if ((byte & 0x80U) != 0U)
        {
            SDA_High();
        }
        else
        {
            SDA_Low();
        }
        SoftIIC_Delay();

        SCL_High();
        SoftIIC_Delay();
        byte <<= 1U;
    }

    /* 第 9 个时钟: 释放 SDA, 读取从机 ACK (低电平有效) */
    SCL_Low();
    SDA_High();
    SoftIIC_Delay();
    SCL_High();
    SoftIIC_Delay();
    ack = SDA_Read();
    SCL_Low();
    SoftIIC_Delay();

    return (ack == 0U) ? 0U : 1U;
}

uint8_t SoftIIC_ReadByte(uint8_t ack)
{
    uint8_t i;
    uint8_t data = 0U;

    SDA_High();             /* 释放 SDA, 由从机驱动 */

    for (i = 0U; i < 8U; i++)
    {
        SCL_High();
        SoftIIC_Delay();
        data <<= 1U;
        if (SDA_Read() != 0U)
        {
            data |= 1U;
        }
        SoftIIC_Delay();
        SCL_Low();
        SoftIIC_Delay();
    }

    /* 第 9 个时钟: 主机发送 ACK(0) 或 NACK(1) */
    if (ack != 0U)
    {
        SDA_Low();
    }
    else
    {
        SDA_High();
    }
    SoftIIC_Delay();
    SCL_High();
    SoftIIC_Delay();
    SCL_Low();
    SoftIIC_Delay();
    SDA_High();

    return data;
}

uint8_t SoftIIC_Write(uint8_t dev_addr, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint8_t err = 0U;

    SoftIIC_Start();
    if (SoftIIC_WriteByte(dev_addr) != 0U)
    {
        err = 1U;
    }
    else
    {
        for (i = 0U; i < len; i++)
        {
            if (SoftIIC_WriteByte(data[i]) != 0U)
            {
                err = 1U;
                break;
            }
        }
    }
    SoftIIC_Stop();

    return err;
}

uint8_t SoftIIC_Read(uint8_t dev_addr, uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint8_t err = 0U;

    SoftIIC_Start();
    if (SoftIIC_WriteByte((uint8_t)(dev_addr | 0x01U)) != 0U)
    {
        err = 1U;
    }
    else
    {
        for (i = 0U; i < len; i++)
        {
            data[i] = SoftIIC_ReadByte((i + 1U < len) ? 1U : 0U);
        }
    }
    SoftIIC_Stop();

    return err;
}
