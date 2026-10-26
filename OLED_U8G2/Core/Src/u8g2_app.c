/**
  ******************************************************************************
  * @file    u8g2_app.c
  * @brief   U8g2 在 STM32F103 + SSD1306 128x64 IIC 上的显示例程
  *
  * 包含:
  *   1. U8g2 demo: 直线、圆、矩形、不同字体;
  *   2. 学号 632407030627 + 姓名 王兴 (汉字用 16x16 XBM);
  *   3. 水平滚动: 使用 640x64 虚拟缓冲，1 像素步进;
  *   4. 垂直滚动: U8g2 文本 + 裁剪窗口，2 像素步进;
  *   5. 动态图案: 旋转雷达扫描线 + 目标点。
  ******************************************************************************
  */

#include "u8g2_app.h"
#include "u8g2_port.h"
#include "cn_font.h"
#include "main.h"

#include <string.h>

/* 虚拟宽屏缓冲: 640 x 64, 用于水平无缝滚动 */
#define WIDE_TILE_WIDTH     80U
#define WIDE_WIDTH          (WIDE_TILE_WIDTH * 8U)

static uint8_t wide_buf[WIDE_TILE_WIDTH * 8U * 8U];
static u8g2_t wide_u8g2;
static u8x8_display_info_t wide_info;
static uint8_t wide_ready = 0U;

/* 32 方向单位圆查表, 半径 26 */
static const int8_t sin_tab[32] =
{
    0, 5, 10, 14, 18, 22, 24, 26, 26, 26, 24, 22, 18, 14, 10, 5,
    0, -5, -10, -14, -18, -22, -24, -26, -26, -26, -24, -22, -18, -14, -10, -5
};

static const int8_t cos_tab[32] =
{
    26, 26, 24, 22, 18, 14, 10, 5, 0, -5, -10, -14, -18, -22, -24, -26,
    -26, -26, -24, -22, -18, -14, -10, -5, 0, 5, 10, 14, 18, 22, 24, 26
};

/* ------------------------------ 虚拟宽屏 -------------------------------- */

static void Wide_Init(void)
{
    if (wide_ready != 0U)
    {
        return;
    }

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&wide_u8g2, U8G2_R0,
                                           U8G2_ByteCallback,
                                           U8G2_GPIOAndDelayCallback);

    /* 复制显示信息并扩大为 640x64 的虚拟屏 */
    wide_info = *wide_u8g2.u8x8.display_info;
    wide_info.tile_width = (uint8_t)WIDE_TILE_WIDTH;
    wide_info.pixel_width = (uint16_t)WIDE_WIDTH;

    wide_u8g2.u8x8.display_info = &wide_info;
    wide_u8g2.tile_buf_ptr = wide_buf;
    wide_u8g2.tile_buf_height = 8U;
    wide_u8g2.pixel_buf_width = (u8g2_uint_t)WIDE_WIDTH;
    wide_u8g2.pixel_buf_height = 64U;
    wide_u8g2.width = (u8g2_uint_t)WIDE_WIDTH;
    wide_u8g2.height = 64U;

    u8g2_SetFont(&wide_u8g2, u8g2_font_8x13_tf);
    u8g2_SetDrawColor(&wide_u8g2, 1U);
    (void)memset(wide_buf, 0, sizeof(wide_buf));

    wide_ready = 1U;
}

/**
  * @brief  把虚拟宽屏中 x_offset 开始的 128x64 窗口复制到真实显存
  */
static void Wide_Blit(uint16_t x_offset)
{
    uint8_t *dst = u8g2_GetBufferPtr(&u8g2);
    uint8_t page;
    uint8_t i;

    /* U8g2 的显存是“每列一个字节”，所以水平移动就是移动列索引，
       不需要在字节内部做移位。 */
    if (x_offset >= WIDE_WIDTH)
    {
        (void)memset(dst, 0, 128U * 8U);
        return;
    }

    for (page = 0U; page < 8U; page++)
    {
        const uint8_t *src = &wide_buf[page * WIDE_TILE_WIDTH * 8U];

        for (i = 0U; i < 16U; i++)
        {
            uint16_t idx = (uint16_t)(x_offset + i);
            dst[page * 128U + i] = (idx < WIDE_WIDTH) ? src[idx] : 0U;
        }
    }
}

/* ------------------------------ 显示界面 -------------------------------- */

/**
  * @brief  1. U8g2 自带 demo 风格界面
  */
static void ShowU8g2Demo(void)
{
    u8g2_ClearBuffer(&u8g2);

    u8g2_SetFont(&u8g2, u8g2_font_10x20_tf);
    u8g2_DrawStr(&u8g2, 0, 20, "U8g2");

    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 62, 18, "Demo");
    u8g2_DrawHLine(&u8g2, 0, 24, 128);
    u8g2_DrawFrame(&u8g2, 2, 27, 124, 35);

    u8g2_DrawCircle(&u8g2, 24, 45, 12, U8G2_DRAW_ALL);
    u8g2_DrawDisc(&u8g2, 24, 45, 5, U8G2_DRAW_ALL);

    u8g2_DrawLine(&u8g2, 48, 34, 60, 28);
    u8g2_DrawLine(&u8g2, 60, 28, 72, 40);
    u8g2_DrawLine(&u8g2, 72, 40, 84, 34);
    u8g2_DrawBox(&u8g2, 90, 28, 30, 12);

    u8g2_SetFont(&u8g2, u8g2_font_8x13_tf);
    u8g2_DrawStr(&u8g2, 46, 58, "SSD1306");

    u8g2_SendBuffer(&u8g2);
    HAL_Delay(4000U);
}

/**
  * @brief  2. 学号 + 姓名（王兴使用 16x16 XBM）
  */
static void ShowIdName(void)
{
    u8g2_ClearBuffer(&u8g2);

    u8g2_SetFont(&u8g2, u8g2_font_8x13_tf);
    u8g2_DrawStr(&u8g2, 0, 14, "ID:632407030627");
    u8g2_DrawStr(&u8g2, 0, 36, "Name:");

    u8g2_DrawXBMP(&u8g2, 52, 22, 16, 16, cn16_wang);
    u8g2_DrawXBMP(&u8g2, 70, 22, 16, 16, cn16_xing);

    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 56, "STM32F103 + U8g2");

    u8g2_SendBuffer(&u8g2);
    HAL_Delay(4000U);
}

/**
  * @brief  3a. 水平滚动：640x64 虚拟缓冲 + 1 像素步进
  */
static void ShowHorizontalScroll(void)
{
    static const char text[] =
        "Hello, welcome to CQJTU IoT 205 Lab!  U8g2 + STM32F103.  ";
    int16_t text_width;
    int16_t offset;

    Wide_Init();

    /* 顶部固定标题 */
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFont(&u8g2, u8g2_font_8x13_tf);
    u8g2_DrawStr(&u8g2, 0, 10, "Horizontal Scroll");
    u8g2_DrawHLine(&u8g2, 0, 12, 128);
    u8g2_SendBuffer(&u8g2);

    /* 在虚拟宽屏中把文字画在 x=128 处，之后向左移动窗口 */
    u8g2_ClearBuffer(&wide_u8g2);
    u8g2_SetFont(&wide_u8g2, u8g2_font_8x13_tf);
    u8g2_DrawStr(&wide_u8g2, 128, 44, text);
    text_width = (int16_t)u8g2_GetStrWidth(&wide_u8g2, text);

    for (offset = 0; offset <= (int16_t)(128 + text_width); offset += 3)
    {
        Wide_Blit((uint16_t)offset);

        /* 重新画固定标题，覆盖 Blit 带进来的背景 */
        u8g2_SetFont(&u8g2, u8g2_font_8x13_tf);
        u8g2_DrawStr(&u8g2, 0, 10, "Horizontal Scroll");
        u8g2_DrawHLine(&u8g2, 0, 12, 128);

        u8g2_SendBuffer(&u8g2);
        HAL_Delay(20U);
    }
}

/**
  * @brief  3b. 垂直滚动：文本行 + U8g2 裁剪窗口
  */
static void ShowVerticalScroll(void)
{
    static const char *lines[] =
    {
        "U8g2 Library",
        "STM32F103C8T6",
        "SSD1306 128x64",
        "I2C: PB6 / PB7",
        "Horizontal Scroll",
        "Vertical Scroll",
        "Animated Pattern",
        "CQJTU IoT 205"
    };

    const int16_t line_count = (int16_t)(sizeof(lines) / sizeof(lines[0]));
    const int16_t total_height = (int16_t)(line_count * 14 + 16);
    int16_t offset;
    int16_t i;
    int16_t y;

    for (offset = 0; offset <= total_height; offset += 2)
    {
        u8g2_ClearBuffer(&u8g2);

        u8g2_SetFont(&u8g2, u8g2_font_8x13_tf);
        u8g2_DrawStr(&u8g2, 0, 10, "Vertical Scroll");
        u8g2_DrawHLine(&u8g2, 0, 12, 128);

        /* 只允许在 page2~page7 显示滚动内容 */
        u8g2_SetClipWindow(&u8g2, 0, 14, 127, 63);

        u8g2_SetFont(&u8g2, u8g2_font_8x13_tf);
        for (i = 0; i < line_count; i++)
        {
            y = (int16_t)(24 + i * 14 - offset);
            if ((y >= 0) && (y < 64))
            {
                u8g2_DrawStr(&u8g2, 4, (u8g2_uint_t)y, lines[i]);
            }
        }

        u8g2_SetMaxClipWindow(&u8g2);
        u8g2_SendBuffer(&u8g2);
        HAL_Delay(30U);
    }
}

/**
  * @brief  4. 动态雷达扫描图案
  */
static void ShowRadar(void)
{
    static const uint8_t blip_angle[4] = {3U, 11U, 20U, 27U};
    static const uint8_t blip_radius[4] = {20U, 12U, 23U, 16U};
    uint8_t angle = 0U;
    uint16_t frame;
    uint8_t i;
    int16_t bx;
    int16_t by;

    for (frame = 0U; frame < 120U; frame++)
    {
        u8g2_ClearBuffer(&u8g2);

        u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
        u8g2_DrawStr(&u8g2, 35, 10, "U8g2 Radar");

        u8g2_DrawCircle(&u8g2, 64, 38, 26, U8G2_DRAW_ALL);
        u8g2_DrawCircle(&u8g2, 64, 38, 18, U8G2_DRAW_ALL);
        u8g2_DrawCircle(&u8g2, 64, 38, 9, U8G2_DRAW_ALL);
        u8g2_DrawHLine(&u8g2, 38, 38, 53);
        u8g2_DrawVLine(&u8g2, 64, 12, 53);

        /* 扫描线 */
        u8g2_DrawLine(&u8g2, 64, 38,
                      (u8g2_uint_t)(64 + cos_tab[angle]),
                      (u8g2_uint_t)(38 + sin_tab[angle]));
        u8g2_DrawDisc(&u8g2, (u8g2_uint_t)(64 + cos_tab[angle]),
                      (u8g2_uint_t)(38 + sin_tab[angle]), 2, U8G2_DRAW_ALL);

        /* 扫描线扫过时点亮目标点 */
        for (i = 0U; i < 4U; i++)
        {
            uint8_t delta = (uint8_t)((angle + 32U - blip_angle[i]) & 31U);
            if (delta < 3U)
            {
                bx = (int16_t)(64 + (cos_tab[blip_angle[i]] * blip_radius[i]) / 26);
                by = (int16_t)(38 + (sin_tab[blip_angle[i]] * blip_radius[i]) / 26);
                u8g2_DrawDisc(&u8g2, (u8g2_uint_t)bx, (u8g2_uint_t)by,
                              2, U8G2_DRAW_ALL);
            }
        }

        u8g2_SendBuffer(&u8g2);
        HAL_Delay(40U);

        angle = (uint8_t)((angle + 1U) & 31U);
    }
}

void U8G2_App_Run(void)
{
    while (1)
    {
        ShowU8g2Demo();
        ShowIdName();
        ShowHorizontalScroll();
        ShowVerticalScroll();
        ShowRadar();
    }
}
