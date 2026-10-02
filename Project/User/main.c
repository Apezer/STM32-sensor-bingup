#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "BSP_I2C.h"
#include "MPU6050.h"

static const BSP_I2C_TypeDef MPU6050_I2C =
{
	GPIOB, GPIO_Pin_10,          /* SCL */
	GPIOB, GPIO_Pin_11,          /* SDA */
	RCC_APB2Periph_GPIOB,        /* GPIO clock */
	10                           /* Half-cycle delay in us */
};

/***********************************************************
 * @brief     Displays one acceleration axis and one angular-rate axis on the OLED
 * @param     Line        OLED row number
 * @param     Axis        Axis label, normally X, Y, or Z
 * @param     AccelMg     Acceleration in milligravity
 * @param     GyroDeciDps Angular rate in 0.1 degrees per second
 * @return    void
 * @example   OLED_ShowAxis(1, 'X', AccelX, GyroX);
 * @note      The displayed values are converted before this function is called
 ****************************************************************/
static void OLED_ShowAxis(uint8_t Line, char Axis,
	int32_t AccelMg, int32_t GyroDeciDps)
{
	OLED_ShowChar(Line, 1, 'A');
	OLED_ShowChar(Line, 2, Axis);
	OLED_ShowSignedNum(Line, 3, AccelMg, 4);
	OLED_ShowString(Line, 8, " G");
	OLED_ShowChar(Line, 10, Axis);
	OLED_ShowSignedNum(Line, 11, GyroDeciDps, 4);
}

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

	OLED_Init();
	MPU6050_Init(&MPU6050_I2C);
	MPU6050_ID = BSP_I2C_ReadReg(&MPU6050_I2C, 0x68, MPU6050_WHO_AM_I);
	if (MPU6050_ID != 0x68)
	{
		OLED_ShowString(1, 1, "MPU6050 ERROR   ");
		OLED_ShowString(2, 1, "PB10: SCL       ");
		OLED_ShowString(3, 1, "PB11: SDA       ");
		OLED_ShowString(4, 1, "ID:");
		OLED_ShowHexNum(4, 4, MPU6050_ID, 2);
		while (1);
	}

	OLED_Clear();
	
	while (1)
	{
		MPU6050_GetData(&MPU6050_Data);
		OLED_ShowAxis(1, 'X', MPU6050_AccelRawToMg(MPU6050_Data.AccelX),
			MPU6050_GyroRawToDeciDps(MPU6050_Data.GyroX));
		OLED_ShowAxis(2, 'Y', MPU6050_AccelRawToMg(MPU6050_Data.AccelY),
			MPU6050_GyroRawToDeciDps(MPU6050_Data.GyroY));
		OLED_ShowAxis(3, 'Z', MPU6050_AccelRawToMg(MPU6050_Data.AccelZ),
			MPU6050_GyroRawToDeciDps(MPU6050_Data.GyroZ));

		Temperature = MPU6050_TemperatureRawToCentiDegree(MPU6050_Data.Temperature);
		OLED_ShowString(4, 1, "T:");
		OLED_ShowSignedNum(4, 3, Temperature / 100, 3);
		OLED_ShowString(4, 7, "C ID:");
		OLED_ShowHexNum(4, 12, MPU6050_ID, 2);
		OLED_ShowString(4, 14, "   ");

		Delay_ms(100);
	}
}
