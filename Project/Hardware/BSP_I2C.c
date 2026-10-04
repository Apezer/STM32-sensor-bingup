#include "stm32f10x.h"
#include "BSP_I2C.h"

/***********************************************************
 * @brief     Sets the software I2C clock line level
 * @param     I2C      Pointer to the software I2C bus configuration
 * @param     BitValue Clock line level, 0 for low and non-zero for released high
 * @return    void
 * @example   BSP_I2C_WriteSCL(I2C, 1);
 * @note      This function changes only the GPIO output level
 ****************************************************************/
static void BSP_I2C_WriteSCL(const BSP_I2C_TypeDef *I2C, uint8_t BitValue)
{
	GPIO_WriteBit(I2C->SCL_GPIO_Port, I2C->SCL_GPIO_Pin, (BitAction)BitValue);
}

/***********************************************************
 * @brief     Sets the software I2C data line level
 * @param     I2C      Pointer to the software I2C bus configuration
 * @param     BitValue Data line level, 0 for low and non-zero for released high
 * @return    void
 * @example   BSP_I2C_WriteSDA(I2C, 0);
 * @note      Writing 1 releases SDA because the GPIO is configured as open-drain
 ****************************************************************/
static void BSP_I2C_WriteSDA(const BSP_I2C_TypeDef *I2C, uint8_t BitValue)
{
	GPIO_WriteBit(I2C->SDA_GPIO_Port, I2C->SDA_GPIO_Pin, (BitAction)BitValue);
}

/***********************************************************
 * @brief     Reads the current software I2C data line level
 * @param     I2C Pointer to the software I2C bus configuration
 * @return    uint8_t SDA level, 0 for low and 1 for high
 * @example   BitValue = BSP_I2C_ReadSDA(I2C);
 * @note      SDA must be released before reading data driven by a slave device
 ****************************************************************/
static uint8_t BSP_I2C_ReadSDA(const BSP_I2C_TypeDef *I2C)
{
	return GPIO_ReadInputDataBit(I2C->SDA_GPIO_Port, I2C->SDA_GPIO_Pin);
}

/***********************************************************
 * @brief     Initializes a GPIO-based software I2C bus
 * @param     I2C Pointer to the software I2C bus configuration
 * @return    void
 * @example   BSP_I2C_Init(&SensorI2C);
 * @note      SCL and SDA are configured as open-drain outputs and released high
 ****************************************************************/
void BSP_I2C_Init(const BSP_I2C_TypeDef *I2C)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(I2C->GPIO_Clock, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = I2C->SCL_GPIO_Pin;
	GPIO_Init(I2C->SCL_GPIO_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = I2C->SDA_GPIO_Pin;
	GPIO_Init(I2C->SDA_GPIO_Port, &GPIO_InitStructure);

	GPIO_SetBits(I2C->SCL_GPIO_Port, I2C->SCL_GPIO_Pin);
	GPIO_SetBits(I2C->SDA_GPIO_Port, I2C->SDA_GPIO_Pin);
}

/***********************************************************
 * @brief     Generates an I2C start condition
 * @param     I2C Pointer to the software I2C bus configuration
 * @return    void
 * @example   BSP_I2C_Start(I2C);
 * @note      SDA changes from high to low while SCL remains high
 ****************************************************************/
void BSP_I2C_Start(const BSP_I2C_TypeDef *I2C)
{
	BSP_I2C_WriteSDA(I2C, 1);
	BSP_I2C_WriteSCL(I2C, 1);
	BSP_I2C_WriteSDA(I2C, 0);
	BSP_I2C_WriteSCL(I2C, 0);
}

/***********************************************************
 * @brief     Generates an I2C stop condition
 * @param     I2C Pointer to the software I2C bus configuration
 * @return    void
 * @example   BSP_I2C_Stop(I2C);
 * @note      SDA changes from low to high while SCL remains high
 ****************************************************************/
void BSP_I2C_Stop(const BSP_I2C_TypeDef *I2C)
{
	BSP_I2C_WriteSCL(I2C, 0);
	BSP_I2C_WriteSDA(I2C, 0);
	BSP_I2C_WriteSCL(I2C, 1);
	BSP_I2C_WriteSDA(I2C, 1);
}

/***********************************************************
 * @brief     Sends one byte on the software I2C bus
 * @param     I2C  Pointer to the software I2C bus configuration
 * @param     Byte Byte to send
 * @return    void
 * @example   BSP_I2C_SendByte(I2C, 0xD0);
 * @note      Data is transmitted from the most significant bit to the least significant bit
 ****************************************************************/
void BSP_I2C_SendByte(const BSP_I2C_TypeDef *I2C, uint8_t Byte)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		BSP_I2C_WriteSCL(I2C, 0);
		BSP_I2C_WriteSDA(I2C, Byte & (0x80 >> i));
		BSP_I2C_WriteSCL(I2C, 1);
	}
	BSP_I2C_WriteSCL(I2C, 0);
}

/***********************************************************
 * @brief     Receives one byte from the software I2C bus
 * @param     I2C Pointer to the software I2C bus configuration
 * @return    uint8_t Received byte
 * @example   Data = BSP_I2C_ReceiveByte(I2C);
 * @note      This function only receives data; the caller must send ACK or NACK afterward
 ****************************************************************/
uint8_t BSP_I2C_ReceiveByte(const BSP_I2C_TypeDef *I2C)
{
	uint8_t i;
	uint8_t Byte = 0x00;

	BSP_I2C_WriteSDA(I2C, 1);
	for (i = 0; i < 8; i++)
	{
		BSP_I2C_WriteSCL(I2C, 0);
		BSP_I2C_WriteSCL(I2C, 1);
		if (BSP_I2C_ReadSDA(I2C) == 1)
		{
			Byte |= (0x80 >> i);
		}
	}
	BSP_I2C_WriteSCL(I2C, 0);

	return Byte;
}

/***********************************************************
 * @brief     Sends an I2C acknowledge bit
 * @param     I2C Pointer to the software I2C bus configuration
 * @return    void
 * @example   BSP_I2C_SendAck(I2C);
 * @note      ACK is represented by a low SDA level during the ninth clock pulse
 ****************************************************************/
void BSP_I2C_SendAck(const BSP_I2C_TypeDef *I2C)
{
	BSP_I2C_WriteSCL(I2C, 0);
	BSP_I2C_WriteSDA(I2C, 0);
	BSP_I2C_WriteSCL(I2C, 1);
	BSP_I2C_WriteSCL(I2C, 0);
	BSP_I2C_WriteSDA(I2C, 1);
}

/***********************************************************
 * @brief     Sends an I2C not-acknowledge bit
 * @param     I2C Pointer to the software I2C bus configuration
 * @return    void
 * @example   BSP_I2C_SendNAck(I2C);
 * @note      NACK is represented by a released high SDA level during the ninth clock pulse
 ****************************************************************/
void BSP_I2C_SendNAck(const BSP_I2C_TypeDef *I2C)
{
	BSP_I2C_WriteSCL(I2C, 0);
	BSP_I2C_WriteSDA(I2C, 1);
	BSP_I2C_WriteSCL(I2C, 1);
	BSP_I2C_WriteSCL(I2C, 0);
}

/***********************************************************
 * @brief     Receives an acknowledge bit from an I2C slave device
 * @param     I2C Pointer to the software I2C bus configuration
 * @return    uint8_t Acknowledge bit, 0 for ACK and 1 for NACK
 * @example   AckBit = BSP_I2C_ReceiveAck(I2C);
 * @note      The master releases SDA before sampling the acknowledge bit
 ****************************************************************/
uint8_t BSP_I2C_ReceiveAck(const BSP_I2C_TypeDef *I2C)
{
	uint8_t AckBit;

	BSP_I2C_WriteSCL(I2C, 0);
	BSP_I2C_WriteSDA(I2C, 1);
	BSP_I2C_WriteSCL(I2C, 1);
	AckBit = BSP_I2C_ReadSDA(I2C);
	BSP_I2C_WriteSCL(I2C, 0);

	return AckBit;
}

/***********************************************************
 * @brief     向使用8位寄存器地址的I2C设备写入一个字节
 * @param     I2C           指向软件I2C总线配置
 * @param     DeviceAddress 7位I2C设备地址
 * @param     RegAddress    8位寄存器地址
 * @param     Data          要写入的一个字节数据
 * @return    无
 * @example   BSP_I2C_WriteReg(I2C, 0x76, 0xF4, 0x4F);
 * @note      DeviceAddress不包含最低位的读写方向位
 ****************************************************************/
void BSP_I2C_WriteReg(const BSP_I2C_TypeDef *I2C,
	uint8_t DeviceAddress, uint8_t RegAddress, uint8_t Data)
{
	BSP_I2C_Start(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)(DeviceAddress << 1));
	BSP_I2C_ReceiveAck(I2C);
	BSP_I2C_SendByte(I2C, RegAddress);
	BSP_I2C_ReceiveAck(I2C);
	BSP_I2C_SendByte(I2C, Data);
	BSP_I2C_ReceiveAck(I2C);
	BSP_I2C_Stop(I2C);
}

/***********************************************************
 * @brief     Reads one byte from an I2C device register
 * @param     I2C           Pointer to the software I2C bus configuration
 * @param     DeviceAddress 7-bit I2C device address
 * @param     RegAddress    Register address to read
 * @return    uint8_t Register value
 * @example   Data = BSP_I2C_ReadReg(I2C, 0x68, 0x75);
 * @note      This function supports devices that use an 8-bit register address
 ****************************************************************/
uint8_t BSP_I2C_ReadReg(const BSP_I2C_TypeDef *I2C,	uint8_t DeviceAddress, uint8_t RegAddress)
{
	uint8_t Data;

	BSP_I2C_Start(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)(DeviceAddress << 1));
	BSP_I2C_ReceiveAck(I2C);
	BSP_I2C_SendByte(I2C, RegAddress);
	BSP_I2C_ReceiveAck(I2C);

	BSP_I2C_Start(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)((DeviceAddress << 1) | 0x01));
	BSP_I2C_ReceiveAck(I2C);
	Data = BSP_I2C_ReceiveByte(I2C);
	BSP_I2C_SendNAck(I2C);
	BSP_I2C_Stop(I2C);

	return Data;
}

/***********************************************************
 * @brief     连续读取使用8位寄存器地址的多个寄存器
 * @param     I2C           指向软件I2C总线配置
 * @param     DeviceAddress 7位I2C设备地址
 * @param     RegAddress    起始寄存器地址
 * @param     Data          保存读取数据的缓冲区
 * @param     Length        需要连续读取的字节数
 * @return    无
 * @example   BSP_I2C_ReadRegs(I2C, 0x76, 0xF7, Buffer, 6);
 * @note      中间字节发送ACK，最后一个字节发送NACK；Length为0时不访问总线
 ****************************************************************/
void BSP_I2C_ReadRegs(const BSP_I2C_TypeDef *I2C,
	uint8_t DeviceAddress, uint8_t RegAddress, uint8_t *Data, uint8_t Length)
{
	uint8_t i;

	if (Length == 0)
	{
		return;
	}

	BSP_I2C_Start(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)(DeviceAddress << 1));
	BSP_I2C_ReceiveAck(I2C);
	BSP_I2C_SendByte(I2C, RegAddress);
	BSP_I2C_ReceiveAck(I2C);

	BSP_I2C_Start(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)((DeviceAddress << 1) | 0x01));
	BSP_I2C_ReceiveAck(I2C);
	for (i = 0; i < Length; i++)
	{
		Data[i] = BSP_I2C_ReceiveByte(I2C);
		if (i == Length - 1)
		{
			BSP_I2C_SendNAck(I2C);
		}
		else
		{
			BSP_I2C_SendAck(I2C);
		}
	}
	BSP_I2C_Stop(I2C);
}

/***********************************************************
 * @brief     向使用16位寄存器地址的I2C设备写入一个字节
 * @param     I2C           指向软件I2C总线配置
 * @param     DeviceAddress 7位I2C设备地址
 * @param     RegAddress    16位寄存器地址
 * @param     Data          要写入的一个字节数据
 * @return    无
 * @example   BSP_I2C_WriteReg16Addr(I2C, 0x29, 0x0018, 0x01);
 * @note      寄存器地址按照高字节在前、低字节在后的顺序发送
 ****************************************************************/
void BSP_I2C_WriteReg16Addr(const BSP_I2C_TypeDef *I2C,
	uint8_t DeviceAddress, uint16_t RegAddress, uint8_t Data)
{
	BSP_I2C_Start(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)(DeviceAddress << 1));
	BSP_I2C_ReceiveAck(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)(RegAddress >> 8));
	BSP_I2C_ReceiveAck(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)RegAddress);
	BSP_I2C_ReceiveAck(I2C);
	BSP_I2C_SendByte(I2C, Data);
	BSP_I2C_ReceiveAck(I2C);
	BSP_I2C_Stop(I2C);
}

/***********************************************************
 * @brief     从使用16位寄存器地址的I2C设备读取一个字节
 * @param     I2C           指向软件I2C总线配置
 * @param     DeviceAddress 7位I2C设备地址
 * @param     RegAddress    16位寄存器地址
 * @return    uint8_t 读取到的寄存器值
 * @example   Data = BSP_I2C_ReadReg16Addr(I2C, 0x29, 0x0000);
 * @note      寄存器地址按照高字节在前、低字节在后的顺序发送
 ****************************************************************/
uint8_t BSP_I2C_ReadReg16Addr(const BSP_I2C_TypeDef *I2C,
	uint8_t DeviceAddress, uint16_t RegAddress)
{
	uint8_t Data;

	BSP_I2C_Start(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)(DeviceAddress << 1));
	BSP_I2C_ReceiveAck(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)(RegAddress >> 8));
	BSP_I2C_ReceiveAck(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)RegAddress);
	BSP_I2C_ReceiveAck(I2C);

	BSP_I2C_Start(I2C);
	BSP_I2C_SendByte(I2C, (uint8_t)((DeviceAddress << 1) | 0x01));
	BSP_I2C_ReceiveAck(I2C);
	Data = BSP_I2C_ReceiveByte(I2C);
	BSP_I2C_SendNAck(I2C);
	BSP_I2C_Stop(I2C);

	return Data;
}
