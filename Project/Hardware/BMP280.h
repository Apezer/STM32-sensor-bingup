#ifndef __BMP280_H
#define __BMP280_H

#include "stm32f10x.h"
#include "BSP_I2C.h"

/* SDO接地时，BMP280的7位I2C地址为0x76。 */
#define BMP280_ADDRESS_7BIT              0x76
#define BMP280_CHIP_ID_VALUE             0x58
#define BMP280_SEA_LEVEL_PRESSURE_PA     101325UL

/* 出厂温度、气压补偿参数寄存器。 */
#define BMP280_REG_DIG_T1                0x88
#define BMP280_REG_DIG_T2                0x8A
#define BMP280_REG_DIG_T3                0x8C
#define BMP280_REG_DIG_P1                0x8E
#define BMP280_REG_DIG_P2                0x90
#define BMP280_REG_DIG_P3                0x92
#define BMP280_REG_DIG_P4                0x94
#define BMP280_REG_DIG_P5                0x96
#define BMP280_REG_DIG_P6                0x98
#define BMP280_REG_DIG_P7                0x9A
#define BMP280_REG_DIG_P8                0x9C
#define BMP280_REG_DIG_P9                0x9E

/* 芯片控制和测量数据寄存器。 */
#define BMP280_REG_CHIP_ID               0xD0
#define BMP280_REG_RESET                 0xE0
#define BMP280_REG_STATUS                0xF3
#define BMP280_REG_CTRL_MEAS             0xF4
#define BMP280_REG_CONFIG                0xF5
#define BMP280_REG_PRESS_MSB             0xF7
#define BMP280_REG_PRESS_LSB             0xF8
#define BMP280_REG_PRESS_XLSB            0xF9
#define BMP280_REG_TEMP_MSB              0xFA
#define BMP280_REG_TEMP_LSB              0xFB
#define BMP280_REG_TEMP_XLSB             0xFC

#define BMP280_RESET_VALUE               0xB6
#define BMP280_STATUS_MEASURING          0x08
#define BMP280_STATUS_IM_UPDATE          0x01

typedef struct
{
	int32_t Temperature;       /* 温度，单位0.01摄氏度。 */
	uint32_t Pressure;         /* 气压，单位Pa。 */
	int32_t Altitude;          /* 海拔，单位cm。 */
} BMP280_DataTypeDef;

void BMP280_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t BMP280_ReadReg(uint8_t RegAddress);
void BMP280_Init(const BSP_I2C_TypeDef *I2C);
uint8_t BMP280_GetID(void);
void BMP280_GetData(BMP280_DataTypeDef *Data);
int32_t BMP280_PressureToAltitudeCm(uint32_t PressurePa,
	uint32_t SeaLevelPressurePa);

#endif
