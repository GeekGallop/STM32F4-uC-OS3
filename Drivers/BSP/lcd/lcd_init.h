#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "sys.h"
#include "os.h"

extern OS_MUTEX LCD_Mutex;
#define USE_HORIZONTAL 1 //���ú�������������ʾ 0��1Ϊ���� 2��3Ϊ����


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 128
#define LCD_H 160

#else
#define LCD_W 160
#define LCD_H 128
#endif


//-----------------LCD�˿ڶ���---------------- 

#define LCD_SCLK_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_13)//SCL=SCLK=SCK
#define LCD_SCLK_Set() GPIO_SetBits(GPIOB,GPIO_Pin_13)

#define LCD_MOSI_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_15)//SDA=MOSI=SDI
#define LCD_MOSI_Set() GPIO_SetBits(GPIOB,GPIO_Pin_15)

#define LCD_RES_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_14)//RES=RST
#define LCD_RES_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_14)

#define LCD_DC_Clr()   GPIO_ResetBits(GPIOC,GPIO_Pin_6)//DC=RS
#define LCD_DC_Set()   GPIO_SetBits(GPIOC,GPIO_Pin_6)
 		     
#define LCD_CS_Clr()   GPIO_ResetBits(GPIOB,GPIO_Pin_12)//CS
#define LCD_CS_Set()   GPIO_SetBits(GPIOB,GPIO_Pin_12)

#define LCD_BLK_Clr()  GPIO_ResetBits(GPIOC,GPIO_Pin_7)//BLK
#define LCD_BLK_Set()  GPIO_SetBits(GPIOC,GPIO_Pin_7)

void LCD_GPIO_Init(void);//��ʼ��GPIO
void LCD_Writ_Bus(u8 dat);//ģ��SPIʱ��
void LCD_WR_DATA8(u8 dat);//д��һ���ֽ�
void LCD_WR_DATA(u16 dat);//д�������ֽ�
void LCD_WR_REG(u8 dat);//д��һ��ָ��
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//�������꺯��
void lcd_config(void);
void LCD_Init(void);//LCD��ʼ��
#endif




