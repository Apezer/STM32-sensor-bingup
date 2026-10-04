#include "stm32f10x.h"
#include "Delay.h"
#include "BSP_I2C.h"
#include "BMP280.h"
#include <math.h>

typedef struct
{
	uint16_t T1;
	int16_t T2;
	int16_t T3;
	uint16_t P1;
	int16_t P2;
	int16_t P3;
	int16_t P4;
	int16_t P5;
	int16_t P6;
	int16_t P7;
	int16_t P8;
	int16_t P9;
} BMP280_CalibrationTypeDef;

static const BSP_I2C_TypeDef *BMP280_I2C;
static BMP280_CalibrationTypeDef BMP280_Calibration;
static int32_t BMP280_TFine;

/***********************************************************
 * @brief     向BMP280寄存器写入一个字节
 * @param     RegAddress BMP280的8位寄存器地址
 * @param     Data       要写入的一个字节数据
 * @return    无
 * @example   BMP280_WriteReg(BMP280_REG_RESET, BMP280_RESET_VALUE);
 * @note      使用本函数前必须先调用BMP280_Init
 ****************************************************************/
void BMP280_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	BSP_I2C_WriteReg(BMP280_I2C, BMP280_ADDRESS_7BIT, RegAddress, Data);
}

/***********************************************************
 * @brief     从BMP280寄存器读取一个字节
 * @param     RegAddress BMP280的8位寄存器地址
 * @return    uint8_t 读取到的寄存器值
 * @example   DeviceID = BMP280_ReadReg(BMP280_REG_CHIP_ID);
 * @note      使用本函数前必须先调用BMP280_Init
 ****************************************************************/
uint8_t BMP280_ReadReg(uint8_t RegAddress)
{
	return BSP_I2C_ReadReg(BMP280_I2C, BMP280_ADDRESS_7BIT, RegAddress);
}

/***********************************************************
 * @brief     从BMP280连续读取多个寄存器
 * @param     RegAddress 起始寄存器地址
 * @param     Data       保存读取数据的缓冲区
 * @param     Length     连续读取的字节数
 * @return    无
 * @example   BMP280_ReadRegs(BMP280_REG_PRESS_MSB, Buffer, 6);
 * @note      BMP280读取期间会自动递增寄存器地址
 ****************************************************************/
static void BMP280_ReadRegs(uint8_t RegAddress, uint8_t *Data, uint8_t Length)
{
	BSP_I2C_ReadRegs(BMP280_I2C, BMP280_ADDRESS_7BIT,
		RegAddress, Data, Length);
}

/***********************************************************
 * @brief     将两个小端字节组合成无符号16位数
 * @param     Data 指向低字节在前的两个字节
 * @return    uint16_t 组合后的无符号16位数
 * @example   Value = BMP280_BytesToU16(Buffer);
 * @note      BMP280的出厂补偿参数采用小端顺序存放
 ****************************************************************/
static uint16_t BMP280_BytesToU16(const uint8_t *Data)
{
	return (uint16_t)(((uint16_t)Data[1] << 8) | Data[0]);
}

/***********************************************************
 * @brief     将两个小端字节组合成有符号16位数
 * @param     Data 指向低字节在前的两个字节
 * @return    int16_t 组合后的有符号16位数
 * @example   Value = BMP280_BytesToS16(Buffer);
 * @note      先按无符号数拼接，再转换为二进制补码有符号数
 ****************************************************************/
static int16_t BMP280_BytesToS16(const uint8_t *Data)
{
	return (int16_t)BMP280_BytesToU16(Data);
}

/***********************************************************
 * @brief     读取BMP280内部保存的出厂校准参数
 * @param     无
 * @return    无
 * @example   BMP280_ReadCalibration();
 * @note      24个字节对应dig_T1至dig_P9，不能使用其他芯片的参数
 ****************************************************************/
static void BMP280_ReadCalibration(void)
{
	uint8_t Buffer[24];

	BMP280_ReadRegs(BMP280_REG_DIG_T1, Buffer, 24);
	BMP280_Calibration.T1 = BMP280_BytesToU16(&Buffer[0]);
	BMP280_Calibration.T2 = BMP280_BytesToS16(&Buffer[2]);
	BMP280_Calibration.T3 = BMP280_BytesToS16(&Buffer[4]);
	BMP280_Calibration.P1 = BMP280_BytesToU16(&Buffer[6]);
	BMP280_Calibration.P2 = BMP280_BytesToS16(&Buffer[8]);
	BMP280_Calibration.P3 = BMP280_BytesToS16(&Buffer[10]);
	BMP280_Calibration.P4 = BMP280_BytesToS16(&Buffer[12]);
	BMP280_Calibration.P5 = BMP280_BytesToS16(&Buffer[14]);
	BMP280_Calibration.P6 = BMP280_BytesToS16(&Buffer[16]);
	BMP280_Calibration.P7 = BMP280_BytesToS16(&Buffer[18]);
	BMP280_Calibration.P8 = BMP280_BytesToS16(&Buffer[20]);
	BMP280_Calibration.P9 = BMP280_BytesToS16(&Buffer[22]);
}

/***********************************************************
 * @brief     使用出厂参数补偿原始温度
 * @param     RawTemperature 20位原始温度ADC值
 * @return    int32_t 补偿后的温度，单位0.01摄氏度
 * @example   Temperature = BMP280_CompensateTemperature(RawTemperature);
 * @note      计算得到的t_fine还会用于气压补偿，必须先计算温度
 ****************************************************************/
static int32_t BMP280_CompensateTemperature(int32_t RawTemperature)
{
	int32_t Var1;
	int32_t Var2;

	Var1 = ((((RawTemperature >> 3) - ((int32_t)BMP280_Calibration.T1 << 1))) *
		((int32_t)BMP280_Calibration.T2)) >> 11;
	Var2 = (((((RawTemperature >> 4) - ((int32_t)BMP280_Calibration.T1)) *
		((RawTemperature >> 4) - ((int32_t)BMP280_Calibration.T1))) >> 12) *
		((int32_t)BMP280_Calibration.T3)) >> 14;
	BMP280_TFine = Var1 + Var2;

	return (BMP280_TFine * 5 + 128) >> 8;
}

/***********************************************************
 * @brief     使用出厂参数补偿原始气压
 * @param     RawPressure 20位原始气压ADC值
 * @return    uint32_t 补偿后的气压，单位Pa
 * @example   Pressure = BMP280_CompensatePressure(RawPressure);
 * @note      使用数据手册中的64位整数算法，调用前必须先完成温度补偿
 ****************************************************************/
static uint32_t BMP280_CompensatePressure(int32_t RawPressure)
{
	int64_t Var1;
	int64_t Var2;
	int64_t Pressure;

	Var1 = ((int64_t)BMP280_TFine) - 128000;
	Var2 = Var1 * Var1 * (int64_t)BMP280_Calibration.P6;
	Var2 = Var2 + ((Var1 * (int64_t)BMP280_Calibration.P5) << 17);
	Var2 = Var2 + (((int64_t)BMP280_Calibration.P4) << 35);
	Var1 = ((Var1 * Var1 * (int64_t)BMP280_Calibration.P3) >> 8) +
		((Var1 * (int64_t)BMP280_Calibration.P2) << 12);
	Var1 = (((((int64_t)1) << 47) + Var1) *
		(int64_t)BMP280_Calibration.P1) >> 33;

	if (Var1 == 0)
	{
		return 0;
	}

	Pressure = 1048576 - RawPressure;
	Pressure = (((Pressure << 31) - Var2) * 3125) / Var1;
	Var1 = (((int64_t)BMP280_Calibration.P9) *
		(Pressure >> 13) * (Pressure >> 13)) >> 25;
	Var2 = (((int64_t)BMP280_Calibration.P8) * Pressure) >> 19;
	Pressure = ((Pressure + Var1 + Var2) >> 8) +
		(((int64_t)BMP280_Calibration.P7) << 4);

	return (uint32_t)((Pressure + 128) >> 8);
}

/***********************************************************
 * @brief     初始化软件I2C总线并配置BMP280连续测量
 * @param     I2C 指向BMP280所使用的软件I2C总线配置
 * @return    无
 * @example   BMP280_Init(&SensorI2C);
 * @note      配置为温度2倍、气压4倍过采样、IIR系数4和250ms待机时间
 ****************************************************************/
void BMP280_Init(const BSP_I2C_TypeDef *I2C)
{
	uint8_t Timeout;

	BMP280_I2C = I2C;
	BSP_I2C_Init(BMP280_I2C);
	Delay_ms(2);

	if (BMP280_GetID() != BMP280_CHIP_ID_VALUE)
	{
		return;
	}

	BMP280_WriteReg(BMP280_REG_RESET, BMP280_RESET_VALUE); /* 软复位芯片，重新加载出厂校准参数。 */
	for (Timeout = 0; Timeout < 100; Timeout++)
	{
		if ((BMP280_ReadReg(BMP280_REG_STATUS) & BMP280_STATUS_IM_UPDATE) == 0)
		{
			break;
		}
		Delay_ms(1);
	}
	BMP280_ReadCalibration();
	BMP280_WriteReg(BMP280_REG_CONFIG, 0x68);    /* 待机250ms，IIR滤波系数4，关闭三线SPI。 */
	BMP280_WriteReg(BMP280_REG_CTRL_MEAS, 0x4F); /* 温度2倍、气压4倍过采样，并进入正常模式。 */
	Delay_ms(20); /* 等待第一次温度和气压转换完成，避免读到复位后的无效数据。 */
}

/***********************************************************
 * @brief     读取BMP280芯片型号ID
 * @param     无
 * @return    uint8_t 芯片ID，BMP280正常值为0x58
 * @example   DeviceID = BMP280_GetID();
 * @note      本函数读取0xD0地址的chip_id寄存器
 ****************************************************************/
uint8_t BMP280_GetID(void)
{
	return BMP280_ReadReg(BMP280_REG_CHIP_ID);
}

/***********************************************************
 * @brief     根据气压和海平面参考气压计算海拔
 * @param     PressurePa         当前绝对气压，单位Pa
 * @param     SeaLevelPressurePa 当前地区海平面参考气压，单位Pa
 * @return    int32_t 估算海拔，单位cm
 * @example   Altitude = BMP280_PressureToAltitudeCm(Pressure, 101325);
 * @note      天气会改变气压；需要准确海拔时应使用当地气象数据校准参考气压
 ****************************************************************/
int32_t BMP280_PressureToAltitudeCm(uint32_t PressurePa,
	uint32_t SeaLevelPressurePa)
{
	double PressureRatio;
	double AltitudeCm;

	if ((PressurePa == 0) || (SeaLevelPressurePa == 0))
	{
		return 0;
	}

	PressureRatio = (double)PressurePa / (double)SeaLevelPressurePa;
	AltitudeCm = 4433000.0 * (1.0 - pow(PressureRatio, 0.190294957));

	return (int32_t)AltitudeCm;
}

/***********************************************************
 * @brief     连续读取并补偿BMP280温度、气压和海拔数据
 * @param     Data 指向保存补偿结果的数据结构
 * @return    无
 * @example   BMP280_GetData(&BMP280_Data);
 * @note      从0xF7开始一次读取6字节，避免气压和温度来自不同次测量
 ****************************************************************/
void BMP280_GetData(BMP280_DataTypeDef *Data)
{
	uint8_t Buffer[6];
	int32_t RawPressure;
	int32_t RawTemperature;

	BMP280_ReadRegs(BMP280_REG_PRESS_MSB, Buffer, 6);
	RawPressure = ((int32_t)Buffer[0] << 12) |
		((int32_t)Buffer[1] << 4) | ((int32_t)Buffer[2] >> 4);
	RawTemperature = ((int32_t)Buffer[3] << 12) |
		((int32_t)Buffer[4] << 4) | ((int32_t)Buffer[5] >> 4);

	Data->Temperature = BMP280_CompensateTemperature(RawTemperature);
	Data->Pressure = BMP280_CompensatePressure(RawPressure);
	Data->Altitude = BMP280_PressureToAltitudeCm(Data->Pressure,
		BMP280_SEA_LEVEL_PRESSURE_PA);
}
