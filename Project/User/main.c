#include "stm32f10x.h"                  /* 设备头文件 */
#include "Delay.h"
#include "OLED.h"
#include "BSP_I2C.h"
#include "BMP280.h"
#include <stdio.h>

static const BSP_I2C_TypeDef OLED_I2C =
{
	GPIOB, GPIO_Pin_8,           /* 时钟线SCL */
	GPIOB, GPIO_Pin_9,           /* 数据线SDA */
	RCC_APB2Periph_GPIOB         /* GPIO端口时钟 */
};

static const BSP_I2C_TypeDef BMP280_I2C =
{
	GPIOB, GPIO_Pin_10,          /* 时钟线SCL */
	GPIOB, GPIO_Pin_11,          /* 数据线SDA */
	RCC_APB2Periph_GPIOB         /* GPIO端口时钟 */
};

/***********************************************************
 * @brief     初始化OLED和BMP280，并持续显示温度、气压和海拔
 * @param     无
 * @return    int 程序入口返回值，正常运行时不会返回
 * @example   main();
 * @note      当芯片ID不等于0x58时，程序会停留在错误提示界面
 ****************************************************************/
int main(void)
{
	uint8_t BMP280_ID;
	BMP280_DataTypeDef BMP280_Data;
	uint32_t TemperatureAbs;
	uint32_t AltitudeAbs;
	char TemperatureSign;
	char AltitudeSign;
	char OLED_Line[17];

	OLED_Init(&OLED_I2C);
	BMP280_Init(&BMP280_I2C);
	BMP280_ID = BMP280_GetID();
	if (BMP280_ID != BMP280_CHIP_ID_VALUE)
	{
		OLED_ShowString(1, 1, "BMP280 ERROR    ");
		OLED_ShowString(2, 1, "PB10: SCL       ");
		OLED_ShowString(3, 1, "PB11: SDA       ");
		sprintf(OLED_Line, "ID:%02X ADDR:76   ", (unsigned int)BMP280_ID);
		OLED_ShowString(4, 1, OLED_Line);
		OLED_Refresh();
		while (1);
	}

	OLED_Clear();
	
	while (1)
	{
		BMP280_GetData(&BMP280_Data);
		TemperatureSign = (BMP280_Data.Temperature < 0) ? '-' : ' ';
		TemperatureAbs = (BMP280_Data.Temperature < 0) ?
			(uint32_t)(-BMP280_Data.Temperature) : (uint32_t)BMP280_Data.Temperature;
		AltitudeSign = (BMP280_Data.Altitude < 0) ? '-' : ' ';
		AltitudeAbs = (BMP280_Data.Altitude < 0) ?
			(uint32_t)(-BMP280_Data.Altitude) : (uint32_t)BMP280_Data.Altitude;

		OLED_ShowString(1, 1, "BMP280 BAROMETER");
		sprintf(OLED_Line, "T:%c%2lu.%02lu C   ", TemperatureSign,
			(unsigned long)(TemperatureAbs / 100),
			(unsigned long)(TemperatureAbs % 100));
		OLED_ShowString(2, 1, OLED_Line);
		sprintf(OLED_Line, "P:%4lu.%02lu hPa ",
			(unsigned long)(BMP280_Data.Pressure / 100),
			(unsigned long)(BMP280_Data.Pressure % 100));
		OLED_ShowString(3, 1, OLED_Line);
		sprintf(OLED_Line, "H:%c%4lu.%02lu m  ", AltitudeSign,
			(unsigned long)(AltitudeAbs / 100),
			(unsigned long)(AltitudeAbs % 100));
		OLED_ShowString(4, 1, OLED_Line);
		OLED_Refresh();

		Delay_ms(250);
	}
}
