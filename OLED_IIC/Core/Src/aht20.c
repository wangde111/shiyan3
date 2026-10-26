/**
  ******************************************************************************
  * @file    aht20.c
  * @brief   AHT20 温湿度传感器驱动
  *
  * 与 OLED 共用软件 IIC 总线 (PB6/PB7)。
  * AHT20 7 位地址 0x38, 8 位写地址 0x70, 读地址 0x71。
  *
  * 测量流程:
  *   1. 发送 0xAC 0x33 0x00 触发测量;
  *   2. 等待 80 ms;
  *   3. 读取 6 字节: 状态 + 湿度 20 bit + 温度 20 bit。
  ******************************************************************************
  */

#include "aht20.h"
#include "soft_iic.h"
#include "main.h"

#define AHT20_ADDR_W        0x70U
#define AHT20_STATUS_BUSY   0x80U
#define AHT20_STATUS_CAL    0x08U

static uint8_t AHT20_ReadStatus(void)
{
    uint8_t status = 0xFFU;

    (void)SoftIIC_Read(AHT20_ADDR_W, &status, 1U);
    return status;
}

uint8_t AHT20_Init(void)
{
    uint8_t status;
    static const uint8_t init_cmd[3] = {0xBEU, 0x08U, 0x00U};

    SoftIIC_Init();

    /* 上电后至少等待 40 ms 才能读状态 */
    HAL_Delay(40U);
    status = AHT20_ReadStatus();

    if ((status & AHT20_STATUS_CAL) == 0U)
    {
        (void)SoftIIC_Write(AHT20_ADDR_W, init_cmd, 3U);
        HAL_Delay(10U);
        status = AHT20_ReadStatus();
    }

    if (((status & AHT20_STATUS_CAL) != 0U) &&
        ((status & AHT20_STATUS_BUSY) == 0U))
    {
        return 0U;
    }
    return 3U;
}

uint8_t AHT20_Read(AHT20_Data *out)
{
    uint8_t buf[6];
    uint32_t hum_raw;
    uint32_t temp_raw;
    static const uint8_t trig_cmd[3] = {0xACU, 0x33U, 0x00U};

    if (out == 0)
    {
        return 1U;
    }
    out->valid = 0U;

    /* 触发一次测量 */
    if (SoftIIC_Write(AHT20_ADDR_W, trig_cmd, 3U) != 0U)
    {
        return 1U;
    }

    /* 测量时间典型 80 ms */
    HAL_Delay(80U);

    if (SoftIIC_Read(AHT20_ADDR_W, buf, 6U) != 0U)
    {
        return 1U;
    }

    if ((buf[0] & AHT20_STATUS_BUSY) != 0U)
    {
        return 2U;
    }
    if ((buf[0] & AHT20_STATUS_CAL) == 0U)
    {
        return 3U;
    }

    /* 湿度: buf[1..3] 的高 20 bit */
    hum_raw = ((uint32_t)buf[1] << 12) |
              ((uint32_t)buf[2] << 4)  |
              ((uint32_t)buf[3] >> 4);

    /* 温度: buf[3] 的低 4 bit + buf[4] + buf[5], 共 20 bit */
    temp_raw = (((uint32_t)buf[3] & 0x0FU) << 16) |
               ((uint32_t)buf[4] << 8) |
               (uint32_t)buf[5];

    /* 湿度 = raw / 2^20 * 100 %, 放大 10 倍 */
    out->hum_x10 = (uint16_t)(((uint64_t)hum_raw * 1000ULL) >> 20);

    /* 温度 = raw / 2^20 * 200 - 50, 放大 10 倍 */
    out->temp_x10 = (int16_t)((int32_t)(((uint64_t)temp_raw * 2000ULL) >> 20) - 500);

    out->valid = 1U;
    return 0U;
}
