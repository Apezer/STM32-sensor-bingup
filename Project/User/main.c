#include "stm32f10x.h"                  /* 设备头文件 */
#include "Delay.h"
#include "OLED.h"
#include "BSP_I2C.h"
#include "VL6180X.h"
#include <stdio.h>

static const BSP_I2C_TypeDef OLED_I2C =
{
	GPIOB, GPIO_Pin_8,           /* 时钟线SCL */
	GPIOB, GPIO_Pin_9,           /* 数据线SDA */
	RCC_APB2Periph_GPIOB         /* GPIO端口时钟 */
};

static const BSP_I2C_TypeDef VL6180X_I2C =
{
	GPIOB, GPIO_Pin_10,          /* 时钟线SCL */
	GPIOB, GPIO_Pin_11,          /* 数据线SDA */
	RCC_APB2Periph_GPIOB         /* GPIO端口时钟 */
};

/***********************************************************
 * @brief     初始化OLED和VL6180X，并持续显示测量距离
 * @param     无
 * @return    int 程序入口返回值，正常运行时不会返回
 * @example   main();
 * @note      当MODEL_ID不等于0xB4时，程序会停留在错误提示界面
 ****************************************************************/
int main(void)
{
	uint8_t VL6180X_ID;
	uint8_t Distance;
	char OLED_Line[17];

	OLED_Init(&OLED_I2C);
	VL6180X_Init(&VL6180X_I2C);
	VL6180X_ID = VL6180X_GetID();
	if (VL6180X_ID != VL6180X_MODEL_ID_VALUE)
	{
		OLED_ShowString(1, 1, "VL6180X ERROR   ");
		OLED_ShowString(2, 1, "PB10: SCL       ");
		OLED_ShowString(3, 1, "PB11: SDA       ");
		sprintf(OLED_Line, "ID:%02X ADDR:29   ", (unsigned int)VL6180X_ID);
		OLED_ShowString(4, 1, OLED_Line);
		OLED_Refresh();
		while (1);
	}

	OLED_Clear();
	
	while (1)
	{
		Distance = VL6180X_ReadDistance();
		OLED_ShowString(1, 1, "VL6180X TOF     ");
		sprintf(OLED_Line, "Distance:%3u mm ", (unsigned int)Distance);
		OLED_ShowString(2, 1, OLED_Line);
		OLED_ShowString(3, 1, "Range:0-100 mm  ");
		sprintf(OLED_Line, "ID:%02X ADDR:29   ", (unsigned int)VL6180X_ID);
		OLED_ShowString(4, 1, OLED_Line);
		OLED_Refresh();

		Delay_ms(50);
	}
}
