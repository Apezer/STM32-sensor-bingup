#include "OLED_Font.h"
#include "OLED.h"
#include <string.h>
#include <stdarg.h>

#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))
#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))


/**
  * OLED显存数组
  * 所有的显示函数，都只是对此显存数组进行读写
  * 随后调用OLED_Update函数或OLED_UpdateArea函数
  * 才会将显存数组的数据发送到OLED硬件，进行显示
  */
uint8_t OLED_DisplayBuf[8][128];

void OLED_GPIO_Init(void)
{
	uint32_t i, j;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Wait for the OLED power supply to stabilize. */
	for (i = 0; i < 1000; i ++)
	{
		for (j = 0; j < 1000; j ++);
	}

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;

	for (i = 0; i < 8; i ++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);
	OLED_W_SCL(0);
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);
	OLED_I2C_SendByte(0x00);
	OLED_I2C_SendByte(Command);
	OLED_I2C_Stop();
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t *Data, uint8_t Count)
{
	uint8_t i;
	
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);
	OLED_I2C_SendByte(0x40);
	for (i = 0; i < Count; i ++)
	{
		OLED_I2C_SendByte(Data[i]);
	}
	OLED_I2C_Stop();
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  *       0             X轴           127 
  *      .------------------------------->
  *    0 |
  *      |
  *      |
  *      |
  *  Y轴 |
  *      |
  *      |
  *      |
  *   63 |
  *      v
  * 
  * @retval OLED默认的Y轴，只能8个Bit为一组写入，即1页等于8个Y轴坐标
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/**
  * 函    数：将OLED显存数组更新到OLED屏幕
  * 参    数：无
  * 返 回 值：无
  * 说    明：所有的显示函数，都只是对OLED显存数组进行读写
  *           随后调用OLED_Update函数或OLED_UpdateArea函数
  *           才会将显存数组的数据发送到OLED硬件，进行显示
  *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_Refresh(void)
{
	uint8_t j;
	/*遍历每一页*/
	for (j = 0; j < 8; j ++)
	{
		/*设置光标位置为每一页的第一列*/
		OLED_SetCursor(j, 0);
		/*连续写入128个数据，将显存数组的数据写入到OLED硬件*/
		OLED_WriteData(OLED_DisplayBuf[j] , 128);
	}
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j ++)				//遍历8页
	{
		for (i = 0; i < 128; i ++)			//遍历128列
		{
			OLED_DisplayBuf[j][i] = 0x00;	//将显存数组数据全部清零
		}
	}
}



/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_DisplayBuf[(Line - 1) * 2][(Column - 1) * 8 + i] = OLED_F8x16[Char - ' '][i];
	}
	for (i = 0; i < 8; i++)
	{
		OLED_DisplayBuf[(Line - 1) * 2 + 1][(Column - 1) * 8 + i] = OLED_F8x16[Char - ' '][i + 8];
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}



/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-2147483648~2147483647
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十六进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval 无
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
  * @brief  OLED显示数字（二进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}


 /**
  * 函    数：将OLED显存数组部分清零
  * 参    数：X 指定区域左上角的横坐标，范围：0~127
  * 参    数：Y 指定区域左上角的纵坐标，范围：0~63
  * 参    数：Width 指定区域的宽度，范围：0~128
  * 参    数：Height 指定区域的高度，范围：0~64
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ClearArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height)
{
	uint8_t i, j;
	
	/*参数检查，保证指定区域不会超出屏幕范围*/
	if (X > 127) {return;}
	if (Y > 63) {return;}
	if (X + Width > 128) {Width = 128 - X;}
	if (Y + Height > 64) {Height = 64 - Y;}
	
	for (j = Y; j < Y + Height; j ++)		//遍历指定页
	{
		for (i = X; i < X + Width; i ++)	//遍历指定列
		{
			OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));	//将显存数组指定数据清零
		}
	}
}
/**
  * 函    数：OLED显示图像
  * 参    数：X 指定图像左上角的横坐标，范围：0~127
  * 参    数：Y 指定图像左上角的纵坐标，范围：0~63
  * 参    数：Width 指定图像的宽度，范围：0~128
  * 参    数：Height 指定图像的高度，范围：0~64
  * 参    数：Image 指定要显示的图像
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowImage(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
	uint8_t i, j;
	
	/*参数检查，保证指定图像不会超出屏幕范围*/
	if (X > 127) {return;}
	if (Y > 63) {return;}
	
	/*将图像所在区域清空*/
	OLED_ClearArea(X, Y, Width, Height);
	
	/*遍历指定图像涉及的相关页*/
	/*(Height - 1) / 8 + 1的目的是Height / 8并向上取整*/
	for (j = 0; j < (Height - 1) / 8 + 1; j ++)
	{
		/*遍历指定图像涉及的相关列*/
		for (i = 0; i < Width; i ++)
		{
			/*超出边界，则跳过显示*/
			if (X + i > 127) {break;}
			if (Y / 8 + j > 7) {return;}
			
			/*显示图像在当前页的内容*/
			OLED_DisplayBuf[Y / 8 + j][X + i] |= Image[j * Width + i] << (Y % 8);
			
			/*超出边界，则跳过显示*/
			/*使用continue的目的是，下一页超出边界时，上一页的后续内容还需要继续显示*/
			if (Y / 8 + j + 1 > 7) {continue;}
			
			/*显示图像在下一页的内容*/
			OLED_DisplayBuf[Y / 8 + j + 1][X + i] |= Image[j * Width + i] >> (8 - Y % 8);
		}
	}
}
/**
  * 函    数：OLED显示汉字串
  * 参    数：X 指定汉字串左上角的横坐标，范围：0~127
  * 参    数：Y 指定汉字串左上角的纵坐标，范围：0~63
  * 参    数：Chinese 指定要显示的汉字串，范围：必须全部为汉字或者全角字符，不要加入任何半角字符
  *           显示的汉字需要在OLED_Data.c里的OLED_CF16x16数组定义
  *           未找到指定汉字时，会显示默认图形（一个方框，内部一个问号）
  * 返 回 值：无
  * 说    明：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
  */
void OLED_ShowChinese(uint8_t X, uint8_t Y, char *Chinese)
{
	uint8_t pChinese = 0;
	uint8_t pIndex;
	uint8_t i;
	char SingleChinese[OLED_CHN_CHAR_WIDTH + 1] = {0};
	
	for (i = 0; Chinese[i] != '\0'; i ++)		//遍历汉字串
	{
		SingleChinese[pChinese] = Chinese[i];	//提取汉字串数据到单个汉字数组
		pChinese ++;							//计次自增
		
		/*当提取次数到达OLED_CHN_CHAR_WIDTH时，即代表提取到了一个完整的汉字*/
		if (pChinese >= OLED_CHN_CHAR_WIDTH)
		{
			pChinese = 0;		//计次归零
			
			/*遍历整个汉字字模库，寻找匹配的汉字*/
			/*如果找到最后一个汉字（定义为空字符串），则表示汉字未在字模库定义，停止寻找*/
			for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex ++)
			{
				/*找到匹配的汉字*/
				if (strcmp(OLED_CF16x16[pIndex].Index, SingleChinese) == 0)
				{
					break;		//跳出循环，此时pIndex的值为指定汉字的索引
				}
			}
			
			/*将汉字字模库OLED_CF16x16的指定数据以16*16的图像格式显示*/
			OLED_ShowImage(X + ((i + 1) / OLED_CHN_CHAR_WIDTH - 1) * 16, Y, 16, 16, OLED_CF16x16[pIndex].Data);
		}
	}
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	uint32_t i, j;

	OLED_GPIO_Init();

	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}

	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
}

