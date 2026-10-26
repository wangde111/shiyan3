/**
  ******************************************************************************
  * @file    oled.c
  * @brief   0.96 寸 OLED (SSD1306, 128x64) 软件 IIC 驱动
  *
  * 一、IIC 接口
  *   SCL = PB6, SDA = PB7, 与 AHT20 共用总线。
  *   从机地址 0x78 (7 位地址 0x3C 左移一位)。
  *
  * 二、SSD1306 显存结构
  *   128 列 x 64 行, 分成 8 页 (page0..page7), 每页 8 行。
  *   每个字节代表一列中的 8 个像素, bit0 在上, bit7 在下。
  *   本驱动先在内存中建立 1 KB 显存, 修改完成后一次刷屏,
  *   支持整屏刷新和局部页刷新, 避免闪烁。
  *
  * 三、汉字显示
  *   16x16 汉字由 32 字节组成:
  *     bytes[0..15]  -> 上半部分 8 行, 每字节一列, bit0 在上
  *     bytes[16..31] -> 下半部分 8 行, 每字节一列, bit0 在上
  *   显示时上半部分写入 page, 下半部分写入 page+1。
  ******************************************************************************
  */

#include "oled.h"
#include "soft_iic.h"
#include "main.h"

#include <string.h>

#define OLED_CTRL_CMD       0x00U
#define OLED_CTRL_DATA      0x40U

/* 默认显存, 1 KB */
static uint8_t OLED_GRAM[OLED_WIDTH * OLED_PAGES];

/* ------------------------------- IIC 封装 -------------------------------- */

static void OLED_Write(uint8_t control, const uint8_t *data, uint16_t len)
{
    uint16_t i;

    SoftIIC_Start();
    if (SoftIIC_WriteByte(OLED_I2C_ADDR) == 0U)
    {
        (void)SoftIIC_WriteByte(control);
        for (i = 0U; i < len; i++)
        {
            (void)SoftIIC_WriteByte(data[i]);
        }
    }
    SoftIIC_Stop();
}

static void OLED_WriteCmd(uint8_t cmd)
{
    OLED_Write(OLED_CTRL_CMD, &cmd, 1U);
}

static void OLED_WriteCmds(const uint8_t *cmds, uint16_t len)
{
    OLED_Write(OLED_CTRL_CMD, cmds, len);
}

/* -------------------------------- 初始化 --------------------------------- */

void OLED_Init(void)
{
    /* SSD1306 上电初始化序列 */
    static const uint8_t init_cmds[] =
    {
        0xAEU,              /* 关显示 */
        0xD5U, 0x80U,       /* 显示时钟分频 */
        0xA8U, 0x3FU,       /* 复用率 1/64 */
        0xD3U, 0x00U,       /* 显示偏移 0 */
        0x40U,              /* 起始行 0 */
        0x8DU, 0x14U,       /* 开内部电荷泵 */
        0x20U, 0x00U,       /* 水平寻址模式 */
        0xA1U,              /* 列地址 127 -> 0 */
        0xC8U,              /* COM 扫描方向: 从下到上 */
        0xDAU, 0x12U,       /* COM 引脚配置 */
        0x81U, 0xCFU,       /* 对比度 */
        0xD9U, 0xF1U,       /* 预充电周期 */
        0xDBU, 0x40U,       /* VCOMH 电压 */
        0xA4U,              /* 显示跟随显存 */
        0xA6U,              /* 正常显示 */
        0x2EU,              /* 关闭滚动 */
        0xAFU               /* 开显示 */
    };

    SoftIIC_Init();
    HAL_Delay(100U);                          /* 等待 OLED 电源稳定 */

    OLED_WriteCmds(init_cmds, (uint16_t)sizeof(init_cmds));

    OLED_Clear();
    OLED_Refresh();
}

/* -------------------------------- 刷新 API -------------------------------- */

void OLED_Clear(void)
{
    (void)memset(OLED_GRAM, 0x00, sizeof(OLED_GRAM));
}

void OLED_ClearPages(uint8_t page_start, uint8_t page_end)
{
    if ((page_start > page_end) || (page_end >= OLED_PAGES))
    {
        return;
    }

    (void)memset(&OLED_GRAM[page_start * OLED_WIDTH], 0x00,
                 (uint32_t)(page_end - page_start + 1U) * OLED_WIDTH);
}

void OLED_RefreshPages(uint8_t page_start, uint8_t page_end)
{
    uint8_t setup[6];

    if ((page_start > page_end) || (page_end >= OLED_PAGES))
    {
        return;
    }

    setup[0] = 0x21U;                         /* 列地址 0..127 */
    setup[1] = 0x00U;
    setup[2] = (uint8_t)(OLED_WIDTH - 1U);
    setup[3] = 0x22U;                         /* 页地址 */
    setup[4] = page_start;
    setup[5] = page_end;

    OLED_WriteCmds(setup, (uint16_t)sizeof(setup));
    OLED_Write(OLED_CTRL_DATA,
               &OLED_GRAM[page_start * OLED_WIDTH],
               (uint16_t)((page_end - page_start + 1U) * OLED_WIDTH));
}

void OLED_Refresh(void)
{
    OLED_RefreshPages(0U, (uint8_t)(OLED_PAGES - 1U));
}

void OLED_DisplayOn(void)
{
    OLED_WriteCmd(0xAFU);
}

void OLED_DisplayOff(void)
{
    OLED_WriteCmd(0xAEU);
}

void OLED_SetContrast(uint8_t contrast)
{
    uint8_t cmds[2];

    cmds[0] = 0x81U;
    cmds[1] = contrast;
    OLED_WriteCmds(cmds, 2U);
}

/* -------------------------------- 绘图 API -------------------------------- */

void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color)
{
    if ((x >= OLED_WIDTH) || (y >= OLED_HEIGHT))
    {
        return;
    }

    if (color != 0U)
    {
        OLED_GRAM[(y / 8U) * OLED_WIDTH + x] |= (uint8_t)(1U << (y % 8U));
    }
    else
    {
        OLED_GRAM[(y / 8U) * OLED_WIDTH + x] &= (uint8_t)~(1U << (y % 8U));
    }
}

void OLED_ShowCharAt(int16_t x, uint8_t page, char ch)
{
    const uint8_t *font;
    uint8_t i;
    int16_t px;

    if (page >= (OLED_PAGES - 1U))
    {
        return;
    }
    if ((ch < ' ') || (ch > '~'))
    {
        ch = ' ';
    }

    font = &OLED_ASCII8x16[(uint8_t)(ch - ' ') * 16U];

    for (i = 0U; i < 8U; i++)
    {
        px = (int16_t)(x + (int16_t)i);
        if ((px >= 0) && (px < (int16_t)OLED_WIDTH))
        {
            OLED_GRAM[page * OLED_WIDTH + (uint16_t)px]        = font[i];
            OLED_GRAM[(page + 1U) * OLED_WIDTH + (uint16_t)px] = font[8U + i];
        }
    }
}

void OLED_ShowCNAt(int16_t x, uint8_t page, uint8_t index)
{
    const uint8_t *font;
    uint8_t i;
    int16_t px;

    if ((index >= OLED_CN_COUNT) || (page >= (OLED_PAGES - 1U)))
    {
        return;
    }

    font = OLED_CN16x16[index];

    for (i = 0U; i < 16U; i++)
    {
        px = (int16_t)(x + (int16_t)i);
        if ((px >= 0) && (px < (int16_t)OLED_WIDTH))
        {
            OLED_GRAM[page * OLED_WIDTH + (uint16_t)px]        = font[i];
            OLED_GRAM[(page + 1U) * OLED_WIDTH + (uint16_t)px] = font[16U + i];
        }
    }
}

void OLED_ShowChar(uint8_t x, uint8_t page, char ch)
{
    OLED_ShowCharAt((int16_t)x, page, ch);
}

void OLED_ShowString(uint8_t x, uint8_t page, const char *str)
{
    if (str == NULL)
    {
        return;
    }

    while ((*str != '\0') && (x <= (OLED_WIDTH - 8U)))
    {
        OLED_ShowChar(x, page, *str);
        x += 8U;
        str++;
    }
}

void OLED_ShowCN(uint8_t x, uint8_t page, uint8_t index)
{
    OLED_ShowCNAt((int16_t)x, page, index);
}

void OLED_ShowCNString(uint8_t x, uint8_t page,
                       const uint8_t *indices, uint8_t count)
{
    uint8_t i;

    if (indices == NULL)
    {
        return;
    }

    for (i = 0U; i < count; i++)
    {
        if (x > (OLED_WIDTH - 16U))
        {
            break;
        }
        OLED_ShowCN(x, page, indices[i]);
        x += 16U;
    }
}

void OLED_ShowNum(uint8_t x, uint8_t page, uint32_t num, uint8_t len)
{
    char buf[12];
    uint8_t i;

    if (len > 10U)
    {
        len = 10U;
    }

    buf[len] = '\0';
    for (i = len; i > 0U; i--)
    {
        buf[i - 1U] = (char)('0' + (num % 10U));
        num /= 10U;
    }

    OLED_ShowString(x, page, buf);
}

/* ------------------------------ 中英文滚屏 ------------------------------ */

int16_t OLED_GlyphsWidth(const OLED_Glyph *glyphs)
{
    int16_t width = 0;

    if (glyphs == NULL)
    {
        return 0;
    }

    while (glyphs->type != OLED_GLYPH_END)
    {
        width = (int16_t)(width + ((glyphs->type == OLED_GLYPH_CN) ? 16 : 8));
        glyphs++;
    }

    return width;
}

void OLED_ScrollGlyphs(const OLED_Glyph *glyphs, uint8_t page, int16_t offset)
{
    int16_t x;

    if ((glyphs == NULL) || (page >= (OLED_PAGES - 1U)))
    {
        return;
    }

    OLED_ClearPages(page, (uint8_t)(page + 1U));

    x = (int16_t)(-offset);
    while (glyphs->type != OLED_GLYPH_END)
    {
        if (glyphs->type == OLED_GLYPH_CN)
        {
            OLED_ShowCNAt(x, page, glyphs->value);
            x = (int16_t)(x + 16);
        }
        else
        {
            OLED_ShowCharAt(x, page, (char)glyphs->value);
            x = (int16_t)(x + 8);
        }
        glyphs++;
    }
}

/* ------------------------------ 硬件滚动 API ------------------------------ */

void OLED_HWScrollStart(uint8_t direction, uint8_t start_page,
                        uint8_t end_page, uint8_t interval)
{
    uint8_t cmds[7];

    if ((start_page > end_page) || (end_page >= OLED_PAGES))
    {
        return;
    }

    OLED_WriteCmd(0x2EU);                     /* 先关闭滚动 */

    cmds[0] = (direction == OLED_SCROLL_RIGHT) ? 0x26U : 0x27U;
    cmds[1] = 0x00U;                          /* 虚拟字节 */
    cmds[2] = (uint8_t)(start_page & 0x07U);
    cmds[3] = (uint8_t)(interval & 0x07U);
    cmds[4] = (uint8_t)(end_page & 0x07U);
    cmds[5] = 0x00U;                          /* 虚拟字节 */
    cmds[6] = 0xFFU;                          /* 虚拟字节 */

    OLED_WriteCmds(cmds, (uint16_t)sizeof(cmds));
    OLED_WriteCmd(0x2FU);                     /* 开启滚动 */
}

void OLED_HWScrollStop(void)
{
    OLED_WriteCmd(0x2EU);
}
