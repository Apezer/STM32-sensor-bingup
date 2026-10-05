#include "stm32f10x.h"                  /* 设备头文件 */
#include "Delay.h"
#include "OLED.h"
#include "BSP_I2C.h"
#include "BSP_SPI.h"
#include "W25Q64.h"
#include <stdio.h>

#define W25Q64_TEST_ADDRESS       (W25Q64_CAPACITY_BYTES - W25Q64_SECTOR_SIZE)
#define W25Q64_TEST_DATA_LENGTH   16

static const BSP_I2C_TypeDef OLED_I2C =
{
	GPIOB, GPIO_Pin_8,           /* 时钟线SCL */
	GPIOB, GPIO_Pin_9,           /* 数据线SDA */
	RCC_APB2Periph_GPIOB         /* GPIO端口时钟 */
};

static const BSP_SPI_TypeDef Flash_SPI =
{
	GPIOB, GPIO_Pin_10,          /* 时钟线CLK/SCK */
	GPIOB, GPIO_Pin_1,           /* 主机输入MISO，对应模块DO */
	GPIOB, GPIO_Pin_11,          /* 主机输出MOSI，对应模块DI */
	RCC_APB2Periph_GPIOB         /* GPIO端口时钟 */
};

static const W25Q64_HandleTypeDef W25Q64_Flash =
{
	&Flash_SPI,                  /* 软件SPI总线 */
	GPIOB, GPIO_Pin_0,           /* 片选线CS */
	RCC_APB2Periph_GPIOB         /* CS端口时钟 */
};

static const uint8_t W25Q64_TestWriteData[W25Q64_TEST_DATA_LENGTH] =
{
	0x57, 0x32, 0x35, 0x51, 0x36, 0x34, 0x46, 0x56,
	0x20, 0x54, 0x45, 0x53, 0x54, 0xA5, 0x5A, 0x00
};

/***********************************************************
 * @brief     检查缓冲区中的每个字节是否都等于指定值
 * @param     Data   指向待检查的数据缓冲区
 * @param     Length 需要检查的字节数
 * @param     Value  期望每个字节具有的值
 * @return    uint8_t 全部相等返回1，否则返回0
 * @example   Result = Buffer_IsValue(Buffer, 16, 0xFF);
 * @note      用于确认W25Q64测试扇区是否已经擦除
 ****************************************************************/
static uint8_t Buffer_IsValue(const uint8_t *Data, uint16_t Length, uint8_t Value)
{
	uint16_t i;

	for (i = 0; i < Length; i++)
	{
		if (Data[i] != Value)
		{
			return 0;
		}
	}

	return 1;
}

/***********************************************************
 * @brief     比较两个缓冲区中的数据是否完全相同
 * @param     Data1  指向第一个数据缓冲区
 * @param     Data2  指向第二个数据缓冲区
 * @param     Length 需要比较的字节数
 * @return    uint8_t 全部相同返回1，否则返回0
 * @example   Result = Buffer_IsEqual(WriteData, ReadData, 16);
 * @note      用于检查写入W25Q64的数据能否被正确读回
 ****************************************************************/
static uint8_t Buffer_IsEqual(const uint8_t *Data1, const uint8_t *Data2, uint16_t Length)
{
	uint16_t i;

	for (i = 0; i < Length; i++)
	{
		if (Data1[i] != Data2[i])
		{
			return 0;
		}
	}

	return 1;
}

/***********************************************************
 * @brief     初始化W25Q64FV并执行扇区擦除、写入和读回校验
 * @param     无
 * @return    int 程序入口返回值，正常运行时不会返回
 * @example   main();
 * @note      测试会擦除并使用最后一个4KB扇区，地址范围为0x7FF000到0x7FFFFF
 ****************************************************************/
int main(void)
{
	uint32_t JEDECID;
	uint16_t ManufacturerDeviceID;
	uint8_t ReadData[W25Q64_TEST_DATA_LENGTH];
	uint8_t ErasePassed;
	uint8_t WritePassed = 0;
	char OLED_Line[17];

	OLED_Init(&OLED_I2C);
	W25Q64_Init(&W25Q64_Flash);
	OLED_Clear();

	JEDECID = W25Q64_ReadJEDECID();
	ManufacturerDeviceID = W25Q64_ReadManufacturerDeviceID();

	if (JEDECID != W25Q64_JEDEC_ID)
	{
		OLED_ShowString(1, 1, "W25Q64FV CHECK  ");
		sprintf(OLED_Line, "JEDEC:%06lX   ", (unsigned long)JEDECID);
		OLED_ShowString(2, 1, OLED_Line);
		sprintf(OLED_Line, "MFG:%02X DEV:%02X  ",
			(unsigned int)(ManufacturerDeviceID >> 8),
			(unsigned int)(ManufacturerDeviceID & 0xFF));
		OLED_ShowString(3, 1, OLED_Line);
		OLED_ShowString(4, 1, "NO ERASE/WRITE  ");
		OLED_Refresh();
		while (1)
		{
			Delay_ms(200);
		}
	}

	W25Q64_EraseSector(W25Q64_TEST_ADDRESS);
	W25Q64_ReadData(W25Q64_TEST_ADDRESS, ReadData, W25Q64_TEST_DATA_LENGTH);
	ErasePassed = Buffer_IsValue(ReadData, W25Q64_TEST_DATA_LENGTH, 0xFF);

	if (ErasePassed == 1)
	{
		W25Q64_WriteData(W25Q64_TEST_ADDRESS, W25Q64_TestWriteData,
			W25Q64_TEST_DATA_LENGTH);
		W25Q64_ReadData(W25Q64_TEST_ADDRESS, ReadData, W25Q64_TEST_DATA_LENGTH);
		WritePassed = Buffer_IsEqual(W25Q64_TestWriteData, ReadData,
			W25Q64_TEST_DATA_LENGTH);
	}

	OLED_ShowString(1, 1, "W25Q64 RW TEST ");
	OLED_ShowString(2, 1, "ADDR:7FF000    ");
	OLED_ShowString(3, 1, (ErasePassed == 1) ? "ERASE:OK       " : "ERASE:FAIL     ");
	if (ErasePassed == 0)
	{
		OLED_ShowString(4, 1, "WRITE:SKIP     ");
	}
	else
	{
		OLED_ShowString(4, 1, (WritePassed == 1) ? "WRITE:PASS     " : "WRITE:FAIL     ");
	}
	OLED_Refresh();

	while (1)
	{
		Delay_ms(200);
	}
}
