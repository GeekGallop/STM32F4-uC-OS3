#include "task.h"
#include "os.h"
#include "./LED/led.h"
// 1. 新增：添加USART头文件（必须）
#include "./USART/usart.h"  

/******************************************************************************************************/
/*uC/OS-III配置*/

/* START_TASK 任务 配置
 * 包括: 任务优先级 任务栈大小 任务控制块 任务栈 任务函数
 */
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
#define TASK2_PRIO      (OS_CFG_PRIO_MAX - 5)           /* 2. 修改：原和TASK1同优先级，改为-5避免冲突 */
#define TASK2_STK_SIZE  1024                            /* 任务栈大小 */
OS_TCB                  Task2_TCB;                      /* 任务控制块 */
CPU_STK                 Task2_STK[TASK2_STK_SIZE];      /* 任务栈 */
void task2(void *p_arg);                                /* 任务函数 */

/* TASK_USART 任务 配置（新增）
 * 优先级：OS_CFG_PRIO_MAX - 6，低于TASK2，避免优先级冲突
 * 栈大小：256（比原128大，适配printf和串口操作）
 */
#define TASK_USART_PRIO     (OS_CFG_PRIO_MAX - 6)       
#define TASK_USART_STK_SIZE 256                         
OS_TCB                  TaskUSART_TCB;                  /* 取消static，和其他任务保持一致 */
CPU_STK                 TaskUSART_Stk[TASK_USART_STK_SIZE];
void TaskUSART(void *p_arg);                            /* 取消static，和其他任务保持一致 */

void app_start(void)
{
    OS_ERR err;
    
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
    OSSchedRoundRobinCfg(OS_TRUE, 0, &err);
    
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
                    (void          *)0,
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
                    (OS_TICK        )0,
                    (void          *)0,
                    (OS_OPT         )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                    (OS_ERR        *)&err);
    
    /* 3. 新增：创建USART任务（和Task1/Task2同位置） */
    OSTaskCreate(   (OS_TCB        *)&TaskUSART_TCB,
                    (CPU_CHAR      *)"usart_task",
                    (OS_TASK_PTR    )TaskUSART,
                    (void          *)0,
                    (OS_PRIO        )TASK_USART_PRIO,
                    (CPU_STK       *)&TaskUSART_Stk[0],
                    (CPU_STK_SIZE   )TASK_USART_STK_SIZE / 10,
                    (CPU_STK_SIZE   )TASK_USART_STK_SIZE,
                    (OS_MSG_QTY     )0,
                    (OS_TICK        )0,
                    (void          *)0,
                    (OS_OPT         )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                    (OS_ERR        *)&err);
    
    /* 删除Start Task */
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

/**
 * @brief       TaskUSART（新增）
 * @param       p_arg : 传入参数(未用到)
 * @retval      无
 * @note        适配现有代码风格，和task1/task2保持一致
 */
void TaskUSART(void *p_arg)
{
    OS_ERR err;
    uint8_t tx_buf[] = "uC/OS-III + STM32F4 USART Test: Hello World!\r\n";
    uint8_t rx_byte;

    (void)p_arg; // 未使用参数，消除编译警告

    while (1)
    {
        /* 1. 发送测试字符串 */
        USART_Send_Buf(tx_buf, sizeof(tx_buf)-1); // 去掉字符串结束符，避免发送多余字节
        /* 2. 用printf输出（重定向后可用） */
        printf("System Tick: %d | USART Task Running, LED0/LED1 Blinking\r\n", OSTimeGet(&err));

        /* 3. 尝试接收1个字节（超时3秒，3000ticks） */
        rx_byte = USART_Recv_Byte(3000);
        if (rx_byte != 0xFF)
        {
            printf("Received Byte: 0x%02X | ASCII: %c\r\n", rx_byte, rx_byte);
        }
        else
        {
            printf("USART Recv Timeout!\r\n");
        }

        /* 延时1秒（1000ticks），和task2节奏匹配 */
        OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
    }
}
