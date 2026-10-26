U8g2 裁剪说明
==============

版本：U8g2 2.37.1
来源：https://github.com/olikraus/u8g2
许可：BSD-2-Clause（见 U8g2 仓库 LICENSE）

本目录只保留了 STM32F103 + SSD1306 128x64 IIC OLED 所需的最小文件集：

    u8g2.h / u8x8.h
    u8g2_setup.c
    u8g2_buffer.c          （删除了 PBM/XBM 导出函数）
    u8g2_cleardisplay.c
    u8g2_d_setup.c         （只保留 ssd1306_i2c_128x64_noname_f）
    u8g2_d_memory.c        （只保留 128x64 全屏缓冲 1 KB）
    u8g2_font.c
    u8g2_fonts_min.c       （只保留 6x10、8x13、10x20 三个 ASCII 字体）
    u8g2_hvline.c
    u8g2_ll_hvline.c
    u8g2_line.c
    u8g2_box.c
    u8g2_circle.c
    u8g2_arc.c
    u8g2_polygon.c
    u8g2_bitmap.c
    u8g2_kerning.c
    u8g2_intersection.c
    u8x8_d_ssd1306_128x64_noname.c
    u8x8_display.c
    u8x8_setup.c
    u8x8_cad.c
    u8x8_byte.c
    u8x8_gpio.c
    u8x8_string.c
    u8x8_8x8.c

裁剪原则：
1. 只保留 SSD1306 128x64 IIC 的 setup 和显存函数；
2. 删除 u8g2_fonts.c 中未使用的字体，避免 39 MB 源文件占用 Flash；
3. 删除 u8g2_buffer.c 中的 PBM/XBM 导出函数，避免引入 u8x8_capture.c；
4. 删除 U8x8 文本字体和未使用的显示驱动。

移植接口在 Core/Src/u8g2_port.c：
    U8G2_ByteCallback()          软件 IIC 字节收发
    U8G2_GPIOAndDelayCallback()  HAL 延时
    U8G2_Port_Init()             u8g2_Setup_ssd1306_i2c_128x64_noname_f()

U8g2 官方 API 文档：
    https://github.com/olikraus/u8g2/wiki
