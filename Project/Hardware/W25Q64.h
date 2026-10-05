#ifndef __W25Q64_H
#define __W25Q64_H

#include "stm32f10x.h"
#include "BSP_SPI.h"

/* W25Q64FV容量和擦写单元。 */
#define W25Q64_CAPACITY_BYTES               0x800000UL
#define W25Q64_MAX_ADDRESS                  0x7FFFFFUL
#define W25Q64_PAGE_SIZE                    256
#define W25Q64_SECTOR_SIZE                  4096UL
#define W25Q64_BLOCK_32KB_SIZE              32768UL
#define W25Q64_BLOCK_64KB_SIZE              65536UL

/* W25Q64FV常用SPI指令。 */
#define W25Q64_CMD_WRITE_ENABLE             0x06
#define W25Q64_CMD_WRITE_DISABLE            0x04
#define W25Q64_CMD_READ_STATUS_REG1         0x05
#define W25Q64_CMD_READ_DATA                0x03
#define W25Q64_CMD_PAGE_PROGRAM             0x02
#define W25Q64_CMD_SECTOR_ERASE_4KB         0x20
#define W25Q64_CMD_BLOCK_ERASE_32KB         0x52
#define W25Q64_CMD_BLOCK_ERASE_64KB         0xD8
#define W25Q64_CMD_CHIP_ERASE               0xC7
#define W25Q64_CMD_READ_MANUFACTURER_ID     0x90
#define W25Q64_CMD_READ_JEDEC_ID            0x9F
#define W25Q64_CMD_POWER_DOWN               0xB9
#define W25Q64_CMD_RELEASE_POWER_DOWN       0xAB
#define W25Q64_CMD_ENABLE_RESET             0x66
#define W25Q64_CMD_RESET_DEVICE             0x99

/* 状态寄存器1位定义。 */
#define W25Q64_STATUS_BUSY                  0x01
#define W25Q64_STATUS_WRITE_ENABLE_LATCH    0x02

/* W25Q64FV器件识别值。 */
#define W25Q64_MANUFACTURER_ID              0xEF
#define W25Q64_DEVICE_ID                    0x16
#define W25Q64_JEDEC_ID                     0xEF4017UL

typedef struct
{
	const BSP_SPI_TypeDef *SPI;    /* 芯片所连接的软件SPI总线。 */
	GPIO_TypeDef *CS_GPIO_Port;    /* 芯片片选引脚端口。 */
	uint16_t CS_GPIO_Pin;          /* 芯片片选引脚。 */
	uint32_t CS_GPIO_Clock;        /* 芯片片选引脚端口时钟。 */
} W25Q64_HandleTypeDef;

void W25Q64_Init(const W25Q64_HandleTypeDef *Device);
uint8_t W25Q64_ReadStatusRegister1(void);
uint32_t W25Q64_ReadJEDECID(void);
uint16_t W25Q64_ReadManufacturerDeviceID(void);
void W25Q64_WriteEnable(void);
void W25Q64_WaitWhileBusy(void);
void W25Q64_ReadData(uint32_t Address, uint8_t *Data, uint32_t Length);
void W25Q64_PageProgram(uint32_t Address, const uint8_t *Data, uint16_t Length);
void W25Q64_WriteData(uint32_t Address, const uint8_t *Data, uint32_t Length);
void W25Q64_EraseSector(uint32_t Address);
void W25Q64_EraseBlock32KB(uint32_t Address);
void W25Q64_EraseBlock64KB(uint32_t Address);
void W25Q64_EraseChip(void);
void W25Q64_PowerDown(void);
void W25Q64_WakeUp(void);

#endif
