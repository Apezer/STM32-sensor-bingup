# STM32 Sensor Bring-up

基于 STM32F103C8T6 和 STM32 标准外设库的传感器与外设学习工程，主要用于练习软件 I²C、软件 SPI、芯片寄存器/指令读写、数据换算以及 OLED 显示。

当前 `main.c` 演示 W25Q64FV SPI Flash 的识别、扇区擦除、数据写入和读回校验，并通过 0.96 英寸 SSD1306 OLED 显示测试结果。工程中同时保留了 MPU6050、VL6180X、BMP280 和 AS5600 驱动，方便后续继续学习和切换器件。

## 当前功能

- GPIO 模拟 I²C，支持通过总线句柄配置不同 GPIO 引脚。
- GPIO 模拟 SPI，使用模式0、MSB先行，并支持总线与不同片选设备复用。
- 支持 8 位寄存器地址的单字节读写和连续批量读取。
- 支持 16 位寄存器地址的单字节读写，可复用于 VL6180X 等器件。
- SSD1306 OLED 显存缓冲与整屏刷新。
- MPU6050 原始加速度、角速度和温度读取。
- 使用加速度原始数据计算 Roll、Pitch。
- VL6180X 型号识别、SR03 初始化和单次距离测量。
- BMP280 芯片识别、出厂校准参数读取和整数补偿算法。
- AS5600 12位角度、磁铁状态、AGC和磁场幅值读取。
- OLED 显示 AS5600 单圈绝对角度和磁铁安装状态。
- W25Q64FV JEDEC ID和制造商/器件ID读取。
- W25Q64FV连续读取、自动跨页写入、4KB扇区/32KB块/64KB块/整片擦除和掉电控制。

## 开发环境

- MCU：STM32F103C8T6
- IDE：Keil MDK-ARM
- 固件库：STM32F10x Standard Peripheral Library 3.5.0
- 编程语言：C
- 下载器：ST-Link 或其他支持 STM32F103 的下载器

工程已经包含启动文件、CMSIS 文件和标准外设库源码，不依赖 STM32CubeMX 或 HAL。

## 当前硬件接线

### SSD1306 OLED

| OLED 引脚 | STM32 引脚 | 说明 |
|---|---|---|
| SCL | PB8 | 软件 I²C 时钟线 |
| SDA | PB9 | 软件 I²C 数据线 |
| VCC | 按模块要求连接 | 常见模块通常支持 3.3V |
| GND | GND | 公共地 |

### AS5600

| AS5600 模块引脚 | STM32 引脚 | 说明 |
|---|---|---|
| SCL | PB10 | 软件 I²C 时钟线 |
| SDA | PB11 | 软件 I²C 数据线 |
| VCC | 3.3V | 请以实际模块的供电标注为准 |
| GND | GND | 公共地 |
| DIR | GND或3.3V | 选择角度值递增的旋转方向，模块未引出时使用板载默认配置 |
| OUT | 不连接 | 当前示例使用I²C，不读取模拟或PWM输出 |

AS5600 使用固定的7位I²C地址 `0x36`。磁铁必须是径向充磁磁铁，并尽量与芯片中心和旋转轴同轴安装。

### W25Q64FV

| W25Q64模块引脚 | STM32引脚 | 说明 |
|---|---|---|
| CS | PB0 | 低电平片选 |
| DO | PB1 | MISO，从Flash到STM32 |
| CLK | PB10 | 软件SPI时钟SCK |
| DI | PB11 | MOSI，从STM32到Flash |
| VCC | 3.3V | 不要将芯片I/O接到5V电平 |
| GND | GND | 公共地 |

软件SPI采用模式0：时钟空闲为低电平，在上升沿采样数据。W25Q64FV容量为64Mbit，即8MB。

> 使用前请确认所购买模块是否带稳压和电平转换，不要仅根据网上其他模块的接线直接选择 5V 供电。SCL、SDA 必须有合适的上拉电阻，并确保上拉电压不超过 STM32 和传感器模块允许的 I/O 电压。

## 编译与运行

1. 使用 Keil 打开 `Project/Project.uvprojx`。
2. 确认目标器件为 `STM32F103C8`。
3. 编译工程并解决本机缺少的 Device Pack 提示。
4. 连接 ST-Link，将程序下载到开发板。
5. 复位后观察 OLED：正常完成时显示测试地址 `7FF000`、`ERASE:OK` 和 `WRITE:PASS`。

当前主程序会在每次复位时擦除W25Q64FV最后一个4KB扇区（`0x7FF000~0x7FFFFF`），并在起始位置写入16字节测试数据。请将该扇区保留为测试区，不要存放其他数据。程序只有在JEDEC ID等于`EF4017`时才会执行擦写。

## 软件 I²C 复用

软件 I²C 总线通过 `BSP_I2C_TypeDef` 描述：

```c
static const BSP_I2C_TypeDef SensorI2C =
{
    GPIOB, GPIO_Pin_10,
    GPIOB, GPIO_Pin_11,
    RCC_APB2Periph_GPIOB
};
```

不同传感器驱动可以保存同一个总线句柄。只要 I²C 地址不冲突，多个传感器也可以连接到同一组 SCL、SDA。

BMP280 使用8位寄存器地址。BSP 中的 `BSP_I2C_ReadRegs` 可以在一次I²C事务中连续读取多个寄存器，驱动用它读取24字节出厂校准参数和6字节测量结果。

## AS5600 测量流程

1. 初始化 PB10/PB11 软件 I²C，总线使用固定7位地址 `0x36`。
2. 读取 `STATUS` 磁铁状态，并分别连续读取 `RAW_ANGLE` 和 `ANGLE` 的高低字节。
3. 从 `AGC` 开始连续读取自动增益值和磁场幅值。角度寄存器地址指针会在各自的高低字节间循环，因此两组角度不能合并成一次跨寄存器读取。
4. 使用 `Angle × 36000 ÷ 4096` 将12位计数换算成百分之一度。
5. 根据 `MD`、`ML`和`MH`状态位判断磁铁未检测、过弱、正常或过强。
6. 将角度和诊断信息写入OLED显存并刷新显示。

## 软件 SPI 与 W25Q64FV

软件SPI总线通过 `BSP_SPI_TypeDef` 描述SCK、MISO和MOSI，W25Q64设备句柄另外保存CS。这样多个SPI设备可以共用三根总线信号线，并分别使用独立片选。

```c
static const BSP_SPI_TypeDef Flash_SPI =
{
    GPIOB, GPIO_Pin_10,
    GPIOB, GPIO_Pin_1,
    GPIOB, GPIO_Pin_11,
    RCC_APB2Periph_GPIOB
};
```

`W25Q64_WriteData` 会自动按256字节页边界拆分写入，但不会自动擦除。Flash编程只能把位从1写成0，因此覆盖已有数据前应先调用相应的扇区或块擦除函数。

## BMP280 测量流程

1. 初始化 PB10/PB11 软件 I²C，并读取 `0xD0` 芯片ID。
2. 写入 `0xB6` 执行软复位，等待出厂校准参数装载完成。
3. 从 `0x88` 开始连续读取 `dig_T1` 至 `dig_P9` 共24字节校准参数。
4. 配置温度2倍过采样、气压4倍过采样、IIR滤波系数4和正常测量模式。
5. 从 `0xF7` 开始连续读取气压和温度的6字节原始数据。
6. 使用数据手册给出的整数补偿算法计算摄氏温度和Pa气压。
7. 使用海平面参考气压将当前气压换算为估算海拔。

VL6180X 使用 16 位寄存器地址，因此 BSP 增加了以下接口：

```c
void BSP_I2C_WriteReg16Addr(const BSP_I2C_TypeDef *I2C,
    uint8_t DeviceAddress, uint16_t RegAddress, uint8_t Data);

uint8_t BSP_I2C_ReadReg16Addr(const BSP_I2C_TypeDef *I2C,
    uint8_t DeviceAddress, uint16_t RegAddress);
```

这里的“16 位”指寄存器地址宽度。当前接口每次读写的寄存器数据仍然是一个字节，寄存器地址按照高字节在前、低字节在后的顺序发送。

## VL6180X 测距流程

1. 初始化软件 I²C。
2. 等待 VL6180X 内部 MCU 完成启动。
3. 检查 `SYSTEM__FRESH_OUT_OF_RESET`。
4. 加载 ST AN4545 推荐的 SR03 初始化配置。
5. 向 `SYSRANGE__START` 写入 `0x01`，启动单次测距。
6. 轮询 `RESULT__INTERRUPT_STATUS_GPIO`，等待新数据就绪。
7. 从 `RESULT__RANGE_VAL` 读取距离值。
8. 清除中断状态，为下一次测量做准备。

SR03 中包含一组 ST 未公开内部位定义的私有调校寄存器。工程使用 `VL6180X_PRIVATE_SR03_REG_xx` 对其编号，并严格采用 AN4545 Rev 2 给出的地址和值，没有为未公开功能编造名称。

## 工程目录

```text
Sensor-bingup-STM32/
├─ Docs/                       芯片数据手册
│  ├─ MPU6050/
│  ├─ BMP280/
│  ├─ AS5600/
│  ├─ W25Q64/
│  ├─ SSD1306-OLED/
│  └─ VL6180X/
├─ Project/
│  ├─ Hardware/                BSP、OLED 和传感器驱动
│  ├─ Library/                 STM32 标准外设库
│  ├─ Start/                   CMSIS 和启动文件
│  ├─ System/                  延时等系统功能
│  ├─ User/                    main.c 和中断文件
│  └─ Project.uvprojx          Keil 工程文件
└─ README.md
```

## 主要文件

- `Project/Hardware/BSP_I2C.c`：通用 GPIO 软件 I²C。
- `Project/Hardware/OLED.c`：SSD1306 显示驱动和显存刷新。
- `Project/Hardware/MPU6050.c`：MPU6050 数据读取及姿态角计算。
- `Project/Hardware/VL6180X.c`：VL6180X 初始化和距离测量。
- `Project/Hardware/BMP280.c`：BMP280 校准、温度气压补偿和海拔换算。
- `Project/Hardware/AS5600.c`：AS5600 角度和磁场诊断信息读取。
- `Project/Hardware/BSP_SPI.c`：通用 GPIO 软件 SPI 模式0实现。
- `Project/Hardware/W25Q64.c`：W25Q64FV识别、读写、擦除和掉电控制。
- `Project/User/main.c`：当前 W25Q64FV + OLED 示例。

## 参考资料

- [VL6180X 数据手册](Docs/VL6180X/Datasheet/VL6180X-Datasheet.pdf)
- [BMP280 数据手册](Docs/BMP280/Datasheet/BST-BMP280-DS001.pdf)
- [AS5600 数据手册](Docs/AS5600/Datasheet/AS5600-DS000365.pdf)
- [W25Q64FV 数据手册](Docs/W25Q64/Datasheet/W25Q64FV-RevS.pdf)
- [MPU6050 产品规格书](Docs/MPU6050/Datasheet/MPU-6000-MPU-6050-Product-Specification-Rev3.4.pdf)
- [MPU6050 寄存器手册](Docs/MPU6050/Datasheet/MPU-6000-MPU-6050-Register-Map-Rev4.2.pdf)
- [SSD1306 数据手册](Docs/SSD1306-OLED/Datasheet/驱动芯片SSD1306数据手册.pdf)
- [ST AN4545：VL6180X Basic Ranging Application Note](https://www.st.com/resource/en/application_note/an4545-vl6180x-basic-ranging-application-note-stmicroelectronics.pdf)

## 后续计划

- 为 BSP I²C 增加超时和错误状态。
- 增加 VL6180X 测距状态和错误码解析。
- 演示多个传感器共享同一条 I²C 总线。
- 增加测距偏移和串扰校准示例。
- 增加按键校准BMP280海平面参考气压的示例。
- 增加AS5600软件零点和多圈角度累计示例。
