#include "stm32f10x.h"
#include "Delay.h"
#include "BSP_I2C.h"
#include "VL6180X.h"

static const BSP_I2C_TypeDef *VL6180X_I2C;

/***********************************************************
 * @brief     向VL6180X寄存器写入一个字节
 * @param     RegAddress VL6180X的16位寄存器地址
 * @param     Data       要写入的一个字节数据
 * @return    无
 * @example   VL6180X_WriteReg(VL6180X_SYSRANGE_START, 0x01);
 * @note      使用本函数前必须先调用VL6180X_Init
 ****************************************************************/
void VL6180X_WriteReg(uint16_t RegAddress, uint8_t Data)
{
	BSP_I2C_WriteReg16Addr(VL6180X_I2C, VL6180X_ADDRESS_7BIT,
		RegAddress, Data);
}

/***********************************************************
 * @brief     从VL6180X寄存器读取一个字节
 * @param     RegAddress VL6180X的16位寄存器地址
 * @return    uint8_t 读取到的寄存器值
 * @example   DeviceID = VL6180X_ReadReg(VL6180X_IDENTIFICATION_MODEL_ID);
 * @note      使用本函数前必须先调用VL6180X_Init
 ****************************************************************/
uint8_t VL6180X_ReadReg(uint16_t RegAddress)
{
	return BSP_I2C_ReadReg16Addr(VL6180X_I2C, VL6180X_ADDRESS_7BIT,
		RegAddress);
}

/***********************************************************
 * @brief     加载ST推荐的SR03标准测距配置
 * @param     无
 * @return    无
 * @example   VL6180X_LoadRangeSettings();
 * @note      配置值来自ST应用笔记AN4545 Rev 2第9章
 ****************************************************************/
static void VL6180X_LoadRangeSettings(void)
{
	/* 以下30项是ST规定的SR03私有调校序列，芯片手册未公开其内部位功能。 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_01, 0x01); /* 加载私有调校值01，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_02, 0x01); /* 加载私有调校值02，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_03, 0x00); /* 加载私有调校值03，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_04, 0xFD); /* 加载私有调校值04，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_05, 0x01); /* 加载私有调校值05，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_06, 0x03); /* 加载私有调校值06，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_07, 0x02); /* 加载私有调校值07，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_08, 0x01); /* 加载私有调校值08，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_09, 0x03); /* 加载私有调校值09，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_10, 0x02); /* 加载私有调校值10，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_11, 0x05); /* 加载私有调校值11，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_12, 0xCE); /* 加载私有调校值12，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_13, 0x03); /* 加载私有调校值13，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_14, 0xF8); /* 加载私有调校值14，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_15, 0x00); /* 加载私有调校值15，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_16, 0x3C); /* 加载私有调校值16，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_17, 0x00); /* 加载私有调校值17，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_18, 0x3C); /* 加载私有调校值18，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_19, 0x09); /* 加载私有调校值19，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_20, 0x09); /* 加载私有调校值20，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_21, 0x01); /* 加载私有调校值21，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_22, 0x17); /* 加载私有调校值22，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_23, 0x00); /* 加载私有调校值23，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_24, 0x05); /* 加载私有调校值24，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_25, 0x05); /* 加载私有调校值25，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_26, 0x05); /* 加载私有调校值26，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_27, 0x1B); /* 加载私有调校值27，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_28, 0x3E); /* 加载私有调校值28，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_29, 0x1F); /* 加载私有调校值29，内部作用未公开 */
	VL6180X_WriteReg(VL6180X_PRIVATE_SR03_REG_30, 0x00); /* 加载私有调校值30，内部作用未公开 */

	VL6180X_WriteReg(VL6180X_SYSTEM_MODE_GPIO1, 0x10); 					/* 将GPIO1配置为低电平有效的中断输出 */
	VL6180X_WriteReg(VL6180X_READOUT_AVERAGING_SAMPLE_PERIOD, 0x30); 	/* 使用默认4.4ms读出平均周期，在测量噪声和执行时间之间折中 */
	VL6180X_WriteReg(VL6180X_SYSALS_ANALOGUE_GAIN, 0x46); 				/* 高4位保持为手册要求的0x4，并将ALS光通道增益设置为1倍 */
	VL6180X_WriteReg(VL6180X_SYSRANGE_VHV_REPEAT_RATE, 0xFF); 			/* 每进行255次测距后自动执行一次VHV系统校准 */
	VL6180X_WriteReg(VL6180X_SYSALS_INTEGRATION_PERIOD_LSB, 0x63); 		/* 将ALS积分时间设置为约100ms */
	VL6180X_WriteReg(VL6180X_SYSRANGE_VHV_RECALIBRATE, 0x01); 			/* 上电后执行一次测距温度校准 */
	VL6180X_WriteReg(VL6180X_SYSRANGE_INTERMEASUREMENT_PERIOD, 0x09); 	/* 连续测距模式的默认测量间隔为100ms */
	VL6180X_WriteReg(VL6180X_SYSALS_INTERMEASUREMENT_PERIOD, 0x31);		/* 连续ALS模式的默认测量间隔为500ms */
	VL6180X_WriteReg(VL6180X_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x24); 		/* 测距和ALS完成时均产生“新样本就绪”中断状态 */
}

/***********************************************************
 * @brief     初始化软件I2C总线并配置VL6180X
 * @param     I2C 指向VL6180X所使用的软件I2C总线配置
 * @return    无
 * @example   VL6180X_Init(&SensorI2C);
 * @note      传感器模块的GPIO0必须保持高电平才能进行I2C通信
 ****************************************************************/
void VL6180X_Init(const BSP_I2C_TypeDef *I2C)
{
	VL6180X_I2C = I2C;
	BSP_I2C_Init(VL6180X_I2C);
	Delay_ms(2); /* 内部MCU的最长启动时间为1ms，这里等待2ms留出余量 */

	if (VL6180X_ReadReg(VL6180X_SYSTEM_FRESH_OUT_OF_RESET) == 0x01) /* 仅在上电或GPIO0复位后重新加载配置 */
	{
		VL6180X_LoadRangeSettings();
		VL6180X_WriteReg(VL6180X_SYSTEM_FRESH_OUT_OF_RESET, 0x00); /* 写0表示本次上电后的配置已经加载完成 */
	}
}

/***********************************************************
 * @brief     读取VL6180X的型号识别寄存器
 * @param     无
 * @return    uint8_t 芯片型号ID，正常值为0xB4
 * @example   DeviceID = VL6180X_GetID();
 * @note      本函数读取地址0x0000处的IDENTIFICATION__MODEL_ID
 ****************************************************************/
uint8_t VL6180X_GetID(void)
{
	return VL6180X_ReadReg(VL6180X_IDENTIFICATION_MODEL_ID);
}

/***********************************************************
 * @brief     以阻塞方式完成一次距离测量
 * @param     无
 * @return    uint8_t 测得的距离，单位为毫米
 * @example   Distance = VL6180X_ReadDistance();
 * @note      数据手册保证的正常测量范围为0到100mm
 ****************************************************************/
uint8_t VL6180X_ReadDistance(void)
{
	uint8_t Distance;
	uint8_t InterruptStatus;

	/* 清除上一次测量可能遗留的测距、ALS和错误中断标志。 */
	VL6180X_WriteReg(VL6180X_SYSTEM_INTERRUPT_CLEAR, 0x07);
	/* 写入0x01，启动一次单次测距。 */
	VL6180X_WriteReg(VL6180X_SYSRANGE_START, 0x01);

	/* 轮询测距中断状态，等待“新样本就绪”；检测到系统错误时也退出等待。 */
	do
	{
		InterruptStatus = VL6180X_ReadReg(VL6180X_RESULT_INTERRUPT_STATUS_GPIO);
		Delay_ms(1);
	}
	while (((InterruptStatus & 0x07) != 0x04) && ((InterruptStatus & 0xC0) == 0x00));

	/* 读取最终距离结果，寄存器数值的单位就是毫米。 */
	Distance = VL6180X_ReadReg(VL6180X_RESULT_RANGE_VAL);
	/* 读取完成后清除所有中断标志，为下一次测量做准备。 */
	VL6180X_WriteReg(VL6180X_SYSTEM_INTERRUPT_CLEAR, 0x07);

	return Distance;
}
