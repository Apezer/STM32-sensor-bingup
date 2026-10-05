#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "stm32f10x.h"

typedef struct
{
	GPIO_TypeDef *SCK_GPIO_Port;
	uint16_t SCK_GPIO_Pin;
	GPIO_TypeDef *MISO_GPIO_Port;
	uint16_t MISO_GPIO_Pin;
	GPIO_TypeDef *MOSI_GPIO_Port;
	uint16_t MOSI_GPIO_Pin;
	/* 如果三根信号线使用不同端口，应使用|合并所有GPIO时钟。 */
	uint32_t GPIO_Clock;
} BSP_SPI_TypeDef;

void BSP_SPI_Init(const BSP_SPI_TypeDef *SPI);
uint8_t BSP_SPI_TransferByte(const BSP_SPI_TypeDef *SPI, uint8_t SendData);

#endif
