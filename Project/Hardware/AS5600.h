#ifndef __AS5600_H
#define __AS5600_H

#include "stm32f10x.h"
#include "BSP_I2C.h"

/* AS5600使用固定的7位I2C地址0x36。 */
#define AS5600_ADDRESS_7BIT             0x36

/* 角度范围和永久配置寄存器。 */
#define AS5600_REG_ZMCO                 0x00
#define AS5600_REG_ZPOS_H               0x01
#define AS5600_REG_ZPOS_L               0x02
#define AS5600_REG_MPOS_H               0x03
#define AS5600_REG_MPOS_L               0x04
#define AS5600_REG_MANG_H               0x05
#define AS5600_REG_MANG_L               0x06
#define AS5600_REG_CONF_H               0x07
#define AS5600_REG_CONF_L               0x08

/* 角度、磁铁状态和磁场强度寄存器。 */
#define AS5600_REG_STATUS               0x0B
#define AS5600_REG_RAW_ANGLE_H          0x0C
#define AS5600_REG_RAW_ANGLE_L          0x0D
#define AS5600_REG_ANGLE_H              0x0E
#define AS5600_REG_ANGLE_L              0x0F
#define AS5600_REG_AGC                  0x1A
#define AS5600_REG_MAGNITUDE_H          0x1B
#define AS5600_REG_MAGNITUDE_L          0x1C
#define AS5600_REG_BURN                 0xFF

/* STATUS寄存器中的磁铁检测状态位。 */
#define AS5600_STATUS_MAGNET_HIGH       0x08
#define AS5600_STATUS_MAGNET_LOW        0x10
#define AS5600_STATUS_MAGNET_DETECTED   0x20

typedef struct
{
	uint16_t RawAngle;          /* 未经过起止位置缩放的12位原始角度。 */
	uint16_t Angle;             /* 经过芯片配置处理后的12位角度。 */
	uint16_t AngleCentiDegree;  /* Angle换算后的角度，单位0.01度。 */
	uint8_t Status;             /* 磁铁检测状态寄存器。 */
	uint8_t AGC;                /* 自动增益控制值。 */
	uint16_t Magnitude;         /* CORDIC计算得到的磁场幅值。 */
} AS5600_DataTypeDef;

void AS5600_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t AS5600_ReadReg(uint8_t RegAddress);
void AS5600_Init(const BSP_I2C_TypeDef *I2C);
uint16_t AS5600_ReadRawAngle(void);
uint16_t AS5600_ReadAngle(void);
uint16_t AS5600_RawToCentiDegree(uint16_t RawAngle);
void AS5600_GetData(AS5600_DataTypeDef *Data);

#endif
