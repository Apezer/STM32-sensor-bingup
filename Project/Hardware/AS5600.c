#include "stm32f10x.h"
#include "Delay.h"
#include "BSP_I2C.h"
#include "AS5600.h"

static const BSP_I2C_TypeDef *AS5600_I2C;

/***********************************************************
 * @brief     向AS5600寄存器写入一个字节
 * @param     RegAddress AS5600的8位寄存器地址
 * @param     Data       要写入的一个字节数据
 * @return    无
 * @example   AS5600_WriteReg(AS5600_REG_CONF_L, 0x00);
 * @note      本工程不会写入BURN寄存器，避免不可逆地烧写OTP
 ****************************************************************/
void AS5600_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	BSP_I2C_WriteReg(AS5600_I2C, AS5600_ADDRESS_7BIT, RegAddress, Data);
}

/***********************************************************
 * @brief     从AS5600寄存器读取一个字节
 * @param     RegAddress AS5600的8位寄存器地址
 * @return    uint8_t 读取到的寄存器值
 * @example   Status = AS5600_ReadReg(AS5600_REG_STATUS);
 * @note      使用本函数前必须先调用AS5600_Init
 ****************************************************************/
uint8_t AS5600_ReadReg(uint8_t RegAddress)
{
	return BSP_I2C_ReadReg(AS5600_I2C, AS5600_ADDRESS_7BIT, RegAddress);
}

/***********************************************************
 * @brief     从AS5600连续读取多个寄存器
 * @param     RegAddress 起始寄存器地址
 * @param     Data       保存读取数据的缓冲区
 * @param     Length     连续读取的字节数
 * @return    无
 * @example   AS5600_ReadRegs(AS5600_REG_RAW_ANGLE_H, Buffer, 2);
 * @note      中间字节发送ACK，最后一个字节发送NACK
 ****************************************************************/
static void AS5600_ReadRegs(uint8_t RegAddress, uint8_t *Data, uint8_t Length)
{
	BSP_I2C_ReadRegs(AS5600_I2C, AS5600_ADDRESS_7BIT,
		RegAddress, Data, Length);
}

/***********************************************************
 * @brief     将AS5600连续寄存器组合成12位数值
 * @param     HighByte 高字节，只有低4位属于有效数据
 * @param     LowByte  低字节
 * @return    uint16_t 范围为0到4095的12位数值
 * @example   Angle = AS5600_Combine12Bit(Buffer[0], Buffer[1]);
 * @note      高字节中的保留位通过0x0FFF掩码清除
 ****************************************************************/
static uint16_t AS5600_Combine12Bit(uint8_t HighByte, uint8_t LowByte)
{
	return (uint16_t)((((uint16_t)HighByte << 8) | LowByte) & 0x0FFF);
}

/***********************************************************
 * @brief     初始化AS5600所使用的软件I2C总线
 * @param     I2C 指向AS5600所使用的软件I2C总线配置
 * @return    无
 * @example   AS5600_Init(&SensorI2C);
 * @note      AS5600上电后即可测量，本函数不修改角度范围且不烧写OTP
 ****************************************************************/
void AS5600_Init(const BSP_I2C_TypeDef *I2C)
{
	AS5600_I2C = I2C;
	BSP_I2C_Init(AS5600_I2C);
	Delay_ms(2);
}

/***********************************************************
 * @brief     读取AS5600未经起止位置缩放的原始角度
 * @param     无
 * @return    uint16_t 12位原始角度，范围为0到4095
 * @example   RawAngle = AS5600_ReadRawAngle();
 * @note      一圈被均分为4096份，每个计数约等于0.08789度
 ****************************************************************/
uint16_t AS5600_ReadRawAngle(void)
{
	uint8_t Buffer[2];

	AS5600_ReadRegs(AS5600_REG_RAW_ANGLE_H, Buffer, 2);

	return AS5600_Combine12Bit(Buffer[0], Buffer[1]);
}

/***********************************************************
 * @brief     读取AS5600经过零点和角度范围处理后的角度
 * @param     无
 * @return    uint16_t 12位处理后角度，范围为0到4095
 * @example   Angle = AS5600_ReadAngle();
 * @note      未配置ZPOS、MPOS和MANG时，本结果通常与原始角度相同
 ****************************************************************/
uint16_t AS5600_ReadAngle(void)
{
	uint8_t Buffer[2];

	AS5600_ReadRegs(AS5600_REG_ANGLE_H, Buffer, 2);

	return AS5600_Combine12Bit(Buffer[0], Buffer[1]);
}

/***********************************************************
 * @brief     将12位角度计数换算成百分之一度
 * @param     RawAngle 范围为0到4095的12位角度计数
 * @return    uint16_t 角度，单位0.01度，范围为0到35991
 * @example   AngleCentiDegree = AS5600_RawToCentiDegree(2048);
 * @note      使用整数四舍五入计算，2048对应180.00度
 ****************************************************************/
uint16_t AS5600_RawToCentiDegree(uint16_t RawAngle)
{
	return (uint16_t)(((uint32_t)(RawAngle & 0x0FFF) * 36000 + 2048) / 4096);
}

/***********************************************************
 * @brief     一次读取AS5600角度、磁铁状态和磁场强度数据
 * @param     Data 指向保存AS5600测量结果的数据结构
 * @return    无
 * @example   AS5600_GetData(&AS5600_Data);
 * @note      两组角度必须分别读取，因为角度寄存器地址指针会在高低字节间循环
 ****************************************************************/
void AS5600_GetData(AS5600_DataTypeDef *Data)
{
	uint8_t RawAngleBuffer[2];
	uint8_t AngleBuffer[2];
	uint8_t MagnetBuffer[3];

	Data->Status = AS5600_ReadReg(AS5600_REG_STATUS);
	AS5600_ReadRegs(AS5600_REG_RAW_ANGLE_H, RawAngleBuffer, 2);
	AS5600_ReadRegs(AS5600_REG_ANGLE_H, AngleBuffer, 2);
	AS5600_ReadRegs(AS5600_REG_AGC, MagnetBuffer, 3);

	Data->RawAngle = AS5600_Combine12Bit(RawAngleBuffer[0], RawAngleBuffer[1]);
	Data->Angle = AS5600_Combine12Bit(AngleBuffer[0], AngleBuffer[1]);
	Data->AngleCentiDegree = AS5600_RawToCentiDegree(Data->Angle);
	Data->AGC = MagnetBuffer[0];
	Data->Magnitude = AS5600_Combine12Bit(MagnetBuffer[1], MagnetBuffer[2]);
}
