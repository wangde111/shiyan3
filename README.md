# STM32F103 OLED 实验

本仓库包含两个基于 STM32F103C8T6 + 0.96 寸 SSD1306 IIC OLED 的实验工程。

完整实验报告：[实验报告.md](实验报告.md)

## 工程一：OLED_IIC（任务 1）

- 软件 IIC 驱动 SSD1306 128×64 OLED
- 显示学号 `632407030627` 和姓名 `王兴`
- 共用 IIC 总线读取 AHT20 温湿度
- 软件滚动完整长字符串 + SSD1306 硬件滚动演示
- Keil 工程：`OLED_IIC/Project/MDK-ARM/OLED_IIC.uvprojx`
- 说明：`OLED_IIC/readme.txt`
- 预览：`OLED_IIC/Docs/screens_preview.png`

## 工程二：OLED_U8G2（任务 2）

- CubeMX 风格 HAL 框架 + U8g2 2.37.1 裁剪移植
- U8g2 demo、学号姓名、水平/垂直滚动、动态雷达图案
- Keil 工程：`OLED_U8G2/Project/MDK-ARM/OLED_U8G2.uvprojx`
- CubeMX 配置：`OLED_U8G2/OLED_U8G2.ioc`
- 说明：`OLED_U8G2/readme.txt`
- I2C 波形示意：`OLED_U8G2/Docs/i2c_sda_waveform.png`

## 硬件连接

| OLED / AHT20 | STM32F103 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SCL | PB6 |
| SDA | PB7 |

OLED 地址 `0x78`，AHT20 地址 `0x38`（8 位写地址 `0x70`）。

## 编译环境

- Keil MDK 5.36
- ARMCC V5.06 update 7
- STM32F103C8T6，64 KB Flash / 20 KB RAM

两个工程均已编译通过，0 Error(s), 0 Warning(s)。

## 打包文件

`dist/` 目录下提供完整工程 ZIP：

- `shiyan3_all_projects.zip`：两个工程 + tools + 说明
- `OLED_U8G2_task2.zip`：任务 2 完整工程
