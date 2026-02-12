//创建中断初始化函数
#include "os.h"
#include "exti.h"
#include "stm32f4xx_gpio.h"
#include "misc.h"
#include "stm32f4xx_syscfg.h"
#include "./LED/led.h"
//中断配置步骤：
//1.配置时钟
//2.配置中断对应引脚
//3.配置连接引脚到对应接线
//4.配置优先级
//5.配置nvic
void exti_config(void)
{   
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//使能gpio时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);//使能gpio时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);//配置后续nvic时钟

    GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType=GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;//输入下拉
    GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;
    GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType=GPIO_OType_OD;
    GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_DOWN;//输入上拉
    GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOE,&GPIO_InitStruct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);//将gpio映射到对应中断线
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource2);//将gpio映射到对应中断线
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource3);//将gpio映射到对应中断线
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource4);//将gpio映射到对应中断线

    EXTI_DeInit();
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_LineCmd=ENABLE;
    EXTI_Init(&EXTI_InitStructure);//exti初始化

    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_LineCmd=ENABLE;
    EXTI_Init(&EXTI_InitStructure);//exti初始化

    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_LineCmd=ENABLE;
    EXTI_Init(&EXTI_InitStructure);//exti初始化

    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_LineCmd=ENABLE;
    EXTI_Init(&EXTI_InitStructure);//exti初始化

    NVIC_InitStruct.NVIC_IRQChannel=EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority=3;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel=EXTI2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel=EXTI3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority=1;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel=EXTI4_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
    NVIC_Init(&NVIC_InitStruct);
}


void EXTI0_IRQHandler(void)
{
    static uint32_t last_time=0;
    OS_ERR err;
    OSIntEnter();
    if(EXTI_GetITStatus(EXTI_Line0)!=RESET)
    {   
        uint32_t current_time=OSTimeGet(&err);//软件消抖

        if(current_time-last_time>50)
        {
            LED0=!LED0;
        }
        last_time=current_time;
        EXTI_ClearITPendingBit(EXTI_Line0);//清理中断标志位
    }
       OSIntExit();
}

void EXTI2_IRQHandler(void)
{

    OSIntEnter();
    if(EXTI_GetITStatus(EXTI_Line2)!=RESET)
    {

         EXTI_ClearITPendingBit(EXTI_Line2);//清理中断标志位
    }
       OSIntExit();
}

void EXTI3_IRQHandler(void)
{
    OSIntEnter();
    if(EXTI_GetITStatus(EXTI_Line3)!=RESET)
    {

         EXTI_ClearITPendingBit(EXTI_Line3);//清理中断标志位
    }
       OSIntExit();
}

void EXTI4_IRQHandler(void)
{
    OSIntEnter();
    if(EXTI_GetITStatus(EXTI_Line4)!=RESET)
    {


         EXTI_ClearITPendingBit(EXTI_Line4);//清理中断标志位
    }
       OSIntExit();
}
