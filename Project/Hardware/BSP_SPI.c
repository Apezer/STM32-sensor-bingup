#include "stm32f10x.h"
#include "BSP_SPI.h"

/***********************************************************
 * @brief     设置软件SPI时钟线电平
 * @param     SPI     指向软件SPI总线配置
 * @param     BitValue 时钟线电平，0为低电平，非0为高电平
 * @return    无
 * @example   BSP_SPI_WriteSCK(SPI, 0);
 * @note      本工程使用SPI模式0，空闲时SCK保持低电平
 ****************************************************************/
static void BSP_SPI_WriteSCK(const BSP_SPI_TypeDef *SPI, uint8_t BitValue)
{
	GPIO_WriteBit(SPI->SCK_GPIO_Port, SPI->SCK_GPIO_Pin, (BitAction)BitValue);
}

/***********************************************************
 * @brief     设置软件SPI主机输出线电平
 * @param     SPI     指向软件SPI总线配置
 * @param     BitValue MOSI线电平，0为低电平，非0为高电平
 * @return    无
 * @example   BSP_SPI_WriteMOSI(SPI, 1);
 * @note      MOSI也常标记为DI，数据从主机发送到从机
 ****************************************************************/
static void BSP_SPI_WriteMOSI(const BSP_SPI_TypeDef *SPI, uint8_t BitValue)
{
	GPIO_WriteBit(SPI->MOSI_GPIO_Port, SPI->MOSI_GPIO_Pin, (BitAction)BitValue);
}

/***********************************************************
 * @brief     读取软件SPI主机输入线电平
 * @param     SPI 指向软件SPI总线配置
 * @return    uint8_t MISO线电平，0为低电平，1为高电平
 * @example   BitValue = BSP_SPI_ReadMISO(SPI);
 * @note      MISO也常标记为DO，数据从从机返回主机
 ****************************************************************/
static uint8_t BSP_SPI_ReadMISO(const BSP_SPI_TypeDef *SPI)
{
	return GPIO_ReadInputDataBit(SPI->MISO_GPIO_Port, SPI->MISO_GPIO_Pin);
}

/***********************************************************
 * @brief     初始化GPIO模拟的软件SPI总线
 * @param     SPI 指向软件SPI总线配置
 * @return    无
 * @example   BSP_SPI_Init(&FlashSPI);
 * @note      使用SPI模式0和MSB先行；片选CS由具体设备驱动管理
 ****************************************************************/
void BSP_SPI_Init(const BSP_SPI_TypeDef *SPI)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(SPI->GPIO_Clock, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = SPI->SCK_GPIO_Pin;
	GPIO_Init(SPI->SCK_GPIO_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SPI->MOSI_GPIO_Pin;
	GPIO_Init(SPI->MOSI_GPIO_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = SPI->MISO_GPIO_Pin;
	GPIO_Init(SPI->MISO_GPIO_Port, &GPIO_InitStructure);

	BSP_SPI_WriteSCK(SPI, 0);
	BSP_SPI_WriteMOSI(SPI, 1);
}

/***********************************************************
 * @brief     使用软件SPI同时发送并接收一个字节
 * @param     SPI      指向软件SPI总线配置
 * @param     SendData 需要发送的一个字节
 * @return    uint8_t 同一时钟周期内接收到的一个字节
 * @example   ReceiveData = BSP_SPI_TransferByte(SPI, 0xFF);
 * @note      数据MSB先行，在SCK上升沿采样，读取数据时发送0xFF产生时钟
 ****************************************************************/
uint8_t BSP_SPI_TransferByte(const BSP_SPI_TypeDef *SPI, uint8_t SendData)
{
	uint8_t i;
	uint8_t ReceiveData = 0x00;

	for (i = 0; i < 8; i++)
	{
		BSP_SPI_WriteSCK(SPI, 0);
		BSP_SPI_WriteMOSI(SPI, SendData & (0x80 >> i));
		BSP_SPI_WriteSCK(SPI, 1);
		if (BSP_SPI_ReadMISO(SPI) == 1)
		{
			ReceiveData |= (0x80 >> i);
		}
	}
	BSP_SPI_WriteSCK(SPI, 0);

	return ReceiveData;
}
