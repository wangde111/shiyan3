#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate SSD1306 fonts used by this project.

Chinese font:
    16 x 16 pixels, column-major, 32 bytes/character.
    bytes[0..15]  : upper 8 rows, one byte per column, LSB = top pixel
    bytes[16..31] : lower 8 rows, one byte per column, LSB = row 8 pixel

ASCII font:
    8 x 16 pixels, column-major, 16 bytes/character.
    bytes[0..7]   : upper 8 rows
    bytes[8..15]  : lower 8 rows

Usage:
    python tools/gen_font.py
"""

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
FONT_PATH = Path(r"C:\Windows\Fonts\simhei.ttf")
CN_CHARS = "学号姓名王兴温度湿欢迎来到重庆交通大物联网实训室，！"
ASCII_FIRST = 0x20
ASCII_LAST = 0x7E


def render_char(ch: str, width: int, height: int, center_vertical: bool) -> list[int]:
    font = ImageFont.truetype(str(FONT_PATH), 16)
    image = Image.new("1", (width, height), 0)
    draw = ImageDraw.Draw(image)

    left, top, right, bottom = font.getbbox(ch)
    x = (width - (right - left)) // 2 - left
    if center_vertical:
        y = (height - (bottom - top)) // 2 - top
    else:
        y = 0
    draw.text((x, y), ch, font=font, fill=1)

    data: list[int] = []
    for page in range(height // 8):
        for col in range(width):
            value = 0
            for bit in range(8):
                if image.getpixel((col, page * 8 + bit)):
                    value |= 1 << bit
            data.append(value)
    return data


def ascii_art(data: list[int], width: int, height: int) -> str:
    lines = []
    for y in range(height):
        line = ""
        for x in range(width):
            byte = data[(y // 8) * width + x]
            line += "#" if byte & (1 << (y % 8)) else "."
        lines.append(line)
    return "\n".join(lines)


def format_bytes(values: list[int]) -> str:
    return ", ".join(f"0x{v:02X}" for v in values)


def main() -> None:
    cn_data = [render_char(ch, 16, 16, center_vertical=True) for ch in CN_CHARS]
    ascii_data = [
        render_char(chr(code), 8, 16, center_vertical=False)
        for code in range(ASCII_FIRST, ASCII_LAST + 1)
    ]

    header = f"""/**
  ******************************************************************************
  * @file    oledfont.h
  * @brief   SSD1306 点阵字库: 16x16 Chinese and 8x16 ASCII.
  ******************************************************************************
  */

#ifndef __OLEDFONT_H
#define __OLEDFONT_H

#ifdef __cplusplus
extern "C" {{
#endif

#include <stdint.h>

/* 16x16 Chinese characters, order:
   学 号 姓 名 王 兴 温 度 湿 欢 迎 来 到 重 庆 交 通 大 物 联 网 实 训 室 ，！ */
#define OLED_CN_XUE        0U
#define OLED_CN_HAO        1U
#define OLED_CN_XING_LABEL 2U
#define OLED_CN_MING       3U
#define OLED_CN_WANG       4U
#define OLED_CN_XING       5U
#define OLED_CN_WEN        6U
#define OLED_CN_DU         7U
#define OLED_CN_SHI        8U
#define OLED_CN_HUAN       9U
#define OLED_CN_YING       10U
#define OLED_CN_LAI        11U
#define OLED_CN_DAO        12U
#define OLED_CN_CHONG      13U
#define OLED_CN_QING       14U
#define OLED_CN_JIAO       15U
#define OLED_CN_TONG       16U
#define OLED_CN_DA         17U
#define OLED_CN_WU         18U
#define OLED_CN_LIAN       19U
#define OLED_CN_NET        20U
#define OLED_CN_SHI2       21U
#define OLED_CN_XUN        22U
#define OLED_CN_ROOM       23U
#define OLED_CN_COMMA      24U
#define OLED_CN_EXCLAM     25U
#define OLED_CN_COUNT      26U

/* 8x16 ASCII 字库覆盖的字符范围 */
#define ASCII_FIRST        0x20U
#define ASCII_LAST         0x7EU

extern const uint8_t OLED_CN16x16[OLED_CN_COUNT][32];
extern const uint8_t OLED_ASCII8x16[(ASCII_LAST - ASCII_FIRST + 1) * 16];

#ifdef __cplusplus
}}
#endif

#endif /* __OLEDFONT_H */
"""

    source_lines = [
        "/**",
        "  ******************************************************************************",
        "  * @file    oledfont.c",
        "  * @brief   SSD1306 点阵字库数据 (由 tools/gen_font.py 生成).",
        "  *",
        "  * 取模方式: 纵向取模, 字节低位在上.",
        "  *   16x16 汉字: 每字 32 字节, 前 16 字节为上半屏, 后 16 字节为下半屏;",
        "  *   8x16 ASCII: 每字 16 字节, 前 8 字节为上半屏, 后 8 字节为下半屏.",
        "  ******************************************************************************",
        "  */",
        "",
        '#include "oledfont.h"',
        "",
        "/* 16x16 汉字: 学 号 姓 名 王 兴 温 度 湿 欢 迎 来 到 重 庆 交 通 大 物 联 网 实 训 室 ，！ */",
        "const uint8_t OLED_CN16x16[OLED_CN_COUNT][32] = {",
    ]
    for ch, data in zip(CN_CHARS, cn_data):
        source_lines.append(f"    /* {ch} */")
        source_lines.append("    {" + format_bytes(data[:16]) + ",")
        source_lines.append("     " + format_bytes(data[16:]) + "},")
    source_lines.append("};")
    source_lines.append("")
    source_lines.append("/* 8x16 ASCII: 0x20..0x7E, 纵向取模 */")
    source_lines.append(
        f"const uint8_t OLED_ASCII8x16[{(ASCII_LAST - ASCII_FIRST + 1) * 16}] = {{"
    )
    for code, data in zip(range(ASCII_FIRST, ASCII_LAST + 1), ascii_data):
        ch = chr(code)
        if ch == " ":
            label = "space"
        elif ch == "\\":
            label = "backslash"
        else:
            label = ch
        source_lines.append(f"    /* 0x{code:02X} '{label}' */")
        source_lines.append("    " + format_bytes(data) + ",")
    source_lines.append("};")
    source_lines.append("")

    inc_dir = ROOT / "OLED_IIC" / "Core" / "Inc"
    src_dir = ROOT / "OLED_IIC" / "Core" / "Src"
    (inc_dir / "oledfont.h").write_text(header, encoding="utf-8", newline="\n")
    (src_dir / "oledfont.c").write_text(
        "\n".join(source_lines) + "\n", encoding="utf-8", newline="\n"
    )

    print("Generated oledfont.h / oledfont.c")
    for ch, data in zip(CN_CHARS, cn_data):
        print(f"\n{ch}")
        print(ascii_art(data, 16, 16))
    for ch in "632407030627":
        data = ascii_data[ord(ch) - ASCII_FIRST]
        print(f"\n{ch}")
        print(ascii_art(data, 8, 16))


if __name__ == "__main__":
    main()
