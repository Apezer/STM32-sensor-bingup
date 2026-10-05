#include "stm32f10x.h"
#include "Delay.h"
#include "BSP_SPI.h"
#include "W25Q64.h"

static const W25Q64_HandleTypeDef *W25Q64_Device;

/***********************************************************
 * @brief     拉低片选并开始一次W25Q64 SPI事务
 * @param     无
 * @return    无
 * @example   W25Q64_Select();
 * @note      一条完整指令及其地址和数据必须在同一次片选期间传输
 ****************************************************************/
static void W25Q64_Select(void)
{
	GPIO_ResetBits(W25Q64_Device->CS_GPIO_Port, W25Q64_Device->CS_GPIO_Pin);
}

/***********************************************************
 * @brief     拉高片选并结束一次W25Q64 SPI事务
 * @param     无
 * @return    无
 * @example   W25Q64_Deselect();
 * @note      页编程和擦除指令在CS上升沿之后开始内部执行
 ****************************************************************/
static void W25Q64_Deselect(void)
{
	GPIO_SetBits(W25Q64_Device->CS_GPIO_Port, W25Q64_Device->CS_GPIO_Pin);
}

/***********************************************************
 * @brief     向W25Q64发送24位存储地址
 * @param     Address 24位存储地址，W25Q64FV有效范围为0x000000到0x7FFFFF
 * @return    无
 * @example   W25Q64_SendAddress(0x001000);
 * @note      地址按照高字节、中字节、低字节的顺序发送
 ****************************************************************/
static void W25Q64_SendAddress(uint32_t Address)
{
	BSP_SPI_TransferByte(W25Q64_Device->SPI, (uint8_t)(Address >> 16));
	BSP_SPI_TransferByte(W25Q64_Device->SPI, (uint8_t)(Address >> 8));
	BSP_SPI_TransferByte(W25Q64_Device->SPI, (uint8_t)Address);
}

/***********************************************************
 * @brief     发送一条没有地址和数据的W25Q64指令
 * @param     Command 要发送的指令码
 * @return    无
 * @example   W25Q64_SendCommand(W25Q64_CMD_WRITE_ENABLE);
 * @note      本函数自动控制CS的拉低和释放
 ****************************************************************/
static void W25Q64_SendCommand(uint8_t Command)
{
	W25Q64_Select();
	BSP_SPI_TransferByte(W25Q64_Device->SPI, Command);
	W25Q64_Deselect();
}

/***********************************************************
 * @brief     初始化W25Q64FV及其软件SPI总线
 * @param     Device 指向W25Q64设备配置，包含SPI总线和独立CS引脚
 * @return    无
 * @example   W25Q64_Init(&FlashDevice);
 * @note      初始化只复位接口状态，不会擦除或修改Flash中的用户数据
 ****************************************************************/
void W25Q64_Init(const W25Q64_HandleTypeDef *Device)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	W25Q64_Device = Device;
	BSP_SPI_Init(W25Q64_Device->SPI);

	RCC_APB2PeriphClockCmd(W25Q64_Device->CS_GPIO_Clock, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = W25Q64_Device->CS_GPIO_Pin;
	GPIO_Init(W25Q64_Device->CS_GPIO_Port, &GPIO_InitStructure);
	W25Q64_Deselect();

	Delay_ms(5);
	W25Q64_SendCommand(W25Q64_CMD_ENABLE_RESET);
	W25Q64_SendCommand(W25Q64_CMD_RESET_DEVICE);
	Delay_ms(1);
}

/***********************************************************
 * @brief     读取W25Q64状态寄存器1
 * @param     无
 * @return    uint8_t 状态寄存器1的当前值
 * @example   Status = W25Q64_ReadStatusRegister1();
 * @note      bit0为忙标志BUSY，bit1为写使能锁存标志WEL
 ****************************************************************/
uint8_t W25Q64_ReadStatusRegister1(void)
{
	uint8_t Status;

	W25Q64_Select();
	BSP_SPI_TransferByte(W25Q64_Device->SPI, W25Q64_CMD_READ_STATUS_REG1);
	Status = BSP_SPI_TransferByte(W25Q64_Device->SPI, 0xFF);
	W25Q64_Deselect();

	return Status;
}

/***********************************************************
 * @brief     读取W25Q64的JEDEC制造商和器件标识
 * @param     无
 * @return    uint32_t 24位JEDEC ID，W25Q64FV正常值为0xEF4017
 * @example   JEDECID = W25Q64_ReadJEDECID();
 * @note      返回值依次包含制造商ID、存储类型和容量代码
 ****************************************************************/
uint32_t W25Q64_ReadJEDECID(void)
{
	uint32_t JEDECID = 0;

	W25Q64_Select();
	BSP_SPI_TransferByte(W25Q64_Device->SPI, W25Q64_CMD_READ_JEDEC_ID);
	JEDECID |= (uint32_t)BSP_SPI_TransferByte(W25Q64_Device->SPI, 0xFF) << 16;
	JEDECID |= (uint32_t)BSP_SPI_TransferByte(W25Q64_Device->SPI, 0xFF) << 8;
	JEDECID |= BSP_SPI_TransferByte(W25Q64_Device->SPI, 0xFF);
	W25Q64_Deselect();

	return JEDECID;
}

/***********************************************************
 * @brief     读取W25Q64制造商ID和器件ID
 * @param     无
 * @return    uint16_t 高字节为制造商ID，低字节为器件ID
 * @example   DeviceID = W25Q64_ReadManufacturerDeviceID();
 * @note      W25Q64FV正常返回0xEF16
 ****************************************************************/
uint16_t W25Q64_ReadManufacturerDeviceID(void)
{
	uint16_t DeviceID;

	W25Q64_Select();
	BSP_SPI_TransferByte(W25Q64_Device->SPI, W25Q64_CMD_READ_MANUFACTURER_ID);
	W25Q64_SendAddress(0x000000);
	DeviceID = (uint16_t)BSP_SPI_TransferByte(W25Q64_Device->SPI, 0xFF) << 8;
	DeviceID |= BSP_SPI_TransferByte(W25Q64_Device->SPI, 0xFF);
	W25Q64_Deselect();

	return DeviceID;
}

/***********************************************************
 * @brief     设置W25Q64写使能锁存位
 * @param     无
 * @return    无
 * @example   W25Q64_WriteEnable();
 * @note      每次页编程或擦除之前都必须重新发送写使能指令
 ****************************************************************/
void W25Q64_WriteEnable(void)
{
	W25Q64_SendCommand(W25Q64_CMD_WRITE_ENABLE);
}

/***********************************************************
 * @brief     等待W25Q64内部编程或擦除操作完成
 * @param     无
 * @return    无
 * @example   W25Q64_WaitWhileBusy();
 * @note      本函数阻塞轮询状态寄存器BUSY位，目前未设置超时
 ****************************************************************/
void W25Q64_WaitWhileBusy(void)
{
	while ((W25Q64_ReadStatusRegister1() & W25Q64_STATUS_BUSY) != 0)
	{
	}
}

/***********************************************************
 * @brief     从W25Q64指定地址连续读取数据
 * @param     Address 起始存储地址
 * @param     Data    保存读取数据的缓冲区
 * @param     Length  需要读取的字节数
 * @return    无
 * @example   W25Q64_ReadData(0x001000, Buffer, 16);
 * @note      连续读取可以跨越页和扇区边界，Length为0时不访问总线
 ****************************************************************/
void W25Q64_ReadData(uint32_t Address, uint8_t *Data, uint32_t Length)
{
	uint32_t i;

	if (Length == 0)
	{
		return;
	}

	W25Q64_Select();
	BSP_SPI_TransferByte(W25Q64_Device->SPI, W25Q64_CMD_READ_DATA);
	W25Q64_SendAddress(Address);
	for (i = 0; i < Length; i++)
	{
		Data[i] = BSP_SPI_TransferByte(W25Q64_Device->SPI, 0xFF);
	}
	W25Q64_Deselect();
}

/***********************************************************
 * @brief     在W25Q64当前页内编程数据
 * @param     Address 页内起始存储地址
 * @param     Data    指向待写入数据
 * @param     Length  待写入字节数
 * @return    无
 * @example   W25Q64_PageProgram(0x001000, Buffer, 16);
 * @note      超过当前页剩余空间的数据会被截断；写入前需保证目标区域已经擦除
 ****************************************************************/
void W25Q64_PageProgram(uint32_t Address, const uint8_t *Data, uint16_t Length)
{
	uint16_t i;
	uint16_t PageRemain;

	if (Length == 0)
	{
		return;
	}

	PageRemain = (uint16_t)(W25Q64_PAGE_SIZE - (Address & (W25Q64_PAGE_SIZE - 1)));
	if (Length > PageRemain)
	{
		Length = PageRemain;
	}

	W25Q64_WriteEnable();
	W25Q64_Select();
	BSP_SPI_TransferByte(W25Q64_Device->SPI, W25Q64_CMD_PAGE_PROGRAM);
	W25Q64_SendAddress(Address);
	for (i = 0; i < Length; i++)
	{
		BSP_SPI_TransferByte(W25Q64_Device->SPI, Data[i]);
	}
	W25Q64_Deselect();
	W25Q64_WaitWhileBusy();
}

/***********************************************************
 * @brief     向W25Q64连续写入任意长度数据
 * @param     Address 起始存储地址
 * @param     Data    指向待写入数据
 * @param     Length  待写入字节数
 * @return    无
 * @example   W25Q64_WriteData(0x0010F0, Buffer, 32);
 * @note      本函数自动跨页拆分，但不会自动擦除；Flash编程只能把位从1写成0
 ****************************************************************/
void W25Q64_WriteData(uint32_t Address, const uint8_t *Data, uint32_t Length)
{
	uint16_t WriteLength;
	uint16_t PageRemain;

	while (Length > 0)
	{
		PageRemain = (uint16_t)(W25Q64_PAGE_SIZE - (Address & (W25Q64_PAGE_SIZE - 1)));
		WriteLength = (Length < PageRemain) ? (uint16_t)Length : PageRemain;
		W25Q64_PageProgram(Address, Data, WriteLength);
		Address += WriteLength;
		Data += WriteLength;
		Length -= WriteLength;
	}
}

/***********************************************************
 * @brief     擦除包含指定地址的4KB扇区
 * @param     Address 扇区内任意地址
 * @return    无
 * @example   W25Q64_EraseSector(0x001000);
 * @note      擦除会把整个扇区恢复为0xFF，并阻塞等待操作完成
 ****************************************************************/
void W25Q64_EraseSector(uint32_t Address)
{
	W25Q64_WriteEnable();
	W25Q64_Select();
	BSP_SPI_TransferByte(W25Q64_Device->SPI, W25Q64_CMD_SECTOR_ERASE_4KB);
	W25Q64_SendAddress(Address);
	W25Q64_Deselect();
	W25Q64_WaitWhileBusy();
}

/***********************************************************
 * @brief     擦除包含指定地址的32KB块
 * @param     Address 32KB块内任意地址
 * @return    无
 * @example   W25Q64_EraseBlock32KB(0x008000);
 * @note      擦除会把整个32KB块恢复为0xFF，并阻塞等待操作完成
 ****************************************************************/
void W25Q64_EraseBlock32KB(uint32_t Address)
{
	W25Q64_WriteEnable();
	W25Q64_Select();
	BSP_SPI_TransferByte(W25Q64_Device->SPI, W25Q64_CMD_BLOCK_ERASE_32KB);
	W25Q64_SendAddress(Address);
	W25Q64_Deselect();
	W25Q64_WaitWhileBusy();
}

/***********************************************************
 * @brief     擦除包含指定地址的64KB块
 * @param     Address 64KB块内任意地址
 * @return    无
 * @example   W25Q64_EraseBlock64KB(0x010000);
 * @note      擦除会把整个64KB块恢复为0xFF，并阻塞等待操作完成
 ****************************************************************/
void W25Q64_EraseBlock64KB(uint32_t Address)
{
	W25Q64_WriteEnable();
	W25Q64_Select();
	BSP_SPI_TransferByte(W25Q64_Device->SPI, W25Q64_CMD_BLOCK_ERASE_64KB);
	W25Q64_SendAddress(Address);
	W25Q64_Deselect();
	W25Q64_WaitWhileBusy();
}

/***********************************************************
 * @brief     擦除W25Q64整片存储空间
 * @param     无
 * @return    无
 * @example   W25Q64_EraseChip();
 * @note      此操作会删除全部数据且耗时较长，函数会阻塞等待完成
 ****************************************************************/
void W25Q64_EraseChip(void)
{
	W25Q64_WriteEnable();
	W25Q64_SendCommand(W25Q64_CMD_CHIP_ERASE);
	W25Q64_WaitWhileBusy();
}

/***********************************************************
 * @brief     使W25Q64进入掉电模式
 * @param     无
 * @return    无
 * @example   W25Q64_PowerDown();
 * @note      进入掉电前应确保芯片不处于编程或擦除忙状态
 ****************************************************************/
void W25Q64_PowerDown(void)
{
	W25Q64_WaitWhileBusy();
	W25Q64_SendCommand(W25Q64_CMD_POWER_DOWN);
	Delay_us(3);
}

/***********************************************************
 * @brief     唤醒处于掉电模式的W25Q64
 * @param     无
 * @return    无
 * @example   W25Q64_WakeUp();
 * @note      发送释放掉电指令后等待芯片恢复到可访问状态
 ****************************************************************/
void W25Q64_WakeUp(void)
{
	W25Q64_SendCommand(W25Q64_CMD_RELEASE_POWER_DOWN);
	Delay_us(3);
}
