/**
  ******************************************************************************
  * @file    aht20.h
  * @brief   AHT20 温湿度传感器驱动 (IIC 地址 0x38, 8 位写地址 0x70)
  ******************************************************************************
  */

#ifndef __AHT20_H
#define __AHT20_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
    int16_t  temp_x10;      /* 温度 x10, 单位 0.1 摄氏度, 例如 253 = 25.3 C */
    uint16_t hum_x10;       /* 湿度 x10, 单位 0.1 %,   例如 567 = 56.7 % */
    uint8_t  valid;
} AHT20_Data;

/* 返回值: 0 成功, 1 总线错误, 2 传感器忙, 3 未校准 */
uint8_t AHT20_Init(void);
uint8_t AHT20_Read(AHT20_Data *out);

#ifdef __cplusplus
}
#endif

#endif /* __AHT20_H */
