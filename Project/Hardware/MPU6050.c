#include "stm32f10x.h"
#include "Delay.h"
#include "BSP_I2C.h"
#include "MPU6050.h"

/* AD0 is low, so the MPU6050 7-bit I2C address is 0x68. */
#define MPU6050_ADDRESS_7BIT   0x68
#define MPU6050_WRITE_ADDRESS  (MPU6050_ADDRESS_7BIT << 1)
#define MPU6050_READ_ADDRESS   ((MPU6050_ADDRESS_7BIT << 1) | 0x01)

static const BSP_I2C_TypeDef *MPU6050_I2C;

/***********************************************************
 * @brief     Writes one byte to an MPU6050 register
 * @param     RegAddress MPU6050 register address
 * @param     Data       Byte to write
 * @return    void
 * @example   MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);
 * @note      MPU6050_Init must be called before using this function
 ****************************************************************/
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	BSP_I2C_Start(MPU6050_I2C);
	BSP_I2C_SendByte(MPU6050_I2C, MPU6050_WRITE_ADDRESS);
	BSP_I2C_ReceiveAck(MPU6050_I2C);
	BSP_I2C_SendByte(MPU6050_I2C, RegAddress);
	BSP_I2C_ReceiveAck(MPU6050_I2C);
	BSP_I2C_SendByte(MPU6050_I2C, Data);
	BSP_I2C_ReceiveAck(MPU6050_I2C);
	BSP_I2C_Stop(MPU6050_I2C);
}

/***********************************************************
 * @brief     Reads one byte from an MPU6050 register
 * @param     RegAddress MPU6050 register address
 * @return    uint8_t Register value
 * @example   DeviceID = MPU6050_ReadReg(MPU6050_WHO_AM_I);
 * @note      A repeated start condition is used before switching to read mode
 ****************************************************************/
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;

	BSP_I2C_Start(MPU6050_I2C);
	BSP_I2C_SendByte(MPU6050_I2C, MPU6050_WRITE_ADDRESS);
	BSP_I2C_ReceiveAck(MPU6050_I2C);
	BSP_I2C_SendByte(MPU6050_I2C, RegAddress);
	BSP_I2C_ReceiveAck(MPU6050_I2C);

	BSP_I2C_Start(MPU6050_I2C);
	BSP_I2C_SendByte(MPU6050_I2C, MPU6050_READ_ADDRESS);
	BSP_I2C_ReceiveAck(MPU6050_I2C);
	Data = BSP_I2C_ReceiveByte(MPU6050_I2C);
	BSP_I2C_SendNAck(MPU6050_I2C);
	BSP_I2C_Stop(MPU6050_I2C);

	return Data;
}

/***********************************************************
 * @brief     Reads multiple consecutive MPU6050 registers
 * @param     RegAddress Address of the first register to read
 * @param     Data       Destination buffer
 * @param     Length     Number of bytes to read
 * @return    void
 * @example   MPU6050_ReadRegs(MPU6050_ACCEL_XOUT_H, Buffer, 14);
 * @note      ACK is sent after intermediate bytes and NACK is sent after the final byte
 ****************************************************************/
static void MPU6050_ReadRegs(uint8_t RegAddress, uint8_t *Data, uint8_t Length)
{
	uint8_t i;

	BSP_I2C_Start(MPU6050_I2C);
	BSP_I2C_SendByte(MPU6050_I2C, MPU6050_WRITE_ADDRESS);
	BSP_I2C_ReceiveAck(MPU6050_I2C);
	BSP_I2C_SendByte(MPU6050_I2C, RegAddress);
	BSP_I2C_ReceiveAck(MPU6050_I2C);

	BSP_I2C_Start(MPU6050_I2C);
	BSP_I2C_SendByte(MPU6050_I2C, MPU6050_READ_ADDRESS);
	BSP_I2C_ReceiveAck(MPU6050_I2C);
	for (i = 0; i < Length; i++)
	{
		Data[i] = BSP_I2C_ReceiveByte(MPU6050_I2C);
		if (i == Length - 1)
		{
			BSP_I2C_SendNAck(MPU6050_I2C);
		}
		else
		{
			BSP_I2C_SendAck(MPU6050_I2C);
		}
	}
	BSP_I2C_Stop(MPU6050_I2C);
}

/***********************************************************
 * @brief     Initializes the software I2C bus and configures the MPU6050
 * @param     I2C Pointer to the software I2C bus configuration used by the MPU6050
 * @return    void
 * @example   MPU6050_Init(&SensorI2C);
 * @note      The default ranges are +/-2 g for acceleration and +/-250 dps for angular rate
 ****************************************************************/
void MPU6050_Init(const BSP_I2C_TypeDef *I2C)
{
	MPU6050_I2C = I2C;
	BSP_I2C_Init(MPU6050_I2C);
	Delay_ms(100);

	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x80);
	Delay_ms(100);
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09);
	MPU6050_WriteReg(MPU6050_CONFIG, 0x03);
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x00);
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x00);
}

/***********************************************************
 * @brief     Reads the MPU6050 device identification register
 * @param     void
 * @return    uint8_t Device ID, normally 0x68 when AD0 is low
 * @example   DeviceID = MPU6050_GetID();
 * @note      This function reads the WHO_AM_I register
 ****************************************************************/
uint8_t MPU6050_GetID(void)
{
	return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

/***********************************************************
 * @brief     Reads raw acceleration, temperature, and angular-rate data
 * @param     Data Pointer to the structure that receives all raw sensor values
 * @return    void
 * @example   MPU6050_GetData(&MPU6050_Data);
 * @note      Fourteen consecutive bytes are read starting at ACCEL_XOUT_H
 ****************************************************************/
void MPU6050_GetData(MPU6050_DataTypeDef *Data)
{
	uint8_t Buffer[14];

	MPU6050_ReadRegs(MPU6050_ACCEL_XOUT_H, Buffer, 14);

	Data->AccelX = (int16_t)((Buffer[0] << 8) | Buffer[1]);
	Data->AccelY = (int16_t)((Buffer[2] << 8) | Buffer[3]);
	Data->AccelZ = (int16_t)((Buffer[4] << 8) | Buffer[5]);
	Data->Temperature = (int16_t)((Buffer[6] << 8) | Buffer[7]);
	Data->GyroX = (int16_t)((Buffer[8] << 8) | Buffer[9]);
	Data->GyroY = (int16_t)((Buffer[10] << 8) | Buffer[11]);
	Data->GyroZ = (int16_t)((Buffer[12] << 8) | Buffer[13]);
}

/***********************************************************
 * @brief     Converts a raw accelerometer value to milligravity
 * @param     RawValue Raw accelerometer value measured with the +/-2 g range
 * @return    int32_t Acceleration in mg
 * @example   AccelMg = MPU6050_AccelRawToMg(MPU6050_Data.AccelX);
 * @note      The conversion uses the +/-2 g sensitivity of 16384 LSB/g
 ****************************************************************/
int32_t MPU6050_AccelRawToMg(int16_t RawValue)
{
	return ((int32_t)RawValue * 1000) / 16384;
}

/***********************************************************
 * @brief     Converts a raw gyroscope value to tenths of a degree per second
 * @param     RawValue Raw gyroscope value measured with the +/-250 dps range
 * @return    int32_t Angular rate in 0.1 dps
 * @example   GyroDeciDps = MPU6050_GyroRawToDeciDps(MPU6050_Data.GyroX);
 * @note      The conversion uses the +/-250 dps sensitivity of 131 LSB/(dps)
 ****************************************************************/
int32_t MPU6050_GyroRawToDeciDps(int16_t RawValue)
{
	return ((int32_t)RawValue * 10) / 131;
}

/***********************************************************
 * @brief     Converts a raw temperature value to hundredths of a degree Celsius
 * @param     RawValue Raw MPU6050 temperature value
 * @return    int32_t Temperature in 0.01 degrees Celsius
 * @example   Temperature = MPU6050_TemperatureRawToCentiDegree(MPU6050_Data.Temperature);
 * @note      The integer conversion follows the MPU6050 temperature formula
 ****************************************************************/
int32_t MPU6050_TemperatureRawToCentiDegree(int16_t RawValue)
{
	return ((int32_t)RawValue * 100) / 340 + 3653;
}
