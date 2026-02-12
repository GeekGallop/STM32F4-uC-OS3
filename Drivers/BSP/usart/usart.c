#include "usart.h"
#include "stdio.h"
#include "string.h"
#include "sys.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "misc.h"
#include "os.h"
/* uC/OS-III 同步对象定义 */
OS_MUTEX  USART_Mutex;
OS_Q      USART_Rx_Queue;

char Serial_RxPacket[100];				
uint8_t Serial_RxFlag;	

uint8_t uart_rx_count;
uint8_t uart_rx_error;
uint8_t uart_tx_error;

/**
 * @brief  USART底层硬件配置（标准库）
 * @note   配置GPIO、时钟、中断、波特率等
 */
static void USART_HW_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    /* 1. 使能时钟 */
    RCC_AHB1PeriphClockCmd(USARTx_GPIO_CLK, ENABLE); // GPIO时钟
    RCC_APB2PeriphClockCmd(USARTx_CLK, ENABLE);      // USART时钟

    /* 2. 配置TX引脚（PA9）：复用推挽输出 */
    GPIO_InitStruct.GPIO_Pin = USARTx_TX_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;        // 复用模式
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

    /* 3. 配置RX引脚（PA10）：复用浮空输入 */
    GPIO_InitStruct.GPIO_Pin = USARTx_RX_GPIO_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

    /* 4. 引脚复用映射 */
    GPIO_PinAFConfig(USARTx_TX_GPIO_PORT, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(USARTx_RX_GPIO_PORT, GPIO_PinSource10, GPIO_AF_USART1);

    /* 5. 配置USART参数 */
    USART_InitStruct.USART_BaudRate = USART_BAUDRATE;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USARTx, &USART_InitStruct);

    /* 6. 配置中断（接收非空中断） */
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE); // 使能接收非空中断

    /* 7. 配置NVIC */
    NVIC_InitStruct.NVIC_IRQChannel = USARTx_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5; // 优先级>OS_CPU_CFG_INT_PRIO_MIN
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    /* 8. 使能USART */
    USART_Cmd(USARTx, ENABLE);
}

/**
 * @brief  USART初始化（含uC/OS同步对象创建）
 */
void USART_Config(void)
{
    OS_ERR err;


    // 互斥锁：解决多任务串口发送冲突
    OSMutexCreate(&USART_Mutex, "USART1 Mutex", &err);
    // 消息队列：存储中断接收的字节（每个消息1字节）
    OSQCreate(&USART_Rx_Queue, "USART1 Rx Queue", USART_RX_QUEUE_SIZE, &err);

    USART_HW_Config();
}

  void USART_Send_Byte(uint8_t byte)
  {
      // 等待发送数据寄存器空（TXE标志位）
      while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
      
      // 写入数据寄存器
      USART_SendData(USART1, byte);
			
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET);
  }

/**
 * @brief  重定向fputc，支持printf输出到串口
 */
int fputc(int ch, FILE *f)
{
    OS_ERR err;
    /* 互斥锁保护printf，避免多任务printf乱码 */
    OSMutexPend(&USART_Mutex, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
    USART_Send_Byte((uint8_t)ch);
    OSMutexPost(&USART_Mutex, OS_OPT_POST_NONE, &err);

    if(err!=OS_ERR_NONE)
    {
        uart_tx_error++;
    }
    return ch;
}


void USART1_IRQHandler(void)                	
{

	OSIntEnter();    
    OS_ERR err;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)	
	{

		uint8_t data_rx = USART_ReceiveData(USARTx);

		/* 将接收到的字节数据转换为指针传递给队列 */
		void *p_msg = (void *)(uintptr_t)data_rx;

			OSQPost(&USART_Rx_Queue, p_msg, sizeof(uint8_t), OS_OPT_POST_FIFO, &err);
        if(err==OS_ERR_NONE)
        {
            uart_rx_count++;//计数接受数目
        }
        else
        {
            uart_rx_error++;//计数丢包数
        }

		USART_ClearITPendingBit(USART1, USART_IT_RXNE);		
	}
	OSIntExit();  											 
} 

