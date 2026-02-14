/**
 ******************************************************************************
 * @file    task.c
 * @brief   Task code with English comments
 * @note    Fixed Protocol_Task delay issue causing data loss
 * @author  User
 * @date    2026-02-13
 ******************************************************************************
 */

#include "task.h"
#include "os.h"
#include "./LED/led.h"
#include "./key/key.h"
#include "./usart/usart.h"
#include "./lcd/lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Global PID parameters */
float p=0.0;
float i=0.0;
float d=0.0;

/* ========== Task Definitions ========== */

/* START TASK */
#define START_TASK_PRIO     2
#define START_STK_SIZE      512
OS_TCB      StartTask_TCB;
CPU_STK     StartTask_STK[START_STK_SIZE];
void start_task(void *p_arg);

/* TASK1 - LED Blink Task */
#define TASK1_PRIO          (OS_CFG_PRIO_MAX - 10)
#define TASK1_STK_SIZE      512
OS_TCB      Task1_TCB;
CPU_STK     Task1_STK[TASK1_STK_SIZE];
void task1(void *p_arg);

/* TASK2 - LED Blink Task */
#define TASK2_PRIO          (OS_CFG_PRIO_MAX - 9)
#define TASK2_STK_SIZE      5512
OS_TCB      Task2_TCB;
CPU_STK     Task2_STK[TASK2_STK_SIZE];
void task2(void *p_arg);

/* TASK3 - Reserved Task */
#define TASK3_PRIO          (OS_CFG_PRIO_MAX - 8)
#define TASK3_STK_SIZE      512
OS_TCB      Task3_TCB;
CPU_STK     Task3_STK[TASK3_STK_SIZE];
void task3(void *p_arg);

/* PROTOCOL TASK - UART Protocol Parsing Task */
#define PROTOCOL_PRIO       6  // High priority for fast UART data processing
#define PROTOCOL_STK_SIZE   2048
OS_TCB      Protocol_Task_TCB;
CPU_STK     Protocol_Task_STK[PROTOCOL_STK_SIZE];
void Protocol_Task(void *p_arg);

/* LCD TASK - LCD Display Task */
#define LCD_TASK_PRIO       10  
#define LCD_TASK_STK_SIZE   1024
OS_TCB      LCD_Task_TCB;
CPU_STK     LCD_Task_STK[LCD_TASK_STK_SIZE];
void LCD_Sensor_Task(void *p_arg);

/* ========== Application Start Function ========== */

void app_start(void)
{
    OS_ERR err;
    
    /* Initialize uC/OS-III */
    OSInit(&err);
    
    /* Create Start Task */
    OSTaskCreate(&StartTask_TCB,
                 "start_task",
                 start_task,
                 NULL,
                 START_TASK_PRIO,
                 &StartTask_STK[0],
                 START_STK_SIZE / 10,
                 START_STK_SIZE,
                 0, 0, NULL,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 &err);
    
    /* Start task scheduler */
    OSStart(&err);
    
    for (;;)
    {
        /* Should never reach here */
    }
}

/* ========== START TASK ========== */

void start_task(void *p_arg)
{
    OS_ERR err;
    CPU_INT32U cnts;
    RCC_ClocksTypeDef rcc_clocks;
    
    (void)p_arg;
    
    /* Initialize CPU library */
    CPU_Init();
    
    /* Configure SysTick based on configured tick rate */
    RCC_GetClocksFreq(&rcc_clocks);
    cnts = ((CPU_INT32U)rcc_clocks.HCLK_Frequency) / OSCfg_TickRate_Hz;
    OS_CPU_SysTickInit(cnts);
    
    /* Enable time slice scheduling */
    OSSchedRoundRobinCfg(OS_TRUE, 10, &err);
    
    /* Create Task1 */
    OSTaskCreate(&Task1_TCB, "task1", task1, NULL,
                 TASK1_PRIO,
                 &Task1_STK[0],
                 TASK1_STK_SIZE / 10,
                 TASK1_STK_SIZE,
                 0, 10, NULL,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 &err);
    
    /* Create Task2 */
    OSTaskCreate(&Task2_TCB, "task2", task2, NULL,
                 TASK2_PRIO,
                 &Task2_STK[0],
                 TASK2_STK_SIZE / 10,
                 TASK2_STK_SIZE,
                 0, 10, NULL,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 &err);
    
    /* Create Task3 */
    OSTaskCreate(&Task3_TCB, "task3", task3, NULL,
                 TASK3_PRIO,
                 &Task3_STK[0],
                 TASK3_STK_SIZE / 10,
                 TASK3_STK_SIZE,
                 0, 10, NULL,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 &err);

    /* Create UART protocol parsing task (fixed version) */
    OSTaskCreate(&Protocol_Task_TCB, "Protocol_Task", Protocol_Task, NULL,
                 PROTOCOL_PRIO,
                 &Protocol_Task_STK[0],
                 PROTOCOL_STK_SIZE / 10,
                 PROTOCOL_STK_SIZE,
                 0, 0, NULL,  // No time slice
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 &err);
    
    /* Create LCD display task */
    OSTaskCreate(&LCD_Task_TCB, "LCD_Task", LCD_Sensor_Task, NULL,
                 8,  // Priority
                 &LCD_Task_STK[0],
                 LCD_TASK_STK_SIZE / 10,
                 LCD_TASK_STK_SIZE,
                 0, 0, NULL,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 &err);
    
    /* Delete startup task */
    OSTaskDel(NULL, &err);
}

/* ========== TASK1 ========== */

void task1(void *p_arg)
{
    OS_ERR err;
    (void)p_arg;
    
    while(1)
    {
        LED0 = !LED0;
        printf("task1 is working!!!\r\n");
        OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
    }
}

/* ========== TASK2 ========== */

void task2(void *p_arg)
{
    OS_ERR err;
    (void)p_arg;
    
    while(1)
    {
        LED1 = !LED1;
        printf("task2 is working!!!\r\n");
        OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
    }
}

/* ========== TASK3 ========== */

void task3(void *p_arg)
{
    OS_ERR err;
    (void)p_arg;
    
    while(1)
    {
        OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
    }
}

/* ========== LCD Sensor TASK ========== */

/**
 * @brief  LCD display task
 * @note   Displays PID parameters on LCD screen
 */
void LCD_Sensor_Task(void*p_arg)
{
    (void)p_arg;
    OS_ERR err;
    u8 buff[20];
    memset(buff,0,sizeof(buff));
    /* Clear LCD screen with white background */
    LCD_Fill(0,0,LCD_HEIGHT,LCD_WIDTH,WHITE);
    
    /* Display task title */
    LCD_ShowString(0,0,"LCD Sensor Task",BLACK,WHITE,16,0);
    printf("LCD Sensor Task is starting!!!");
    
    while(1)
    {
        /* Format and display PID parameters */
        sprintf((char*)buff,"p:%.2f,i:%.2f",p,i);
        LCD_ShowString(0,20,buff,BLACK,WHITE,16,0);
        sprintf((char*)buff,"d:%.2f",d);
				LCD_ShowString(0,40,buff,BLACK,WHITE,16,0);
        /* Delay 20ms for display refresh */
        OSTimeDly(20, OS_OPT_TIME_DLY, &err);
    }
}

/* ========== PROTOCOL TASK ========== */

/**
 * @brief  UART protocol parsing task (fixed version)
 * @note   Fix points:
 *         1. Remove delay at end of loop
 *         2. Fix buffer index error
 *         3. Add buffer clear and null terminator
 *         4. Optimize state machine logic
 */
void Protocol_Task(void *p_arg)
{
    (void)p_arg;
    
    OS_ERR err;
    void *p_msg;
    OS_MSG_SIZE msg_size;
    CPU_TS ts;
    
    /* State machine variables */
    uint8_t rx_state = 0;      // 0=Wait for packet header '[', 1=Receive data
    char rx_buffer[50];        // Receive buffer
    uint8_t rx_index = 0;      // Receive position
    
    /* Initialize buffer */
    memset(rx_buffer, 0, sizeof(rx_buffer));  
    
    while(1)
    {
        /* Block waiting for message (infinite wait) */
        p_msg = OSQPend(&USART_Rx_Queue, 
                        0,                      // Infinite wait
                        OS_OPT_PEND_BLOCKING, 
                        &msg_size, 
                        &ts, 
                        &err);
        
        if(err == OS_ERR_NONE)
        {
            /* Retrieve byte data from pointer */
            uint8_t rx_data = (uint8_t)(uintptr_t)p_msg;
            
            /* State machine processing */
            switch(rx_state)
            {
                case 0:  /* Wait for packet header '[' */
                {
                    if(rx_data == '[')
                    {
                        rx_state = 1;
                        rx_index = 0;
                        memset(rx_buffer, 0, sizeof(rx_buffer));  // Clear buffer
                        printf("Packet Start\r\n");
                    }
                    break;
                }
                
                case 1:  /* Receive data */
                {
                    if(rx_data == ']')
                    {
                        /* Packet end */
                        rx_state = 0;
                        rx_buffer[rx_index] = '\0';  // Add string terminator
                        
                        printf("Received Packet: [%s]\r\n", rx_buffer);
                        
                        /* Parse protocol */
                        char *str1 = strtok(rx_buffer, ",");
                        char *str2 = strtok(NULL, ",");
                        char *str3 = strtok(NULL, ",");
                        char *str4 = strtok(NULL, ",");
                        
                        if(str1 != NULL && strcmp(str1, "PID") == 0)
                        {
                            if(str2 && str3 && str4)
                            {
                                p = atof(str2);
                                i = atof(str3);
                                d = atof(str4);
                                printf("PID Parameters: P=%.3f, I=%.3f, D=%.3f\r\n", p, i, d);
                            }
                            else
                            {
                                printf("PID Parse Error: Missing parameters\r\n");
                            }
                        }
                        else if(str1 != NULL && strcmp(str1, "LED") == 0)
                        {
                            if(str2 && strcmp(str2, "ON") == 0)
                            {
                                LED0 = 0;
                                printf("LED ON\r\n");
                            }
                            else if(str2 && strcmp(str2, "OFF") == 0)
                            {
                                LED0 = 1;
                                printf("LED OFF\r\n");
                            }
                        }
                        else
                        {
                            printf("Unknown Command: %s\r\n", str1 ? str1 : "NULL");
                        }
                        
                        /* Reset index */
                        rx_index = 0;
                    }
                    else
                    {
                        /* Store in buffer */
                        if(rx_index < sizeof(rx_buffer) - 1)
                        {
                            rx_buffer[rx_index++] = rx_data;  // Fix: assign then increment
                        }
                        else
                        {
                            /* Buffer overflow, reset state */
                            printf("Buffer Overflow! Reset.\r\n");
                            rx_state = 0;
                            rx_index = 0;
                        }
                    }
                    break;
                }
                
                default:
                    rx_state = 0;
                    break;
            }
        }
        else
        {
            /* Queue receive error */
            printf("OSQPend Error: %d\r\n", err);
        }
    }
}
