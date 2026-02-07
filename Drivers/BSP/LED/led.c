#include "./LED/led.h"

/* 初始化PB5和PE5为输出口.并使能这两个口的时钟 */
/* LED IO初始化 */ 
void led_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);   /* 使能PA端口时钟 */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;               /* LED0-->PB.5 端口配置 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;           /* 推挽输出 */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;       /* IO口速度为50MHz */
    GPIO_Init(GPIOA, &GPIO_InitStructure);                  /* 根据设定参数初始化GPIOB.5 */
    GPIO_SetBits(GPIOA,GPIO_Pin_0);                         /* PB.5 输出高 */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;               /* LED1-->PE.5 端口配置, 推挽输出 */
    GPIO_Init(GPIOA, &GPIO_InitStructure);                  /* 推挽输出，IO口速度为50MHz */
    GPIO_SetBits(GPIOA,GPIO_Pin_1);                         /* PE.5 输出高  */
}
