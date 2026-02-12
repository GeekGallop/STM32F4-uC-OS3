#include "task.h"
#include "os.h"
#include "./LED/led.h"
#include "./key/key.h"
#include "./usart/usart.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#define START_TASK_PRIO 2                               /* 任务优先级 */
#define START_STK_SIZE  512                             /* 任务栈大小 */
OS_TCB                  StartTask_TCB;                  /* 任务控制块 */
CPU_STK                 StartTask_STK[START_STK_SIZE];  /* 任务栈 */
void start_task(void *p_arg);                           /* 任务函数 */

/* TASK1 任务 配置
 * 包括: 任务优先级 任务栈大小 任务控制块 任务栈 任务函数
 */
#define TASK1_PRIO      (OS_CFG_PRIO_MAX - 4)           /* 任务优先级 */
#define TASK1_STK_SIZE  1024                            /* 任务栈大小 */
OS_TCB                  Task1_TCB;                      /* 任务控制块 */
CPU_STK                 Task1_STK[TASK1_STK_SIZE];      /* 任务栈 */
void task1(void *p_arg);                                /* 任务函数 */

/* TASK2 任务 配置
 * 包括: 任务优先级 任务栈大小 任务控制块 任务栈 任务函数
 */
#define TASK2_PRIO      (OS_CFG_PRIO_MAX - 5)          
#define TASK2_STK_SIZE  1024                            /* 任务栈大小 */
OS_TCB                  Task2_TCB;                      /* 任务控制块 */
CPU_STK                 Task2_STK[TASK2_STK_SIZE];      /* 任务栈 */
void task2(void *p_arg);                                /* 任务函数 */

/* TASK3 任务 配置
 * 包括: 任务优先级 任务栈大小 任务控制块 任务栈 任务函数
 */
#define TASK3_PRIO      (OS_CFG_PRIO_MAX - 6)           /* 2. 修改：原和TASK1同优先级，改为-5避免冲突 */
#define TASK3_STK_SIZE  1024                            /* 任务栈大小 */
OS_TCB                  Task3_TCB;                      /* 任务控制块 */
CPU_STK                 Task3_STK[TASK3_STK_SIZE];      /* 任务栈 */
void task3(void *p_arg);                                /* 任务函数 */


#define PROTOCOL_PRIO    (OS_CFG_PRIO_MAX - 7)
#define PROTOCOL_STK_SIZE  2048
OS_TCB                  Protocol_Task_TCB;
CPU_STK                 Protocol_Task_STK[PROTOCOL_STK_SIZE];
void Protocol_Task(void*p_arg);

void app_start(void)
{
    OS_ERR err;//错误句柄
    
    /* 初始化uC/OS-III */
    OSInit(&err);
    
    /* 创建Start Task */
    OSTaskCreate(   (OS_TCB        *)&StartTask_TCB,
                    (CPU_CHAR      *)"start_task",
                    (OS_TASK_PTR    )start_task,
                    (void          *)0,
                    (OS_PRIO        )START_TASK_PRIO,
                    (CPU_STK       *)&StartTask_STK[0],
                    (CPU_STK_SIZE   )START_STK_SIZE / 10,
                    (CPU_STK_SIZE   )START_STK_SIZE,
                    (OS_MSG_QTY     )0,
                    (OS_TICK        )0,
                    (void          *)0,
                    (OS_OPT         )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                    (OS_ERR        *)&err);
    
    /* 开始任务调度 */
    OSStart(&err);
    
    for (;;)
    {
        /* 不会进入这里 */
    }
}

/**
 * @brief       start_task
 * @param       p_arg : 传入参数(未用到)
 * @retval      无
 */
void start_task(void *p_arg)
{
    OS_ERR err;
    CPU_INT32U cnts;
    RCC_ClocksTypeDef  rcc_clocks;
    
    /* 初始化CPU库 */
    CPU_Init();
    
    /* 根据配置的节拍频率配置SysTick */
    RCC_GetClocksFreq(&rcc_clocks);                                     /* 获取各个时钟频率 */
    cnts = ((CPU_INT32U)rcc_clocks.HCLK_Frequency) / OSCfg_TickRate_Hz; /* 返回HCLK时钟频率 */
    OS_CPU_SysTickInit(cnts);
    
    /* 开启时间片调度，时间片设为默认值 */
    OSSchedRoundRobinCfg(OS_TRUE, 10, &err);
    
    /* 创建Task1 */
    OSTaskCreate(   (OS_TCB        *)&Task1_TCB,
                    (CPU_CHAR      *)"task1",
                    (OS_TASK_PTR    )task1,
                    (void          *)0,
                    (OS_PRIO        )TASK1_PRIO,
                    (CPU_STK       *)&Task1_STK[0],
                    (CPU_STK_SIZE   )TASK1_STK_SIZE / 10,
                    (CPU_STK_SIZE   )TASK1_STK_SIZE,
                    (OS_MSG_QTY     )0,
                    (OS_TICK        )0,
                    (void          *)10,
                    (OS_OPT         )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                    (OS_ERR        *)&err);
    
    /* 创建Task2 */
    OSTaskCreate(   (OS_TCB        *)&Task2_TCB,
                    (CPU_CHAR      *)"task2",
                    (OS_TASK_PTR    )task2,
                    (void          *)0,
                    (OS_PRIO        )TASK2_PRIO,
                    (CPU_STK       *)&Task2_STK[0],
                    (CPU_STK_SIZE   )TASK2_STK_SIZE / 10,
                    (CPU_STK_SIZE   )TASK2_STK_SIZE,
                    (OS_MSG_QTY     )0,
                    (OS_TICK        )10,
                    (void          *)10,
                    (OS_OPT         )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                    (OS_ERR        *)&err);
    
		/* 创建Task3 */
    OSTaskCreate(   (OS_TCB        *)&Task3_TCB,
                    (CPU_CHAR      *)"task3",
                    (OS_TASK_PTR    )task3,
                    (void          *)0,
                    (OS_PRIO        )TASK3_PRIO,
                    (CPU_STK       *)&Task3_STK[0],
                    (CPU_STK_SIZE   )TASK3_STK_SIZE / 10,
                    (CPU_STK_SIZE   )TASK3_STK_SIZE,
                    (OS_MSG_QTY     )0,
                    (OS_TICK        )10,
                    (void          *)10,
                    (OS_OPT         )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                    (OS_ERR        *)&err);


    OSTaskCreate(   (OS_TCB        *)&Protocol_Task_TCB,
                    (CPU_CHAR      *)"Protocol_Task",
                    (OS_TASK_PTR    )Protocol_Task,
                    (void          *)0,
                    (OS_PRIO        )PROTOCOL_PRIO,
                    (CPU_STK       *)&Protocol_Task_STK[0],
                    (CPU_STK_SIZE   )PROTOCOL_STK_SIZE/ 10,
                    (CPU_STK_SIZE   )PROTOCOL_STK_SIZE,
                    (OS_MSG_QTY     )0,
                    (OS_TICK        )10,
                    (void          *)0,
                    (OS_OPT         )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                    (OS_ERR        *)&err);
														
    OSTaskDel((OS_TCB *)0, &err);
}

/**
 * @brief       task1
 * @param       p_arg : 传入参数(未用到)
 * @retval      无
 */
void task1(void *p_arg)
{
    OS_ERR err;
    
    while(1)
    {
        LED0 = !LED0;
        OSTimeDly(500, OS_OPT_TIME_DLY, &err);          /* 延时500ticks */
    }
}

/**
 * @brief       task2
 * @param       p_arg : 传入参数(未用到)
 * @retval      无
 */
void task2(void *p_arg)
{
    OS_ERR err;
    
    while(1)
    {
        LED1 = !LED1;
        
        OSTimeDly(1000, OS_OPT_TIME_DLY, &err);         /* 延时1000ticks */
    }
}


void task3(void *p_arg)
{
		OS_ERR err;
		while(1)
		{
			if(KEY_Scan(0)==1)
			{
				//OSTaskDel(&Task2_TCB,&err);
                OSTaskSuspend(&Task2_TCB,&err);//任务挂起
            }
            if(KEY_Scan(1)==2)
            {
                OSTaskResume(&Task2_TCB,&err);//恢复任务
            }
			OSTimeDly(1000, OS_OPT_TIME_DLY, &err);         /* 延时1000ticks */
		}
	
}

void Protocol_Task(void*p_arg)
{
    (void*)p_arg;
    OS_ERR err;
    static uint8_t rx_state = 0;      // 状态机：0=等待包头，1=接收数据
    char rx_buffer[100];    // 接收缓冲区（任务私有）
    uint8_t rx_index = 0;      // 接收位置
    while(1)
    {
        void* msg=OSQPend(&USART_Rx_Queue,100,OS_OPT_PEND_BLOCKING,NULL,NULL,&err);
        if(err==OS_ERR_NONE)//接收到信号了
        {
            uint8_t rx_data=(uint8_t)(uintptr_t)msg;
            switch(rx_state)
            {
                case 0:
                {
                    if(rx_data=='[')
                    {
                        rx_state=1;
                        rx_index=0;
                    }
                }
                break;

                case 1:
                {
                    if(rx_data==']')
                    {
                        rx_state=0;
                        //此处进行处理    
                        char*str1=strtok(rx_buffer,",");
				        char*str2=strtok(NULL,",");
				        char*str3=strtok(NULL,",");
				        char*str4=strtok(NULL,",");

                        if (strcmp(str1, "PID") == 0)
                       {
                            printf("%f,%f,%f\r\n",atof(str2),atof(str3),atof(str4));
                       }


                    }
                    else  if(rx_index<=sizeof(rx_buffer))
                    {
                        rx_buffer[++rx_index]=rx_data;
                    }
                    else
                    {
                        rx_state=0;
                    }
                }
                break;
            }
        }
				OSTimeDly(10, OS_OPT_TIME_DLY, &err);         /* 延时1000ticks */

    }

}

