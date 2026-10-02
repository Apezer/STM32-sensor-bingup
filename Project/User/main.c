#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "BSP_I2C.h"
#include "MPU6050.h"
#include <stdio.h>

static const BSP_I2C_TypeDef MPU6050_I2C =
{
	GPIOB, GPIO_Pin_10,          /* SCL */
	GPIOB, GPIO_Pin_11,          /* SDA */
	RCC_APB2Periph_GPIOB,        /* GPIO clock */
	10                           /* Half-cycle delay in us */
};

/***********************************************************
 * @brief     Initializes the OLED and MPU6050, then continuously displays sensor data
 * @param     void
 * @return    int Program entry point return value, not reached during normal operation
 * @example   main();
 * @note      The program stops on the error screen when WHO_AM_I is not equal to 0x68
 ****************************************************************/
int main(void)
{
	MPU6050_DataTypeDef MPU6050_Data;
	uint8_t MPU6050_ID;
	int32_t Temperature;
	char OLED_Line[17];

	OLED_Init();
	MPU6050_Init(&MPU6050_I2C);
	MPU6050_ID = BSP_I2C_ReadReg(&MPU6050_I2C, 0x68, MPU6050_WHO_AM_I);
	if (MPU6050_ID != 0x68)
	{
		OLED_ShowString(1, 1, "MPU6050 ERROR   ");
		OLED_ShowString(2, 1, "PB10: SCL       ");
		OLED_ShowString(3, 1, "PB11: SDA       ");
		sprintf(OLED_Line, "ID:%02X           ", (unsigned int)MPU6050_ID);
		OLED_ShowString(4, 1, OLED_Line);
		OLED_Refresh();
		while (1);
	}

	OLED_Clear();
	
	while (1)
	{
		MPU6050_GetData(&MPU6050_Data);

		sprintf(OLED_Line, "AX%+05ld GX%+05ld ",
			(long)MPU6050_AccelRawToMg(MPU6050_Data.AccelX),
			(long)MPU6050_GyroRawToDeciDps(MPU6050_Data.GyroX));
		OLED_ShowString(1, 1, OLED_Line);

		sprintf(OLED_Line, "AY%+05ld GY%+05ld ",
			(long)MPU6050_AccelRawToMg(MPU6050_Data.AccelY),
			(long)MPU6050_GyroRawToDeciDps(MPU6050_Data.GyroY));
		OLED_ShowString(2, 1, OLED_Line);

		sprintf(OLED_Line, "AZ%+05ld GZ%+05ld ",
			(long)MPU6050_AccelRawToMg(MPU6050_Data.AccelZ),
			(long)MPU6050_GyroRawToDeciDps(MPU6050_Data.GyroZ));
		OLED_ShowString(3, 1, OLED_Line);

		Temperature = MPU6050_TemperatureRawToCentiDegree(MPU6050_Data.Temperature);
		sprintf(OLED_Line, "T:%+04ldC ID:%02X   ",
			(long)(Temperature / 100), MPU6050_ID);
		OLED_ShowString(4, 1, OLED_Line);
		OLED_Refresh();

		Delay_ms(100);
	}
}
