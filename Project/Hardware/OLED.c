#include "OLED_Font.h"
#include "OLED.h"
#include "BSP_I2C.h"
#include <string.h>
#include <stdarg.h>

static const BSP_I2C_TypeDef *OLED_I2C;

/* OLED显存缓冲区：8页，每页128字节，调用OLED_Refresh后才会更新屏幕。 */
uint8_t OLED_DisplayBuf[8][128];

/*****************************************************************
 * @brief     向SSD1306发送一个命令字节
 * @param     Command 要发送的SSD1306命令
 * @return    void
 * @example   OLED_WriteCommand(0xAE);
 * @note      控制字节0x00表示后续字节为命令
 ****************************************************************/
void OLED_WriteCommand(uint8_t Command)
{
	BSP_I2C_Start(OLED_I2C);
	BSP_I2C_SendByte(OLED_I2C, 0x78);
	BSP_I2C_ReceiveAck(OLED_I2C);
	BSP_I2C_SendByte(OLED_I2C, 0x00);
	BSP_I2C_ReceiveAck(OLED_I2C);
	BSP_I2C_SendByte(OLED_I2C, Command);
	BSP_I2C_ReceiveAck(OLED_I2C);
	BSP_I2C_Stop(OLED_I2C);
}

/*****************************************************************
 * @brief     向SSD1306连续写入显示数据
 * @param     Data  指向待发送数据缓冲区的指针
 * @param     Count 要发送的字节数
 * @return    void
 * @example   OLED_WriteData(OLED_DisplayBuf[0], 128);
 * @note      控制字节0x40表示后续字节写入GDDRAM
 ****************************************************************/
void OLED_WriteData(uint8_t *Data, uint8_t Count)
{
	uint8_t i;
	
	BSP_I2C_Start(OLED_I2C);
	BSP_I2C_SendByte(OLED_I2C, 0x78);
	BSP_I2C_ReceiveAck(OLED_I2C);
	BSP_I2C_SendByte(OLED_I2C, 0x40);
	BSP_I2C_ReceiveAck(OLED_I2C);
	for (i = 0; i < Count; i ++)
	{
		BSP_I2C_SendByte(OLED_I2C, Data[i]);
		BSP_I2C_ReceiveAck(OLED_I2C);
	}
	BSP_I2C_Stop(OLED_I2C);
}

/*****************************************************************
 * @brief     设置SSD1306的页地址和列地址
 * @param     Y 页地址，范围为0~7
 * @param     X 列地址，范围为0~127
 * @return    void
 * @example   OLED_SetCursor(0, 0);
 * @note      Y表示8像素高的页，不是0~63的像素坐标
 ****************************************************************/
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/*****************************************************************
 * @brief     将STM32显存缓冲区更新到OLED屏幕
 * @param     void
 * @return    void
 * @example   OLED_Refresh();
 * @note      函数会按页发送完整的1024字节显存数据
 ****************************************************************/
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

/*****************************************************************
 * @brief     清空STM32中的OLED显存缓冲区
 * @param     void
 * @return    void
 * @example   OLED_Clear();
 * @note      调用后还需执行OLED_Refresh才会更新屏幕
 ****************************************************************/
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



/*****************************************************************
 * @brief     在显存缓冲区中写入一个8x16 ASCII字符
 * @param     Line   显示行，范围为1~4
 * @param     Column 显示列，范围为1~16
 * @param     Char   要显示的ASCII可见字符
 * @return    void
 * @example   OLED_ShowChar(1, 1, 'A');
 * @note      调用后还需执行OLED_Refresh才会更新屏幕
 ****************************************************************/
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

/*****************************************************************
 * @brief     在显存缓冲区中写入ASCII字符串
 * @param     Line   起始行，范围为1~4
 * @param     Column 起始列，范围为1~16
 * @param     String 指向以'\0'结尾字符串的指针
 * @return    void
 * @example   OLED_ShowString(1, 1, "MPU6050");
 * @note      字符串不应超出屏幕右边界
 ****************************************************************/
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}



/*****************************************************************
 * @brief     计算X的Y次方
 * @param     X 底数
 * @param     Y 指数
 * @return    uint32_t X的Y次方
 * @example   Value = OLED_Pow(10, 3);
 * @note      该函数用于数字的逐位显示
 ****************************************************************/
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/*****************************************************************
 * @brief     在显存缓冲区中写入无符号十进制数字
 * @param     Line   起始行，范围为1~4
 * @param     Column 起始列，范围为1~16
 * @param     Number 要显示的无符号数值
 * @param     Length 显示位数，范围为1~10
 * @return    void
 * @example   OLED_ShowNum(1, 1, 1234, 4);
 * @note      位数不足时在左侧显示0
 ****************************************************************/
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/*****************************************************************
 * @brief     在显存缓冲区中写入带符号十进制数字
 * @param     Line   起始行，范围为1~4
 * @param     Column 起始列，范围为1~16
 * @param     Number 要显示的有符号数值
 * @param     Length 不包含符号位的数字位数
 * @return    void
 * @example   OLED_ShowSignedNum(1, 1, -123, 3);
 * @note      函数会在数字前显示'+'或'-'
 ****************************************************************/
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

/*****************************************************************
 * @brief     在显存缓冲区中写入十六进制数字
 * @param     Line   起始行，范围为1~4
 * @param     Column 起始列，范围为1~16
 * @param     Number 要显示的无符号数值
 * @param     Length 显示位数，范围为1~8
 * @return    void
 * @example   OLED_ShowHexNum(1, 1, 0x68, 2);
 * @note      字母A~F使用大写形式显示
 ****************************************************************/
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

/*****************************************************************
 * @brief     在显存缓冲区中写入二进制数字
 * @param     Line   起始行，范围为1~4
 * @param     Column 起始列，范围为1~16
 * @param     Number 要显示的无符号数值
 * @param     Length 显示位数，范围为1~16
 * @return    void
 * @example   OLED_ShowBinNum(1, 1, 0x0F, 8);
 * @note      位数不足时在左侧显示0
 ****************************************************************/
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}


/*****************************************************************
 * @brief     清空显存缓冲区中的指定矩形区域
 * @param     X      区域左上角X坐标，范围为0~127
 * @param     Y      区域左上角Y坐标，范围为0~63
 * @param     Width  区域宽度，范围为0~128
 * @param     Height 区域高度，范围为0~64
 * @return    void
 * @example   OLED_ClearArea(0, 0, 32, 16);
 * @note      超出屏幕的区域会被自动裁剪
 ****************************************************************/
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
/*****************************************************************
 * @brief     将二值图像写入OLED显存缓冲区
 * @param     X      图像左上角X坐标，范围为0~127
 * @param     Y      图像左上角Y坐标，范围为0~63
 * @param     Width  图像宽度，范围为0~128
 * @param     Height 图像高度，范围为0~64
 * @param     Image  指向图像数据的指针
 * @return    void
 * @example   OLED_ShowImage(0, 0, 16, 16, Image);
 * @note      图像数据按页存放，调用后需执行OLED_Refresh
 ****************************************************************/
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
/*****************************************************************
 * @brief     将16x16汉字串写入OLED显存缓冲区
 * @param     X       汉字串左上角X坐标，范围为0~127
 * @param     Y       汉字串左上角Y坐标，范围为0~63
 * @param     Chinese 指向以'\0'结尾的汉字字符串
 * @return    void
 * @example   OLED_ShowChinese(0, 0, "传感器");
 * @note      汉字必须已在OLED_CF16x16字模表中定义
 ****************************************************************/
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

/*****************************************************************
 * @brief     初始化OLED的I2C总线和SSD1306显示参数
 * @param     I2C 指向OLED所使用的软件I2C总线句柄
 * @return    void
 * @example   OLED_Init(&OLED_I2C);
 * @note      函数只清空STM32显存，需要OLED_Refresh才会更新屏幕
 ****************************************************************/
void OLED_Init(const BSP_I2C_TypeDef *I2C)
{
	uint32_t i, j;

	OLED_I2C = I2C;
	BSP_I2C_Init(OLED_I2C);

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
