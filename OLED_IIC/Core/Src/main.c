/**
  ******************************************************************************
  * @file    main.c
  * @brief   实验三：0.96 寸 OLED (IIC) + AHT20 温湿度 + 滚动显示
  *
  * 功能:
  *   1. 显示学号 632407030627 和姓名 王兴;
  *   2. 显示 AHT20 的温度和湿度, 每秒更新一次;
  *   3. 左右滑动显示长字符串:
  *        "Hello，欢迎来到重庆交通大学物联网205实训室！"
  *      软件滚动显示完整长字符串, 另外演示 SSD1306 硬件滚动模式。
  *
  * 硬件连接:
  *   OLED  VCC -> 3.3V, GND -> GND, SCL -> PB6, SDA -> PB7
  *   AHT20 VCC -> 3.3V, GND -> GND, SCL -> PB6, SDA -> PB7
  *   (OLED 地址 0x78, AHT20 地址 0x70/0x71, 可共用同一 IIC 总线)
  ******************************************************************************
  */

#include "main.h"
#include "oled.h"
#include "aht20.h"

#define STUDENT_ID      "632407030627"

/* 用字形序列表示中英文混合字符串, 避免源文件编码影响字符串常量:
 *   Hello，欢迎来到重庆交通大学物联网205实训室！
 *   Hello，欢迎来到
 */
#define G_ASCII(c)      {OLED_GLYPH_ASCII, (uint8_t)(c)}
#define G_CN(i)         {OLED_GLYPH_CN,    (uint8_t)(i)}
#define G_END           {OLED_GLYPH_END,   0U}

static const OLED_Glyph ScrollText[] =
{
    G_ASCII('H'), G_ASCII('e'), G_ASCII('l'), G_ASCII('l'), G_ASCII('o'),
    G_CN(OLED_CN_COMMA),
    G_CN(OLED_CN_HUAN), G_CN(OLED_CN_YING), G_CN(OLED_CN_LAI), G_CN(OLED_CN_DAO),
    G_CN(OLED_CN_CHONG), G_CN(OLED_CN_QING), G_CN(OLED_CN_JIAO),
    G_CN(OLED_CN_TONG), G_CN(OLED_CN_DA), G_CN(OLED_CN_XUE),
    G_CN(OLED_CN_WU), G_CN(OLED_CN_LIAN), G_CN(OLED_CN_NET),
    G_ASCII('2'), G_ASCII('0'), G_ASCII('5'),
    G_CN(OLED_CN_SHI2), G_CN(OLED_CN_XUN), G_CN(OLED_CN_ROOM),
    G_CN(OLED_CN_EXCLAM),
    G_END
};

static const OLED_Glyph HWScrollText[] =
{
    G_ASCII('H'), G_ASCII('e'), G_ASCII('l'), G_ASCII('l'), G_ASCII('o'),
    G_CN(OLED_CN_COMMA),
    G_CN(OLED_CN_HUAN), G_CN(OLED_CN_YING), G_CN(OLED_CN_LAI), G_CN(OLED_CN_DAO),
    G_END
};

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);

static void ShowMainScreen(void);
static void ShowSoftwareScroll(void);
static void ShowHardwareScroll(void);
static void DrawScrollFrame(void);
static void DrawEnvData(const AHT20_Data *data, uint8_t ok);
static void FormatFixed1(char *buf, int16_t value_x10);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    OLED_Init();                /* 内部会初始化软件 IIC 总线 */
    (void)AHT20_Init();         /* AHT20 未接时不影响 OLED 显示 */

    while (1)
    {
        ShowMainScreen();       /* 学号、姓名 + 温湿度, 约 7 秒 */
        ShowSoftwareScroll();   /* 软件滚动完整长字符串, 约 8 秒 */
        ShowHardwareScroll();   /* SSD1306 硬件滚动, 约 7 秒 */
    }
}

/**
  * @brief  主界面: 学号、姓名、AHT20 温湿度, 每秒刷新一次温湿度
  */
static void ShowMainScreen(void)
{
    AHT20_Data data;
    uint8_t i;
    uint8_t ok;

    OLED_Clear();

    /* 第 1 行: 学号。2 个汉字 32 像素 + 12 位数字 96 像素 = 128 像素 */
    OLED_ShowCN(0U, 0U, OLED_CN_XUE);
    OLED_ShowCN(16U, 0U, OLED_CN_HAO);
    OLED_ShowString(32U, 0U, STUDENT_ID);

    /* 第 2 行: 姓名 王兴, 右侧显示 AHT20 */
    OLED_ShowCN(10U, 2U, OLED_CN_XING_LABEL);
    OLED_ShowCN(26U, 2U, OLED_CN_MING);
    OLED_ShowCN(42U, 2U, OLED_CN_WANG);
    OLED_ShowCN(58U, 2U, OLED_CN_XING);
    OLED_ShowString(78U, 2U, "AHT20");

    /* 先读一次并显示温湿度 */
    ok = (AHT20_Read(&data) == 0U) ? 1U : 0U;
    DrawEnvData(&data, ok);
    OLED_Refresh();

    /* 之后每秒更新一次温湿度, 只刷新 page4~page7 */
    for (i = 0U; i < 6U; i++)
    {
        HAL_Delay(1000U);
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

        ok = (AHT20_Read(&data) == 0U) ? 1U : 0U;
        DrawEnvData(&data, ok);
        OLED_RefreshPages(4U, 7U);
    }
}

/**
  * @brief  绘制温度、湿度两行, ok = 0 时显示错误信息
  * @note   只修改 page4~page7, 配合 OLED_RefreshPages(4,7) 做局部刷新
  */
static void DrawEnvData(const AHT20_Data *data, uint8_t ok)
{
    char buf[8];

    OLED_ClearPages(4U, 7U);

    if (ok == 0U)
    {
        OLED_ShowString(44U, 4U, "AHT20");
        OLED_ShowString(44U, 6U, "ERROR");
        return;
    }

    /* 温度: 温度:25.3C */
    OLED_ShowCN(20U, 4U, OLED_CN_WEN);
    OLED_ShowCN(36U, 4U, OLED_CN_DU);
    OLED_ShowString(52U, 4U, ":");
    FormatFixed1(buf, data->temp_x10);
    OLED_ShowString(60U, 4U, buf);
    OLED_ShowString(100U, 4U, "C");

    /* 湿度: 湿度:56.7% */
    OLED_ShowCN(20U, 6U, OLED_CN_SHI);
    OLED_ShowCN(36U, 6U, OLED_CN_DU);
    OLED_ShowString(52U, 6U, ":");
    FormatFixed1(buf, (int16_t)data->hum_x10);
    OLED_ShowString(60U, 6U, buf);
    OLED_ShowString(100U, 6U, "%");
}

/**
  * @brief  把放大 10 倍的整数格式化成 "xx.x" 或 "-xx.x"
  */
static void FormatFixed1(char *buf, int16_t value_x10)
{
    uint16_t value;
    uint8_t n = 0U;

    if (value_x10 < 0)
    {
        buf[n++] = '-';
        value = (uint16_t)(-value_x10);
    }
    else
    {
        value = (uint16_t)value_x10;
    }

    if (value >= 1000U)
    {
        buf[n++] = '1';
        buf[n++] = '0';
        buf[n++] = '0';
    }
    else
    {
        if (value >= 100U)
        {
            buf[n++] = (char)('0' + (value / 100U));
        }
        buf[n++] = (char)('0' + ((value / 10U) % 10U));
    }

    buf[n++] = '.';
    buf[n++] = (char)('0' + (value % 10U));
    buf[n] = '\0';
}

/**
  * @brief  滚动界面的静态标题: 上下各一行, 中间 page2~page3 留给滚动文字
  */
static void DrawScrollFrame(void)
{
    static const uint8_t top_text[6] =
    {
        OLED_CN_CHONG, OLED_CN_QING, OLED_CN_JIAO,
        OLED_CN_TONG,  OLED_CN_DA,   OLED_CN_XUE
    };

    OLED_Clear();

    /* 重庆交通大学, 6 x 16 = 96 像素, 居中 */
    OLED_ShowCNString(16U, 0U, top_text, 6U);

    /* 物联网205实训室, 3 x 16 + 3 x 8 + 3 x 16 = 120 像素 */
    OLED_ShowCN(4U, 6U, OLED_CN_WU);
    OLED_ShowCN(20U, 6U, OLED_CN_LIAN);
    OLED_ShowCN(36U, 6U, OLED_CN_NET);
    OLED_ShowString(52U, 6U, "205");
    OLED_ShowCN(76U, 6U, OLED_CN_SHI2);
    OLED_ShowCN(92U, 6U, OLED_CN_XUN);
    OLED_ShowCN(108U, 6U, OLED_CN_ROOM);

    OLED_Refresh();
}

/**
  * @brief  软件滚动: 显示完整的长字符串, 逐像素左移
  */
static void ShowSoftwareScroll(void)
{
    int16_t width;
    int16_t offset;

    DrawScrollFrame();

    width = OLED_GlyphsWidth(ScrollText);

    /* 从屏幕右侧外面开始, 一直滚动到字符串完全离开左侧 */
    for (offset = -128; offset <= width; offset = (int16_t)(offset + 2))
    {
        OLED_ScrollGlyphs(ScrollText, 2U, offset);
        OLED_RefreshPages(2U, 3U);
        HAL_Delay(25U);
    }

    HAL_Delay(300U);
}

/**
  * @brief  硬件滚动: 使用 SSD1306 自带的 0x26/0x27 滚屏命令
  * @note   硬件滚动只能移动芯片内部 128x64 显存的内容, 所以这里显示
  *         长字符串的开头部分 "Hello，欢迎来到"; 完整长字符串由上面的
  *         软件滚动方式显示。
  */
static void ShowHardwareScroll(void)
{
    DrawScrollFrame();

    /* 把要滚动的文字画在 page2~page3 */
    OLED_ScrollGlyphs(HWScrollText, 2U, 0);
    OLED_RefreshPages(2U, 3U);

    /* page2~page3 水平向左滚动, interval=0 速度最快 */
    OLED_HWScrollStart(OLED_SCROLL_LEFT, 2U, 3U, 0U);

    HAL_Delay(7000U);

    OLED_HWScrollStop();
    HAL_Delay(300U);
}

/**
  * @brief  系统时钟: HSE 8 MHz -> PLL x9 -> SYSCLK 72 MHz
  */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState            = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue      = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                       RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  GPIO 初始化: 仅初始化板载 LED, IIC 引脚在 SoftIIC_Init() 中初始化
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

    GPIO_InitStruct.Pin   = LED_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif
