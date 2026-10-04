#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"                  // Device header
#include "BSP_I2C.h"


extern uint8_t OLED_DisplayBuf[8][128];
extern const uint8_t Diode[];
extern const uint8_t Kenan[];

void OLED_WriteCommand(uint8_t Command);
void OLED_WriteData(uint8_t *Data, uint8_t Count);

void OLED_Init(const BSP_I2C_TypeDef *I2C);
void OLED_Clear(void);

void OLED_Refresh(void);
void OLED_ClearArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

void OLED_ShowImage(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
void OLED_ShowChinese(uint8_t X, uint8_t Y, char *Chinese);

#endif
