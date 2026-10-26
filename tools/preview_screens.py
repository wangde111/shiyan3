#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Render OLED screens from oledfont.c for visual checking.

Usage:
    python tools/preview_screens.py

Output:
    OLED_IIC/Docs/screens_preview.png
"""

import re
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
FONT_C = ROOT / "OLED_IIC" / "Core" / "Src" / "oledfont.c"
OUT = ROOT / "OLED_IIC" / "Docs" / "screens_preview.png"

W, H = 128, 64
PAGES = 8


def load_fonts():
    src = FONT_C.read_text(encoding="utf-8")
    src = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
    cn_block = re.search(
        r"OLED_CN16x16\[OLED_CN_COUNT\]\[32\]\s*=\s*\{(.*?)\};", src, re.S
    ).group(1)
    ascii_block = re.search(
        r"OLED_ASCII8x16\[1520\]\s*=\s*\{(.*?)\};", src, re.S
    ).group(1)
    cn = [int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", cn_block)]
    ascii_data = [int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", ascii_block)]
    assert len(cn) == 26 * 32, len(cn)
    assert len(ascii_data) == 1520, len(ascii_data)
    return cn, ascii_data


CN, ASCII = load_fonts()


class Screen:
    def __init__(self):
        self.gram = bytearray(W * PAGES)

    def clear(self):
        self.gram = bytearray(W * PAGES)

    def clear_pages(self, start, end):
        for p in range(start, end + 1):
            for x in range(W):
                self.gram[p * W + x] = 0

    def show_char_at(self, x, page, ch):
        if page >= PAGES - 1:
            return
        code = ord(ch)
        if code < 0x20 or code > 0x7E:
            code = 0x20
        off = (code - 0x20) * 16
        for i in range(8):
            px = x + i
            if 0 <= px < W:
                self.gram[page * W + px] = ASCII[off + i]
                self.gram[(page + 1) * W + px] = ASCII[off + 8 + i]

    def show_cn_at(self, x, page, index):
        if page >= PAGES - 1 or index >= 26:
            return
        off = index * 32
        for i in range(16):
            px = x + i
            if 0 <= px < W:
                self.gram[page * W + px] = CN[off + i]
                self.gram[(page + 1) * W + px] = CN[off + 16 + i]

    def show_str(self, x, page, text):
        for ch in text:
            if x > W - 8:
                break
            self.show_char_at(x, page, ch)
            x += 8

    def scroll_glyphs(self, glyphs, page, offset):
        self.clear_pages(page, page + 1)
        x = -offset
        for kind, value in glyphs:
            if kind == "a":
                self.show_char_at(x, page, value)
                x += 8
            else:
                self.show_cn_at(x, page, value)
                x += 16


# 字库索引
XUE, HAO, XING_LABEL, MING, WANG, XING = range(6)
WEN, DU, SHI, HUAN, YING, LAI, DAO, CHONG, QING, JIAO, TONG, DA, WU, LIAN, NET, SHI2, XUN, ROOM, COMMA, EXCLAM = range(
    6, 26
)

scroll_text = [
    ("a", "H"), ("a", "e"), ("a", "l"), ("a", "l"), ("a", "o"),
    ("c", COMMA),
    ("c", HUAN), ("c", YING), ("c", LAI), ("c", DAO),
    ("c", CHONG), ("c", QING), ("c", JIAO), ("c", TONG), ("c", DA), ("c", XUE),
    ("c", WU), ("c", LIAN), ("c", NET),
    ("a", "2"), ("a", "0"), ("a", "5"),
    ("c", SHI2), ("c", XUN), ("c", ROOM),
    ("c", EXCLAM),
]

hw_text = [
    ("a", "H"), ("a", "e"), ("a", "l"), ("a", "l"), ("a", "o"),
    ("c", COMMA), ("c", HUAN), ("c", YING), ("c", LAI), ("c", DAO),
]


def draw_scroll_frame(scr):
    scr.clear()
    for i, idx in enumerate([CHONG, QING, JIAO, TONG, DA, XUE]):
        scr.show_cn_at(16 + i * 16, 0, idx)
    scr.show_cn_at(4, 6, WU)
    scr.show_cn_at(20, 6, LIAN)
    scr.show_cn_at(36, 6, NET)
    scr.show_str(52, 6, "205")
    scr.show_cn_at(76, 6, SHI2)
    scr.show_cn_at(92, 6, XUN)
    scr.show_cn_at(108, 6, ROOM)


def main_screen():
    scr = Screen()
    scr.show_cn_at(0, 0, XUE)
    scr.show_cn_at(16, 0, HAO)
    scr.show_str(32, 0, "632407030627")
    scr.show_cn_at(10, 2, XING_LABEL)
    scr.show_cn_at(26, 2, MING)
    scr.show_cn_at(42, 2, WANG)
    scr.show_cn_at(58, 2, XING)
    scr.show_str(78, 2, "AHT20")
    scr.show_cn_at(20, 4, WEN)
    scr.show_cn_at(36, 4, DU)
    scr.show_str(52, 4, ":")
    scr.show_str(60, 4, "25.3")
    scr.show_str(100, 4, "C")
    scr.show_cn_at(20, 6, SHI)
    scr.show_cn_at(36, 6, DU)
    scr.show_str(52, 6, ":")
    scr.show_str(60, 6, "56.7")
    scr.show_str(100, 6, "%")
    return scr


def scroll_frame(offset):
    scr = Screen()
    draw_scroll_frame(scr)
    scr.scroll_glyphs(scroll_text, 2, offset)
    return scr


def hw_frame():
    scr = Screen()
    draw_scroll_frame(scr)
    scr.scroll_glyphs(hw_text, 2, 0)
    return scr


def to_image(scr):
    scale = 5
    img = Image.new("RGB", (W * scale, H * scale), (8, 10, 20))
    px = img.load()
    on = (90, 230, 255)
    for y in range(H):
        for x in range(W):
            if scr.gram[(y // 8) * W + x] & (1 << (y % 8)):
                for dy in range(scale):
                    for dx in range(scale):
                        px[x * scale + dx, y * scale + dy] = on
    return img


def main():
    panels = [
        ("1. ID / name + AHT20", main_screen()),
        ("2. software scroll", scroll_frame(100)),
        ("3. hardware scroll", hw_frame()),
    ]
    scale = 5
    gap = 16
    label_h = 24
    total_w = (W * scale) * len(panels) + gap * (len(panels) - 1)
    total_h = H * scale + label_h
    out = Image.new("RGB", (total_w, total_h), (24, 26, 34))
    draw = ImageDraw.Draw(out)
    for i, (label, scr) in enumerate(panels):
        x = i * (W * scale + gap)
        out.paste(to_image(scr), (x, label_h))
        draw.text((x + 2, 5), label, fill=(200, 220, 255))
    OUT.parent.mkdir(parents=True, exist_ok=True)
    out.save(OUT)
    print(f"saved {OUT}")


if __name__ == "__main__":
    main()
