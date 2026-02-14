#ifndef __LCD_H
#define __LCD_H		
#include "sys.h"
#include "os.h"
#include "stddef.h"
/* ==================== 私有宏定义 ==================== */
extern OS_MUTEX LCD_Mutex; 
/**
 * @brief  LCD互斥锁操作宏（简化代码）
 */
#define LCD_MUTEX_PEND()  do { \
    OS_ERR err; \
    OSMutexPend(&LCD_Mutex, 0, OS_OPT_PEND_BLOCKING, NULL, &err); \
} while(0)

#define LCD_MUTEX_POST()  do { \
    OS_ERR err; \
    OSMutexPost(&LCD_Mutex, OS_OPT_POST_NONE, &err); \
} while(0)

/* ==================== 初始化函数 ==================== */


void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color);//ָ�����������ɫ
void LCD_DrawPoint(u16 x,u16 y,u16 color);//��ָ��λ�û�һ����
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color);//��ָ��λ�û�һ����
void LCD_ShowChinese(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);//��ʾ���ִ�
void LCD_ShowChinese12x12(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);//��ʾ����12x12����
void LCD_ShowChinese16x16(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);//��ʾ����16x16����
void LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);//��ʾ����24x24����
void LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);//��ʾ����32x32����
void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode);//��ʾһ���ַ�
void LCD_ShowString(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 sizey,u8 mode);//��ʾ�ַ���
void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[]);//��ʾͼƬ
void LCD_Show_Euler_Simple(float roll, float pitch, float yaw);



//������ɫ
#define COLOR_YELLOW_MAIN  0xFFE0  // ������/�����ᣨ���ƣ�
#define COLOR_YELLOW_SUB   0xF79E  // ������ǳ�ƣ�
#define COLOR_BACKGROUND   0x0000  // ��������ɫ��

// �����������
#define X_MAIN_INTERVAL 20   // X����������
#define X_SUB_INTERVAL  4    // X���������
#define Y_MAIN_INTERVAL 16   // Y����������
#define Y_SUB_INTERVAL  4    // Y���������

// ��������
#define WIDTH_MAIN  2
#define WIDTH_SUB   1
#define WIDTH_AXIS  3
// LCD�ֱ��ʶ���
#define LCD_WIDTH  160  // ����X��0~159
#define LCD_HEIGHT 128  // ����Y��0~127





#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 			       0XFFE0
#define GBLUE			       0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			     0XBC40 //��ɫ
#define BRRED 			     0XFC07 //�غ�ɫ
#define GRAY  			     0X8430 //��ɫ
#define DARKBLUE      	 0X01CF	//����ɫ
#define LIGHTBLUE      	 0X7D7C	//ǳ��ɫ  
#define GRAYBLUE       	 0X5458 //����ɫ
#define LIGHTGREEN     	 0X841F //ǳ��ɫ
#define LGRAY 			     0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ
#define LGRAYBLUE        0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE           0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)

#endif





