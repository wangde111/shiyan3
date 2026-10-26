/**
  ******************************************************************************
  * @file    soft_iic.h
  * @brief   软件 IIC 总线 (PB6 = SCL, PB7 = SDA)
  *
  * OLED (SSD1306, 0x78) 和 AHT20 (0x70/0x71) 共用这条总线。
  * 采用开漏输出, 依靠总线上拉电阻产生高电平。
  ******************************************************************************
  */

#ifndef __SOFT_IIC_H
#define __SOFT_IIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 初始化 SCL/SDA 为开漏输出并释放总线 */
void SoftIIC_Init(void);

/* 基本时序 */
void SoftIIC_Start(void);
void SoftIIC_Stop(void);

/* 发送一个字节, 返回 0 = 收到 ACK, 1 = NACK */
uint8_t SoftIIC_WriteByte(uint8_t byte);

/* 读取一个字节, ack = 1 时主机回 ACK, ack = 0 时回 NACK */
uint8_t SoftIIC_ReadByte(uint8_t ack);

/* 连续写: Start + 地址 + 数据 + Stop, 返回 0 成功 */
uint8_t SoftIIC_Write(uint8_t dev_addr, const uint8_t *data, uint16_t len);

/* 连续读: Start + 地址(读) + 数据 + Stop, 返回 0 成功 */
uint8_t SoftIIC_Read(uint8_t dev_addr, uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __SOFT_IIC_H */
