任务 2：STM32CubeMX HAL 框架 + U8g2 + 0.96 寸 IIC OLED
==========================================================

一、实验内容
------------
使用 STM32CubeMX 设计 HAL 库程序框架，下载 U8g2 源码，针对
STM32F103C8T6 + SSD1306 128x64 IIC OLED 进行裁剪和移植，完成：

    1. U8g2 demo 例程；
    2. 显示学号 632407030627 和姓名 王兴；
    3. 上下、左右滑动显示；
    4. 显示动态图案（旋转雷达扫描）。

二、CubeMX 框架配置
-------------------
工程结构采用 CubeMX 生成的标准 HAL 框架：

    Core/Inc/main.h
    Core/Inc/stm32f1xx_hal_conf.h
    Core/Inc/stm32f1xx_it.h
    Core/Src/main.c
    Core/Src/stm32f1xx_it.c
    Core/Src/stm32f1xx_hal_msp.c
    Core/Src/system_stm32f1xx.c
    Drivers/CMSIS
    Drivers/STM32F1xx_HAL_Driver
    Startup/startup_stm32f103xb.s

CubeMX 配置文件：OLED_U8G2.ioc

主要配置：
    MCU           : STM32F103C8Tx，LQFP48
    RCC           : HSE 8 MHz，PLL x9，SYSCLK 72 MHz
    APB1          : 36 MHz，APB2 : 72 MHz
    SYS           : SysTick
    GPIO          : PB6 -> OLED_SCL，PB7 -> OLED_SDA（GPIO 输出）
                    PC13 -> LED
    Toolchain     : MDK-ARM

说明：OLED 驱动采用 GPIO 模拟 IIC，因此在 CubeMX 中把 PB6/PB7
配置为普通 GPIO 输出；SoftIIC_Init() 会在运行时把这两个引脚设置
为开漏输出（GPIO_MODE_OUTPUT_OD）。

如果 CubeMX 提示缺少 STM32Cube FW_F1 固件包，可以在 CubeMX 的
Help -> Manage embedded software packages 中安装 STM32Cube MCU
Package F1，然后重新打开 OLED_U8G2.ioc。

三、U8g2 裁剪与移植
-------------------
U8g2 版本：2.37.1
源码位置：Middlewares/U8G2
裁剪说明：Middlewares/U8G2/README.txt

1. 只保留 SSD1306 128x64 IIC 所需文件：

    u8g2_d_setup.c   只保留 u8g2_Setup_ssd1306_i2c_128x64_noname_f()
    u8g2_d_memory.c  只保留 128x64 全屏缓冲 1 KB
    u8g2_fonts_min.c 只保留 6x10、8x13、10x20 三个 ASCII 字体
    u8g2_buffer.c    删除 PBM/XBM 导出函数，避免引入 u8x8_capture.c

2. 移植回调（Core/Src/u8g2_port.c）：

    U8G2_ByteCallback()
        - U8X8_MSG_BYTE_START_TRANSFER：SoftIIC_Start()，发送 0x78；
        - U8X8_MSG_BYTE_SEND：逐字节发送；
        - U8X8_MSG_BYTE_END_TRANSFER：SoftIIC_Stop()；
        - U8X8_MSG_BYTE_INIT：SoftIIC_Init()。

    U8G2_GPIOAndDelayCallback()
        - U8X8_MSG_DELAY_MILLI：HAL_Delay()；
        - U8X8_MSG_DELAY_10MICRO：软件循环延时；
        - U8X8_MSG_DELAY_100NANO / NANO：NOP。

    U8G2_Port_Init()
        - u8g2_Setup_ssd1306_i2c_128x64_noname_f()
        - u8g2_InitDisplay()
        - u8g2_SetPowerSave(0)
        - u8g2_ClearBuffer() / u8g2_SendBuffer()

四、显示功能
------------
程序循环显示 5 个界面：

1. U8g2 demo
   - 10x20、6x10、8x13 三种字体；
   - 直线、矩形框、圆、实心圆、折线、填充矩形；
   - 显示 "U8g2 Demo"、"SSD1306"。

2. 学号 + 姓名
   - "ID:632407030627"；
   - "Name:" + 王兴（16x16 XBM，数组在 Core/Src/cn_font.c）；
   - 底部显示 "STM32F103 + U8g2"。
   - XBM 点阵按 U8g2 DrawXBMP 的要求使用 LSB first（低位在左），
     每行 2 字节，共 16 行 32 字节。

3. 水平滚动
   - 使用一个 640x64 的虚拟 U8g2 缓冲区；
   - 在虚拟缓冲区中把长字符串画在 x=128 处；
   - 每帧把虚拟缓冲区的一个 128x64 窗口按 1 像素步进复制到真实
     显存，再 u8g2_SendBuffer()；
   - 这样可以显示超过 128 像素的长字符串，并且动画连续。

4. 垂直滚动
   - 8 行文本在一个裁剪窗口（page2~page7）内向上移动；
   - 每帧偏移 2 像素；
   - 使用 u8g2_SetClipWindow() 限制显示区域。

5. 动态雷达图案
   - 三层同心圆 + 十字线；
   - 32 方向 sin/cos 查表，扫描线每 40 ms 旋转一格；
   - 4 个目标点在扫描线经过时点亮。

五、编译与下载
--------------
Keil 工程：

    Project/MDK-ARM/OLED_U8G2.uvprojx

编译结果（MDK 5.36 / ARMCC V5.06 update 7）：

    Program Size: Code=12712 RO-data=8424 RW-data=56 ZI-data=8096
    0 Error(s), 0 Warning(s)

下载文件：

    Output/OLED_U8G2.hex

接线：

    OLED VCC -> 3.3V
    OLED GND -> GND
    OLED SCL -> PB6
    OLED SDA -> PB7

六、Keil 虚拟逻辑分析仪采集 I2C SDA 波形
----------------------------------------
1. 如何添加信号

   进入 Debug 模式，打开：
       View -> Analysis Windows -> Logic Analyzer
   点击 Setup，添加两个信号（Display Type 选 Bit）：

       Signal: GPIOB->IDR    Bit: 7    /* SDA，PB7 */
       Signal: GPIOB->IDR    Bit: 6    /* SCL，PB6 */

   也可以添加 GPIOB->ODR 观察软件驱动的输出电平。要观察从机 ACK，
   应使用 IDR（输入数据寄存器），因为 ACK 是 OLED 把 SDA 拉低。
   如果只添加一个信号，建议至少同时添加 SCL 和 SDA，否则无法判断
   数据位的采样时刻。

2. SDA/SCL 波形是什么样子

   标准 IIC 一帧（写 OLED）如下：

       SCL : ‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾|__|‾‾
       SDA : ‾‾\___________________________________________________/‾‾
               START   D7 D6 D5 D4 D3 D2 D1 D0  ACK        STOP

   具体特征：
   - 空闲：SCL=1，SDA=1；
   - 起始位：SCL 为高时，SDA 由 1 变 0；
   - 数据位：SCL 低时改变 SDA，SCL 高时数据有效，高位在前；
   - 第 9 个时钟：主机释放 SDA，从机拉低表示 ACK；
   - 停止位：SCL 为高时，SDA 由 0 变 1；
   - OLED 地址 0x78 的二进制是 0111 1000；
   - 控制字节 0x00 表示命令，0x40 表示数据；
   - 一次 OLED 刷新会看到很多个 9 时钟的字节。

   如果只采集 SDA、不采集 SCL，可以看到起始/停止和 0/1 电平变化，
   但无法确定每个数据位的采样时刻，必须同时采集 SCL 才能解码。

   示意图：Docs/i2c_sda_waveform.png
   （地址 0x78 = 0111 1000，第 9 个时钟为 ACK，最后为 STOP）

3. 为什么 Keil 虚拟逻辑分析仪可能采集不到真实 I2C

   - Keil 的 Logic Analyzer 是调试/仿真工具，不是高速硬件逻辑分析仪；
     它通过调试接口周期采样变量，采样率和时间精度有限，400 kHz 的
     IIC 边沿可能被漏掉；
   - 在硬件在线调试时，它采集的是变量/寄存器值，不是引脚上的真实
     模拟波形，开漏上拉、上升沿、毛刺等看不到；
   - 如果使用硬件 I2C 外设，PB6/PB7 处于复用开漏模式，仿真器不一定
     能正确模拟 I2C 外设的引脚输出；
   - Keil 的软件仿真通常没有 OLED 从机模型，因此 ACK 可能永远是高
     电平，无法看到真实的从机应答；
   - 如果代码运行在 72 MHz，软件 IIC 的延时很短，Logic Analyzer 的
     采样窗口可能不足以覆盖完整的一帧。

4. 推荐的替代方法

   （1）外接逻辑分析仪（最推荐）
        Saleae Logic、DSLogic 或 24 MHz 8 通道廉价逻辑分析仪，
        同时接 SCL/SDA/GND，使用 I2C 协议解码器，可以直接看到：
            地址、读写位、ACK/NACK、寄存器/数据字节、START/STOP。

   （2）示波器 + I2C 解码
        双通道示波器分别接 SCL/SDA，触发 SDA 下降沿，使用示波器的
        I2C 解码功能；没有解码功能时按上面的时序人工判读。

   （3）串口/SWO 软件日志
        在 U8G2_ByteCallback() 中记录 START/ADDR/DATA/ACK/STOP，
        通过 UART 或 SWO/ITM 输出到 PC。这样验证的是“软件协议层”，
        不能替代物理波形，但非常适合排查地址、ACK、数据错误。

   （4）GPIO 镜像 + Keil Logic Analyzer
        如果没有外部逻辑分析仪，可以把 SCL/SDA 的电平同时复制到
        两个空闲 GPIO，降低 IIC 速度（例如 10 kHz），再用 Keil 的
        Logic Analyzer 采集镜像引脚。这样能看到协议时序，但仍然是
        软件镜像，不是总线上的真实波形。

   （5）Proteus / 虚拟仿真
        在 Proteus 中搭建 STM32 + SSD1306，使用虚拟 I2C 调试器或
        虚拟示波器观察总线；适合没有硬件时验证协议。

   （6）第二块 MCU 做 I2C Sniffer
        用另一块 STM32/Arduino 只监听 SCL/SDA，在 SCL 上升沿采样
        SDA，按字节状态机解码，再通过串口输出。成本低，适合课堂。

七、文件说明
------------
    OLED_U8G2.ioc                 CubeMX 配置文件
    Core/Src/main.c               主程序、时钟、GPIO
    Core/Src/u8g2_port.c          U8g2 到 HAL/软件 IIC 的移植
    Core/Src/u8g2_app.c           demo、学号姓名、滚动、雷达
    Core/Src/soft_iic.c           软件 IIC 底层
    Core/Src/cn_font.c            王、兴 16x16 XBM 字模
    Middlewares/U8G2              U8g2 裁剪源码
    Project/MDK-ARM/OLED_U8G2.uvprojx
    Output/OLED_U8G2.hex

八、注意事项
------------
- U8g2 源码和本工程源文件使用 UTF-8 编码；若 Keil 中文注释乱码，
  在 Edit -> Configuration -> Editor 中把 Encoding 改为 UTF-8。
- 如果 OLED 不亮，检查 3.3V、GND、PB6/PB7 是否接反、地址是否为 0x78。
- 如果只显示一半或花屏，检查 OLED 控制器是否为 SSD1306；SH1106 需要
  使用对应的 U8g2 setup 函数。
- 水平滚动使用 640x64 虚拟缓冲，额外占用约 5 KB RAM；STM32F103C8T6
  的 20 KB RAM 足够。
