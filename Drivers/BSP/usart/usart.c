#include "usart.h"
#include "stdio.h"
#include "string.h"

/* uC/OS-III 同步对象定义 */
OS_MUTEX  USART_Mutex;
OS_Q      USART_Rx_Queue;

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
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
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

    /* 1. 创建uC/OS同步对象 */
    // 互斥锁：解决多任务串口发送冲突
    OSMutexCreate(&USART_Mutex, "USART1 Mutex", &err);
    // 消息队列：存储中断接收的字节（每个消息1字节）
    OSQCreate(&USART_Rx_Queue, "USART1 Rx Queue", USART_RX_QUEUE_SIZE, &err);

    /* 2. 配置硬件 */
    USART_HW_Config();
}

/**
 * @brief  发送单个字节（底层）
 */
void USART_Send_Byte(uint8_t byte)
{
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET); // 等待发送寄存器空
    USART_SendData(USARTx, byte);
}

/**
 * @brief  发送缓冲区（多任务安全，互斥锁保护）
 */
void USART_Send_Buf(uint8_t *buf, uint16_t len)
{
    OS_ERR err;
    if (buf == NULL || len == 0) return;

    /* 获取互斥锁：确保同一时间只有1个任务发送 */
    OSMutexPend(&USART_Mutex, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
    /* 逐字节发送 */
    for (uint16_t i = 0; i < len; i++)
    {
        USART_Send_Byte(buf[i]);
    }
    /* 释放互斥锁 */
    OSMutexPost(&USART_Mutex, OS_OPT_POST_NONE, &err);
}

/**
 * @brief  接收单个字节（从uC/OS队列读取，带超时）
 * @param  timeout: 超时时间（OS_TICK，1000=1秒 @OS_CFG_TICK_RATE_HZ=1000）
 * @retval 接收的字节（超时返回0xFF）
 */
uint8_t USART_Recv_Byte(OS_TICK timeout)
{
    OS_ERR err;
    void *msg;

    /* 从队列读取1字节（阻塞等待） */
    msg = OSQPend(&USART_Rx_Queue, timeout, OS_OPT_PEND_BLOCKING, NULL, NULL, &err);
    if (err != OS_ERR_NONE)
    {
        return 0xFF; // 超时
    }
    return (uint8_t)msg;
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
    return ch;
}
