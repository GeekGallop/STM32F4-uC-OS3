#ifndef __USART_H
#define __USART_H

#include "stm32f4xx.h"
#include "os.h"
#include "string.h"
#include "stdio.h"
/* 串口配置（USART1，PA9/PA10，标准库） */
#define USARTx                   USART1
#define USARTx_CLK               RCC_APB2Periph_USART1
#define USARTx_GPIO_CLK          RCC_AHB1Periph_GPIOA
#define USARTx_TX_GPIO_PORT      GPIOA
#define USARTx_TX_GPIO_PIN       GPIO_Pin_9
#define USARTx_RX_GPIO_PORT      GPIOA
#define USARTx_RX_GPIO_PIN       GPIO_Pin_10
#define USARTx_IRQn              USART1_IRQn
#define USARTx_IRQHandler        USART1_IRQHandler

/* 同步配置 */
#define USART_RX_QUEUE_SIZE      256  // 接收队列大小（字节）
#define USART_BAUDRATE           115200

/* uC/OS-III 同步对象（全局） */
extern OS_MUTEX  USART_Mutex;    // 发送互斥锁
extern OS_Q      USART_Rx_Queue; // 接收消息队列

/* 函数声明 */
void USART_Config(void);         // USART初始化（硬件+uC/OS对象）
void USART_Send_Byte(uint8_t byte); // 发送单个字节
void USART_Send_Buf(uint8_t *buf, uint16_t len); // 发送缓冲区（多任务安全）
uint8_t USART_Recv_Byte(OS_TICK timeout); // 接收单个字节（带超时）
int fputc(int ch, FILE *f);      // 重定向printf到串口

#endif /* __USART_H */

