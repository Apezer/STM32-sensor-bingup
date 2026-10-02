#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"                  // Device header


extern uint8_t OLED_DisplayBuf[8][128];
extern const uint8_t Diode[];
extern const uint8_t Kenan[];

void OLED_GPIO_Init(void);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_I2C_Start(void);
void OLED_I2C_Stop(void);
void OLED_I2C_SendByte(uint8_t Byte);

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
