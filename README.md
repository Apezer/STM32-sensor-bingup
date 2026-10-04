# STM32 Sensor Bring-up

基于 STM32F103C8T6 和 STM32 标准外设库的传感器学习工程，主要用于练习软件 I²C、传感器寄存器读写、数据换算以及 OLED 显示。

当前 `main.c` 演示 VL6180X 单次距离测量，并通过 0.96 英寸 SSD1306 OLED 显示距离、设备 ID 和 I²C 地址。工程中同时保留了 MPU6050 驱动，方便后续继续学习和切换传感器。

## 当前功能

- GPIO 模拟 I²C，支持通过总线句柄配置不同 GPIO 引脚。
- 支持 8 位寄存器地址的通用读取。
- 支持 16 位寄存器地址的单字节读写，可复用于 VL6180X 等器件。
- SSD1306 OLED 显存缓冲与整屏刷新。
- MPU6050 原始加速度、角速度和温度读取。
- 使用加速度原始数据计算 Roll、Pitch。
- VL6180X 型号识别、SR03 初始化和单次距离测量。
- OLED 显示 VL6180X 距离，单位为毫米。

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

### VL6180X

| VL6180X 引脚 | STM32 引脚 | 说明 |
|---|---|---|
| SCL | PB10 | 软件 I²C 时钟线 |
| SDA | PB11 | 软件 I²C 数据线 |
| VCC | 按模块要求连接 | 芯片本体与转接板的供电范围可能不同 |
| GND | GND | 公共地 |

VL6180X 默认 7 位 I²C 地址为 `0x29`，`IDENTIFICATION__MODEL_ID` 正常读取值为 `0xB4`。如果模块引出了 `GPIO0/XSHUT`，需要保持高电平才能进行 I²C 通信。

> 使用前请确认所购买模块是否带稳压和电平转换，不要仅根据网上其他模块的接线直接选择 5V 供电。SCL、SDA 必须有合适的上拉电阻，并确保上拉电压不超过 STM32 和传感器模块允许的 I/O 电压。

## 编译与运行

1. 使用 Keil 打开 `Project/Project.uvprojx`。
2. 确认目标器件为 `STM32F103C8`。
3. 编译工程并解决本机缺少的 Device Pack 提示。
4. 连接 ST-Link，将程序下载到开发板。
5. 复位后观察 OLED：
   - 正常时显示 VL6180X 距离，单位为毫米。
   - 如果设备 ID 不是 `0xB4`，OLED 会显示错误信息和当前读取到的 ID。

VL6180X 数据手册保证的典型测距范围为 `0~100 mm`。在目标反射率和环境光条件合适时可能测得更远距离，但不属于保证范围。

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
- `Project/User/main.c`：当前 VL6180X + OLED 示例。

## 参考资料

- [VL6180X 数据手册](Docs/VL6180X/Datasheet/VL6180X-Datasheet.pdf)
- [MPU6050 产品规格书](Docs/MPU6050/Datasheet/MPU-6000-MPU-6050-Product-Specification-Rev3.4.pdf)
- [MPU6050 寄存器手册](Docs/MPU6050/Datasheet/MPU-6000-MPU-6050-Register-Map-Rev4.2.pdf)
- [SSD1306 数据手册](Docs/SSD1306-OLED/Datasheet/驱动芯片SSD1306数据手册.pdf)
- [ST AN4545：VL6180X Basic Ranging Application Note](https://www.st.com/resource/en/application_note/an4545-vl6180x-basic-ranging-application-note-stmicroelectronics.pdf)

## 后续计划

- 为 BSP I²C 增加超时和错误状态。
- 增加 VL6180X 测距状态和错误码解析。
- 增加 VL6180X 环境光测量。
- 演示多个传感器共享同一条 I²C 总线。
- 增加测距偏移和串扰校准示例。
