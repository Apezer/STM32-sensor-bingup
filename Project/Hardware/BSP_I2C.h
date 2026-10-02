#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "stm32f10x.h"

typedef struct
{
	GPIO_TypeDef *SCL_GPIO_Port;
	uint16_t SCL_GPIO_Pin;
	GPIO_TypeDef *SDA_GPIO_Port;
	uint16_t SDA_GPIO_Pin;
	/* If SCL and SDA use different ports, combine both clocks with |. */
	uint32_t GPIO_Clock;
	uint16_t DelayTimeUs;
} BSP_I2C_TypeDef;

void BSP_I2C_Init(const BSP_I2C_TypeDef *I2C);
void BSP_I2C_Start(const BSP_I2C_TypeDef *I2C);
void BSP_I2C_Stop(const BSP_I2C_TypeDef *I2C);
void BSP_I2C_SendByte(const BSP_I2C_TypeDef *I2C, uint8_t Byte);
uint8_t BSP_I2C_ReceiveByte(const BSP_I2C_TypeDef *I2C);
void BSP_I2C_SendAck(const BSP_I2C_TypeDef *I2C);
void BSP_I2C_SendNAck(const BSP_I2C_TypeDef *I2C);
uint8_t BSP_I2C_ReceiveAck(const BSP_I2C_TypeDef *I2C);
uint8_t BSP_I2C_ReadReg(const BSP_I2C_TypeDef *I2C,
	uint8_t DeviceAddress, uint8_t RegAddress);

#endif
