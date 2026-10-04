# STM32 Sensor Bring-up

基于 STM32F103C8T6 和 STM32 标准外设库的传感器学习工程，主要用于练习软件 I²C、传感器寄存器读写、数据换算以及 OLED 显示。

当前 `main.c` 演示 BMP280 温度、气压和海拔测量，并通过 0.96 英寸 SSD1306 OLED 显示补偿后的数据。工程中同时保留了 MPU6050 和 VL6180X 驱动，方便后续继续学习和切换传感器。

## 当前功能

- GPIO 模拟 I²C，支持通过总线句柄配置不同 GPIO 引脚。
- 支持 8 位寄存器地址的单字节读写和连续批量读取。
- 支持 16 位寄存器地址的单字节读写，可复用于 VL6180X 等器件。
- SSD1306 OLED 显存缓冲与整屏刷新。
- MPU6050 原始加速度、角速度和温度读取。
- 使用加速度原始数据计算 Roll、Pitch。
- VL6180X 型号识别、SR03 初始化和单次距离测量。
- BMP280 芯片识别、出厂校准参数读取和整数补偿算法。
- OLED 显示 BMP280 温度、气压和估算海拔。

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

### BMP280

| BMP280 模块引脚 | STM32 引脚 | 说明 |
|---|---|---|
| SCL | PB10 | 软件 I²C 时钟线 |
| SDA | PB11 | 软件 I²C 数据线 |
| VCC | 3.3V | 请以实际模块的供电标注为准 |
| GND | GND | 公共地 |
| SDO | GND | 将7位I²C地址选择为 `0x76` |
| CSB/CS | 高电平 | 使用I²C模式时应保持高电平，部分模块已经上拉 |

BMP280 的 `chip_id` 正常读取值为 `0x58`。当前模块的 SDO 接地，因此使用7位I²C地址 `0x76`。

> 使用前请确认所购买模块是否带稳压和电平转换，不要仅根据网上其他模块的接线直接选择 5V 供电。SCL、SDA 必须有合适的上拉电阻，并确保上拉电压不超过 STM32 和传感器模块允许的 I/O 电压。

## 编译与运行

1. 使用 Keil 打开 `Project/Project.uvprojx`。
2. 确认目标器件为 `STM32F103C8`。
3. 编译工程并解决本机缺少的 Device Pack 提示。
4. 连接 ST-Link，将程序下载到开发板。
5. 复位后观察 OLED：
   - 正常时显示温度、气压和基于标准海平面气压估算的海拔。
   - 如果设备 ID 不是 `0x58`，OLED 会显示错误信息和当前读取到的 ID。

默认海平面参考气压为 `101325 Pa`。气象变化会影响绝对海拔，需要准确海拔时应将 `BMP280_SEA_LEVEL_PRESSURE_PA` 改为当地当前海平面气压。

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
- `Project/User/main.c`：当前 BMP280 + OLED 示例。

## 参考资料

- [VL6180X 数据手册](Docs/VL6180X/Datasheet/VL6180X-Datasheet.pdf)
- [BMP280 数据手册](Docs/BMP280/Datasheet/BST-BMP280-DS001.pdf)
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
