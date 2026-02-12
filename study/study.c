/*******************************************************************************
 * 文件名称: study.c
 * 功能描述: STM32F4 + uC-OS3 串口配置详细步骤说明文档
 * 创建日期: 2026-02-12
 * 
 * 本文档详细说明了如何在uC-OS3实时操作系统环境下配置STM32F4的串口收发功能
 * 包含硬件配置、中断配置、uC-OS3同步对象使用等完整步骤
 * 
 * 【重要提示】原始源码文件中的中文注释存在编码问题（乱码），本文档已修正
 ******************************************************************************/

/*******************************************************************************
 * 零、原始代码中的乱码注释翻译对照表
 ******************************************************************************/
/*
 * 原始文件中由于编码问题（GBK/UTF-8混用），部分中文注释显示为乱码。
 * 以下是乱码注释的正确翻译：
 * 
 * usart.h 文件中的乱码翻译：
 * -------------------------
 * extern OS_MUTEX  USART_Mutex;    // ���ͻ�����
 *   正确翻译：// 发送互斥锁
 * 
 * extern OS_Q      USART_Rx_Queue; // ������Ϣ����
 *   正确翻译：// 接收消息队列
 * 
 * void USART_Config(void);         // USART��ʼ����Ӳ��+uC/OS����
 *   正确翻译：// USART初始化（硬件+uC/OS对象）
 * 
 * void USART_Send_Byte(uint8_t byte); // ���͵����ֽ�
 *   正确翻译：// 发送单个字节
 * 
 * void USART_Send_Buf(uint8_t *buf, uint16_t len); // ���ͻ�������������ȫ��
 *   正确翻译：// 发送缓冲区（多任务安全）
 * 
 * uint8_t USART_Recv_Byte(OS_TICK timeout); // ���յ����ֽڣ�����ʱ��
 *   正确翻译：// 接收单个字节（带超时）
 * 
 * int fputc(int ch, FILE *f);      // �ض���printf������
 *   正确翻译：// 重定向printf到串口
 * 
 * 
 * usart.c 文件中的乱码翻译：
 * -------------------------
 * void USART1_IRQHandler(void)     // ����1�жϷ������
 *   正确翻译：// 串口1中断服务函数
 * 
 * static uint8_t RxState = 0;      // �����ʾ��ǰ״̬��״̬�ľ�̬����
 *   正确翻译：// 定义表示当前状态机状态的静态变量
 * 
 * static uint8_t pRxPacket = 0;    // �����ʾ��ǰ��������λ�õľ�̬����
 *   正确翻译：// 定义表示当前接收数据位置的静态变量
 * 
 * if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)  // �ж��Ƿ���USART1�Ľ����¼��������ж�
 *   正确翻译：// 判断是否是USART1的接收事件触发的中断
 * 
 * uint8_t RxData = USART_ReceiveData(USART1);  // ��ȡ���ݼĴ���������ڽ��յ����ݱ���
 *   正确翻译：// 读取数据寄存器，将接收到的数据保存
 * 
 * if (RxData == '[' && Serial_RxFlag == 0)     // �������ȷʵ�ǰ�ͷ��������һ�����ݰ��Ѵ������
 *   正确翻译：// 如果接收正确的包头，并且上一个数据包已处理完
 * 
 * RxState = 1;                     // ����һ��״̬
 *   正确翻译：// 置下一个状态
 * 
 * pRxPacket = 0;                   // ���ݰ���λ�ù���
 *   正确翻译：// 数据包的位置归零
 * 
 * if (RxData == ']')               // ����յ��ڶ�����β
 *   正确翻译：// 如果接收到第二个包尾
 * 
 * RxState = 0;                     // ״̬��0
 *   正确翻译：// 状态归0
 * 
 * Serial_RxPacket[pRxPacket] = '\0';  // ���յ����ַ����ݰ�����һ���ַ���������־
 *   正确翻译：// 给接收到的字符串数据包添加一个字符串结束标志
 * 
 * Serial_RxFlag = 1;               // �������ݰ���־λ��1���ɹ�����һ�����ݰ�
 *   正确翻译：// 置接收数据包标志位为1，成功接收一个数据包
 * 
 * else                             // ���յ�������������
 *   正确翻译：// 如果接收到的是数据内容
 * 
 * Serial_RxPacket[pRxPacket] = RxData;  // �����ݴ������ݰ������ָ��λ��
 *   正确翻译：// 将数据存入数据包数组的指定位置
 * 
 * pRxPacket ++;                    // ���ݰ���λ������
 *   正确翻译：// 数据包的位置自增
 * 
 * USART_ClearITPendingBit(USART1, USART_IT_RXNE);  // �����־λ
 *   正确翻译：// 清除标志位
 * 
 * 
 * 【解决编码问题的方法】
 * ----------------------
 * 1. Keil MDK中：Edit -> Configuration -> Editor -> Encoding 选择 GB2312 或 UTF-8
 * 2. 重新打开文件，乱码应该正常显示
 * 3. 如需转换编码，可使用Notepad++等工具批量转换为UTF-8
 */

/*******************************************************************************
 * 一、项目串口配置概述
 ******************************************************************************/
/*
 * 本项目使用的串口配置：
 * - 串口外设：USART1
 * - 引脚配置：PA9(TX)、PA10(RX)
 * - 波特率：115200
 * - 数据位：8位
 * - 停止位：1位
 * - 校验位：无
 * - 硬件流控：无
 * - 工作模式：中断接收 + 轮询发送
 * - uC-OS3同步机制：互斥锁(发送保护) + 消息队列(接收缓冲)
 */

/*******************************************************************************
 * 二、串口配置完整步骤（基于STM32标准库）
 ******************************************************************************/

/*------------------------------------------------------------------------------
 * 步骤1：头文件配置（usart.h）
 *----------------------------------------------------------------------------*/
/*
 * 1.1 定义串口硬件参数宏
 * 
 * #define USARTx                   USART1              // 使用USART1外设
 * #define USARTx_CLK               RCC_APB2Periph_USART1  // USART1时钟（APB2总线）
 * #define USARTx_GPIO_CLK          RCC_AHB1Periph_GPIOA   // GPIO时钟（AHB1总线）
 * #define USARTx_TX_GPIO_PORT      GPIOA               // TX引脚端口
 * #define USARTx_TX_GPIO_PIN       GPIO_Pin_9          // TX引脚号
 * #define USARTx_RX_GPIO_PORT      GPIOA               // RX引脚端口
 * #define USARTx_RX_GPIO_PIN       GPIO_Pin_10         // RX引脚号
 * #define USARTx_IRQn              USART1_IRQn         // 中断通道
 * #define USARTx_IRQHandler        USART1_IRQHandler   // 中断服务函数名
 * 
 * 1.2 定义串口功能参数宏
 * 
 * #define USART_RX_QUEUE_SIZE      256                 // 接收队列大小（字节数）
 * #define USART_BAUDRATE           115200              // 波特率
 * 
 * 1.3 声明uC-OS3同步对象（全局变量）
 * 
 * extern OS_MUTEX  USART_Mutex;       // 互斥锁：保护发送操作，防止多任务冲突
 * extern OS_Q      USART_Rx_Queue;    // 消息队列：存储中断接收的字节
 * 
 * 1.4 声明函数接口
 * 
 * void USART_Config(void);                          // 串口初始化（硬件+uC/OS对象）
 * void USART_Send_Byte(uint8_t byte);               // 发送单字节（底层函数）
 * void USART_Send_Buf(uint8_t *buf, uint16_t len);  // 发送缓冲区（多任务安全）
 * uint8_t USART_Recv_Byte(OS_TICK timeout);         // 接收单字节（带超时）
 * int fputc(int ch, FILE *f);                       // 重定向printf到串口
 */

/*------------------------------------------------------------------------------
 * 步骤2：硬件底层配置（usart.c中的USART_HW_Config函数）
 *----------------------------------------------------------------------------*/
/*
 * 2.1 使能时钟
 * 
 * RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // 使能GPIOA时钟
 * RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); // 使能USART1时钟
 * 
 * 注意：
 * - STM32F4的USART1挂载在APB2总线上，最高频率84MHz
 * - USART2/3挂载在APB1总线上，最高频率42MHz
 * 
 * 2.2 配置GPIO引脚
 * 
 * // TX引脚（PA9）配置为复用推挽输出
 * GPIO_InitTypeDef GPIO_InitStruct;
 * GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
 * GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;        // 复用功能模式
 * GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;   // 输出速度50MHz
 * GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;      // 推挽输出
 * GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
 * GPIO_Init(GPIOA, &GPIO_InitStruct);
 * 
 * // RX引脚（PA10）配置为复用输入
 * GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
 * GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;        // 复用功能模式
 * GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉（防止浮空接收干扰）
 * GPIO_Init(GPIOA, &GPIO_InitStruct);
 * 
 * 2.3 配置引脚复用映射
 * 
 * GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);   // PA9复用为USART1_TX
 * GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);  // PA10复用为USART1_RX
 * 
 * 注意：STM32F4必须显式配置复用功能，否则引脚不工作
 * 
 * 2.4 配置USART参数
 * 
 * USART_InitTypeDef USART_InitStruct;
 * USART_InitStruct.USART_BaudRate = 115200;                      // 波特率115200
 * USART_InitStruct.USART_WordLength = USART_WordLength_8b;       // 数据位8位
 * USART_InitStruct.USART_StopBits = USART_StopBits_1;            // 停止位1位
 * USART_InitStruct.USART_Parity = USART_Parity_No;               // 无校验
 * USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
 * USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;   // 使能收发
 * USART_Init(USART1, &USART_InitStruct);
 * 
 * 2.5 使能接收中断
 * 
 * USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // 使能接收非空中断（RXNE）
 * 
 * 注意：
 * - USART_IT_RXNE：接收数据寄存器非空中断，每接收1字节触发1次
 * - 不使能发送中断，采用轮询方式发送（简单可靠）
 * 
 * 2.6 配置NVIC中断优先级
 * 
 * NVIC_InitTypeDef NVIC_InitStruct;
 * NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
 * NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5;  // 抢占优先级5
 * NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;         // 子优先级0
 * NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
 * NVIC_Init(&NVIC_InitStruct);
 * 
 * 重要提示：
 * - uC-OS3要求中断优先级数值 > OS临界区优先级（通常为4）
 * - 本项目设置为5，确保中断可以被OS管理
 * - 数值越大优先级越低（STM32 Cortex-M4特性）
 * 
 * 2.7 使能USART外设
 * 
 * USART_Cmd(USART1, ENABLE);  // 启动USART1
 */

/*------------------------------------------------------------------------------
 * 步骤3：uC-OS3同步对象创建（usart.c中的USART_Config函数）
 *----------------------------------------------------------------------------*/
/*
 * 3.1 创建互斥锁（保护发送操作）
 * 
 * OS_ERR err;
 * OSMutexCreate(&USART_Mutex, "USART1 Mutex", &err);
 * 
 * 作用：
 * - 防止多个任务同时调用printf或USART_Send_Buf导致数据混乱
 * - 例如：Task1打印"Hello"，Task2打印"World"，不加锁可能输出"HeWolrllod"
 * 
 * 3.2 创建消息队列（存储接收数据）
 * 
 * OSQCreate(&USART_Rx_Queue, "USART1 Rx Queue", USART_RX_QUEUE_SIZE, &err);
 * 
 * 作用：
 * - 中断中接收的字节存入队列，任务从队列读取（中断与任务解耦）
 * - 队列大小256字节，可缓冲高速数据
 * - 支持阻塞读取，任务可等待数据到达
 * 
 * 3.3 调用硬件配置
 * 
 * USART_HW_Config();  // 执行步骤2的硬件初始化
 */

/*------------------------------------------------------------------------------
 * 步骤4：发送函数实现
 *----------------------------------------------------------------------------*/
/*
 * 4.1 底层发送函数（轮询方式）
 * 
 * void USART_Send_Byte(uint8_t byte)
 * {
 *     // 等待发送数据寄存器空（TXE标志位）
 *     while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
 *     
 *     // 写入数据寄存器
 *     USART_SendData(USART1, byte);
 * }
 * 
 * 特点：
 * - 简单可靠，适合低速发送
 * - 阻塞等待，不适合大量数据（可改用DMA）
 * 
 * 4.2 多任务安全发送函数（互斥锁保护）
 * 
 * void USART_Send_Buf(uint8_t *buf, uint16_t len)
 * {
 *     OS_ERR err;
 *     if (buf == NULL || len == 0) return;
 *     
 *     // 获取互斥锁（阻塞等待）
 *     OSMutexPend(&USART_Mutex, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
 *     
 *     // 逐字节发送
 *     for (uint16_t i = 0; i < len; i++)
 *     {
 *         USART_Send_Byte(buf[i]);
 *     }
 *     
 *     // 释放互斥锁
 *     OSMutexPost(&USART_Mutex, OS_OPT_POST_NONE, &err);
 * }
 * 
 * 使用示例：
 * uint8_t data[] = "Hello uC-OS3\r\n";
 * USART_Send_Buf(data, strlen((char*)data));
 * 
 * 4.3 重定向printf函数
 * 
 * int fputc(int ch, FILE *f)
 * {
 *     OS_ERR err;
 *     OSMutexPend(&USART_Mutex, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
 *     USART_Send_Byte((uint8_t)ch);
 *     OSMutexPost(&USART_Mutex, OS_OPT_POST_NONE, &err);
 *     return ch;
 * }
 * 
 * 配置要求（Keil MDK）：
 * - 勾选 Options -> Target -> Use MicroLIB
 * - 包含头文件 #include <stdio.h>
 * 
 * 使用示例：
 * printf("Task1 running, count=%d\r\n", count);
 */

/*------------------------------------------------------------------------------
 * 步骤5：接收函数实现
 *----------------------------------------------------------------------------*/
/*
 * 5.1 任务层接收函数（从队列读取）
 * 
 * uint8_t USART_Recv_Byte(OS_TICK timeout)
 * {
 *     OS_ERR err;
 *     void *msg;
 *     
 *     // 从队列读取1字节（阻塞等待，带超时）
 *     msg = OSQPend(&USART_Rx_Queue, timeout, OS_OPT_PEND_BLOCKING, NULL, NULL, &err);
 *     
 *     if (err != OS_ERR_NONE)
 *     {
 *         return 0xFF;  // 超时返回0xFF
 *     }
 *     
 *     return (uint8_t)msg;  // 返回接收的字节
 * }
 * 
 * 参数说明：
 * - timeout：超时时间（单位：OS Tick）
 *   例如：timeout=1000 表示等待1秒（假设OS_CFG_TICK_RATE_HZ=1000）
 *   timeout=0 表示永久等待
 * 
 * 使用示例：
 * uint8_t rx_data = USART_Recv_Byte(1000);  // 等待1秒
 * if (rx_data != 0xFF)
 * {
 *     printf("Received: %c\r\n", rx_data);
 * }
 * 
 * 5.2 中断服务函数（接收数据并发送到队列）
 * 
 * void USART1_IRQHandler(void)
 * {
 *     OS_ERR err;
 *     
 *     // 进入中断（uC-OS3必须调用）
 *     OSIntEnter();
 *     
 *     // 检查接收非空中断标志
 *     if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
 *     {
 *         // 读取接收数据寄存器（自动清除RXNE标志）
 *         uint8_t rx_data = USART_ReceiveData(USART1);
 *         
 *         // 将数据发送到队列（不阻塞）
 *         OSQPost(&USART_Rx_Queue, (void*)rx_data, sizeof(uint8_t), OS_OPT_POST_FIFO, &err);
 *         
 *         // 清除中断标志（可选，读数据已自动清除）
 *         USART_ClearITPendingBit(USART1, USART_IT_RXNE);
 *     }
 *     
 *     // 退出中断（uC-OS3必须调用）
 *     OSIntExit();
 * }
 * 
 * 注意事项：
 * - 必须调用OSIntEnter()和OSIntExit()，否则OS调度异常
 * - 中断中不能使用阻塞API（如OSMutexPend），只能用POST类函数
 * - 队列满时OSQPost会返回错误，可根据err处理（本项目未处理）
 */

/*******************************************************************************
 * 三、在uC-OS3任务中使用串口的示例
 ******************************************************************************/

/*------------------------------------------------------------------------------
 * 示例1：发送任务（定时打印信息）
 *----------------------------------------------------------------------------*/
/*
void uart_send_task(void *p_arg)
{
    OS_ERR err;
    uint32_t count = 0;
    
    while (1)
    {
        // 方法1：使用printf（已重定向到串口）
        printf("Task running, count=%d\r\n", count);
        
        // 方法2：使用USART_Send_Buf
        // uint8_t buf[50];
        // uint16_t len = sprintf((char*)buf, "Count=%d\r\n", count);
        // USART_Send_Buf(buf, len);
        
        count++;
        OSTimeDly(1000, OS_OPT_TIME_DLY, &err);  // 延时1秒
    }
}
*/

/*------------------------------------------------------------------------------
 * 示例2：接收任务（等待串口数据并处理）
 *----------------------------------------------------------------------------*/
/*
void uart_recv_task(void *p_arg)
{
    OS_ERR err;
    uint8_t rx_data;
    
    while (1)
    {
        // 阻塞等待接收数据（超时5秒）
        rx_data = USART_Recv_Byte(5000);
        
        if (rx_data != 0xFF)  // 接收成功
        {
            // 回显接收的数据
            printf("Received: %c (0x%02X)\r\n", rx_data, rx_data);
            
            // 根据接收数据执行操作
            switch (rx_data)
            {
                case '1':
                    printf("LED ON\r\n");
                    // LED_ON();
                    break;
                case '0':
                    printf("LED OFF\r\n");
                    // LED_OFF();
                    break;
                default:
                    printf("Unknown command\r\n");
                    break;
            }
        }
        else  // 超时
        {
            printf("Timeout, no data received\r\n");
        }
    }
}
*/

/*------------------------------------------------------------------------------
 * 示例3：接收字符串（带协议解析）
 *----------------------------------------------------------------------------*/
/*
 * 本项目中断中已实现简单协议：[数据内容]
 * 例如：发送 [LED1] 会被解析到 Serial_RxPacket[] 数组
 * 
 * 使用方法：
 * 
void protocol_task(void *p_arg)
{
    OS_ERR err;
    
    while (1)
    {
        // 检查接收标志
        if (Serial_RxFlag == 1)
        {
            // 处理接收的数据包
            printf("Received packet: %s\r\n", Serial_RxPacket);
            
            // 命令解析
            if (strcmp(Serial_RxPacket, "LED1") == 0)
            {
                LED1 = !LED1;
                printf("LED1 toggled\r\n");
            }
            else if (strcmp(Serial_RxPacket, "STATUS") == 0)
            {
                printf("System running OK\r\n");
            }
            
            // 清除标志
            Serial_RxFlag = 0;
        }
        
        OSTimeDly(10, OS_OPT_TIME_DLY, &err);  // 10ms轮询一次
    }
}
*/

/*******************************************************************************
 * 四、中断服务函数详解（含协议解析状态机）
 ******************************************************************************/

/*------------------------------------------------------------------------------
 * 4.1 项目原始中断函数分析（裸机代码，存在多任务安全问题）
 *----------------------------------------------------------------------------*/
/*
 * 【原始代码】本项目的中断函数实现了一个简单的数据包协议：[数据内容]
 * 
 * void USART1_IRQHandler(void)
 * {
 *     OSIntEnter();  // 进入中断（uC-OS3必须）
 *     
 *     // 定义静态变量（保持状态）
 *     static uint8_t RxState = 0;      // 当前状态：0=等待包头，1=接收数据
 *     static uint8_t pRxPacket = 0;    // 当前接收位置索引
 *     
 *     if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
 *     {
 *         uint8_t RxData = USART_ReceiveData(USART1);  // 读取接收数据
 *         
 *         // 状态0：等待包头 '['
 *         if (RxState == 0)
 *         {
 *             if (RxData == '[' && Serial_RxFlag == 0)  // 检测到包头且上一包已处理
 *             {
 *                 RxState = 1;      // 切换到接收数据状态
 *                 pRxPacket = 0;    // 位置归零
 *             }
 *         }
 *         // 状态1：接收数据内容
 *         else if (RxState == 1)
 *         {
 *             if (RxData == ']')    // 检测到包尾 ']'
 *             {
 *                 RxState = 0;      // 状态归零，等待下一个包
 *                 Serial_RxPacket[pRxPacket] = '\0';  // 添加字符串结束符
 *                 Serial_RxFlag = 1;  // 置标志位，通知任务处理
 *             }
 *             else  // 接收数据内容
 *             {
 *                 Serial_RxPacket[pRxPacket] = RxData;  // 存入缓冲区
 *                 pRxPacket++;  // 位置自增
 *             }
 *         }
 *         
 *         USART_ClearITPendingBit(USART1, USART_IT_RXNE);  // 清除中断标志
 *     }
 *     
 *     OSIntExit();  // 退出中断（uC-OS3必须）
 * }
 * 
 * 
 * 【存在的问题】这是典型的裸机代码移植到RTOS的问题：
 * ============================================================
 * 
 * 问题1：多任务竞争条件（Race Condition）
 * ----------------------------------------
 * - Serial_RxFlag 和 Serial_RxPacket[] 是全局变量
 * - 中断中写入（Serial_RxFlag = 1），任务中读取和清除（Serial_RxFlag = 0）
 * - 如果任务正在读取 Serial_RxPacket[] 时，中断又开始写入新数据，会导致数据混乱
 * 
 * 问题2：数据覆盖风险
 * -------------------
 * - 如果任务处理速度慢，Serial_RxFlag 还是1时，新的数据包到达
 * - 中断会因为 Serial_RxFlag == 0 的判断而丢弃新包头
 * - 或者更糟：如果去掉这个判断，新数据会覆盖旧数据
 * 
 * 问题3：缓冲区溢出保护缺失
 * -------------------------
 * - pRxPacket++ 没有上限检查
 * - 如果接收超过100字节（Serial_RxPacket[100]），会导致数组越界
 * 
 * 问题4：不符合uC-OS3设计理念
 * ---------------------------
 * - uC-OS3提供了消息队列、信号量等同步机制
 * - 应该使用这些机制而不是全局标志位
 * - 裸机的轮询方式不适合RTOS
 * 
 * 
 * 【是否需要修改】答案：强烈建议修改！
 * ========================================
 * 
 * 虽然这段代码在简单场景下可能"能用"，但存在以下隐患：
 * 1. 多任务环境下不安全
 * 2. 数据可能丢失或损坏
 * 3. 不符合RTOS最佳实践
 * 4. 难以扩展和维护
 */

/*------------------------------------------------------------------------------
 * 4.2 改进方案1：使用消息队列（推荐，符合uC-OS3设计）
 *----------------------------------------------------------------------------*/
/*
 * 【设计思路】
 * - 中断中：将每个接收字节发送到消息队列
 * - 任务中：从队列读取字节，进行协议解析
 * - 优点：中断与任务完全解耦，线程安全
 * 
 * 【实现代码】
 * 
 * // ========== 中断函数（简化版，只负责接收字节） ==========
 * void USART1_IRQHandler(void)
 * {
 *     OS_ERR err;
 *     OSIntEnter();
 *     
 *     if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
 *     {
 *         uint8_t rx_data = USART_ReceiveData(USART1);
 *         
 *         // 将字节发送到队列（中断中使用POST，不阻塞）
 *         OSQPost(&USART_Rx_Queue, (void*)rx_data, sizeof(uint8_t), 
 *                 OS_OPT_POST_FIFO, &err);
 *         
 *         // 如果队列满，err会返回错误，可以记录丢包计数
 *         // if (err != OS_ERR_NONE) { rx_error_count++; }
 *         
 *         USART_ClearITPendingBit(USART1, USART_IT_RXNE);
 *     }
 *     
 *     OSIntExit();
 * }
 * 
 * 
 * // ========== 任务函数（负责协议解析） ==========
 * void uart_protocol_task(void *p_arg)
 * {
 *     OS_ERR err;
 *     uint8_t rx_state = 0;      // 状态机状态
 *     uint8_t rx_buffer[100];    // 接收缓冲区（任务私有，线程安全）
 *     uint8_t rx_index = 0;      // 接收位置
 *     
 *     while (1)
 *     {
 *         // 从队列读取1字节（阻塞等待，超时1秒）
 *         void *msg = OSQPend(&USART_Rx_Queue, 1000, OS_OPT_PEND_BLOCKING, 
 *                             NULL, NULL, &err);
 *         
 *         if (err == OS_ERR_NONE)  // 接收成功
 *         {
 *             uint8_t rx_data = (uint8_t)msg;
 *             
 *             // 状态机解析协议
 *             switch (rx_state)
 *             {
 *                 case 0:  // 等待包头 '['
 *                     if (rx_data == '[')
 *                     {
 *                         rx_state = 1;
 *                         rx_index = 0;
 *                     }
 *                     break;
 *                 
 *                 case 1:  // 接收数据
 *                     if (rx_data == ']')  // 包尾
 *                     {
 *                         rx_buffer[rx_index] = '\0';  // 添加结束符
 *                         rx_state = 0;
 *                         
 *                         // 处理接收到的数据包
 *                         printf("Received packet: %s\r\n", rx_buffer);
 *                         
 *                         // 命令解析
 *                         if (strcmp((char*)rx_buffer, "LED1") == 0)
 *                         {
 *                             LED1 = !LED1;
 *                         }
 *                         else if (strcmp((char*)rx_buffer, "STATUS") == 0)
 *                         {
 *                             printf("System OK\r\n");
 *                         }
 *                     }
 *                     else  // 数据内容
 *                     {
 *                         if (rx_index < sizeof(rx_buffer) - 1)  // 防止溢出
 *                         {
 *                             rx_buffer[rx_index++] = rx_data;
 *                         }
 *                         else  // 缓冲区满，重置状态
 *                         {
 *                             rx_state = 0;
 *                             printf("Buffer overflow!\r\n");
 *                         }
 *                     }
 *                     break;
 *             }
 *         }
 *         else if (err == OS_ERR_TIMEOUT)  // 超时
 *         {
 *             // 可以在这里做超时处理，例如重置状态机
 *             // rx_state = 0;
 *         }
 *     }
 * }
 * 
 * 
 * 【优点】
 * 1. 线程安全：rx_buffer 是任务私有变量，不会被中断修改
 * 2. 不丢数据：队列可以缓冲256字节（USART_RX_QUEUE_SIZE）
 * 3. 可扩展：可以轻松添加多个协议解析任务
 * 4. 符合RTOS设计：中断只负责接收，任务负责处理
 * 5. 有溢出保护：rx_index 有边界检查
 */

/*------------------------------------------------------------------------------
 * 4.3 改进方案2：使用信号量通知（适合简单场景）
 *----------------------------------------------------------------------------*/
/*
 * 【设计思路】
 * - 中断中：将字节存入环形缓冲区，发送信号量通知任务
 * - 任务中：等待信号量，从环形缓冲区读取数据
 * - 优点：效率高，适合高速数据
 * 
 * 【实现代码】
 * 
 * // ========== 全局变量定义 ==========
 * #define RX_BUFFER_SIZE 256
 * typedef struct {
 *     uint8_t buffer[RX_BUFFER_SIZE];
 *     volatile uint16_t head;  // 写指针（中断修改）
 *     volatile uint16_t tail;  // 读指针（任务修改）
 * } RingBuffer_t;
 * 
 * RingBuffer_t uart_rx_ring;
 * OS_SEM uart_rx_sem;  // 信号量：通知有数据到达
 * 
 * 
 * // ========== 初始化函数 ==========
 * void USART_Config(void)
 * {
 *     OS_ERR err;
 *     
 *     // 创建信号量
 *     OSSemCreate(&uart_rx_sem, "UART RX Sem", 0, &err);
 *     
 *     // 初始化环形缓冲区
 *     uart_rx_ring.head = 0;
 *     uart_rx_ring.tail = 0;
 *     
 *     // ... 硬件初始化
 *     USART_HW_Config();
 * }
 * 
 * 
 * // ========== 中断函数 ==========
 * void USART1_IRQHandler(void)
 * {
 *     OS_ERR err;
 *     OSIntEnter();
 *     
 *     if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
 *     {
 *         uint8_t rx_data = USART_ReceiveData(USART1);
 *         
 *         // 计算下一个写位置
 *         uint16_t next_head = (uart_rx_ring.head + 1) % RX_BUFFER_SIZE;
 *         
 *         // 检查缓冲区是否满
 *         if (next_head != uart_rx_ring.tail)
 *         {
 *             uart_rx_ring.buffer[uart_rx_ring.head] = rx_data;
 *             uart_rx_ring.head = next_head;
 *             
 *             // 发送信号量通知任务
 *             OSSemPost(&uart_rx_sem, OS_OPT_POST_1, &err);
 *         }
 *         // else: 缓冲区满，丢弃数据
 *         
 *         USART_ClearITPendingBit(USART1, USART_IT_RXNE);
 *     }
 *     
 *     OSIntExit();
 * }
 * 
 * 
 * // ========== 任务函数 ==========
 * void uart_protocol_task(void *p_arg)
 * {
 *     OS_ERR err;
 *     uint8_t rx_state = 0;
 *     uint8_t rx_buffer[100];
 *     uint8_t rx_index = 0;
 *     
 *     while (1)
 *     {
 *         // 等待信号量（阻塞，超时1秒）
 *         OSSemPend(&uart_rx_sem, 1000, OS_OPT_PEND_BLOCKING, NULL, &err);
 *         
 *         if (err == OS_ERR_NONE)
 *         {
 *             // 从环形缓冲区读取数据
 *             while (uart_rx_ring.tail != uart_rx_ring.head)
 *             {
 *                 uint8_t rx_data = uart_rx_ring.buffer[uart_rx_ring.tail];
 *                 uart_rx_ring.tail = (uart_rx_ring.tail + 1) % RX_BUFFER_SIZE;
 *                 
 *                 // 协议解析（同方案1）
 *                 switch (rx_state)
 *                 {
 *                     case 0:
 *                         if (rx_data == '[')
 *                         {
 *                             rx_state = 1;
 *                             rx_index = 0;
 *                         }
 *                         break;
 *                     
 *                     case 1:
 *                         if (rx_data == ']')
 *                         {
 *                             rx_buffer[rx_index] = '\0';
 *                             rx_state = 0;
 *                             printf("Packet: %s\r\n", rx_buffer);
 *                         }
 *                         else if (rx_index < sizeof(rx_buffer) - 1)
 *                         {
 *                             rx_buffer[rx_index++] = rx_data;
 *                         }
 *                         else
 *                         {
 *                             rx_state = 0;
 *                         }
 *                         break;
 *                 }
 *             }
 *         }
 *     }
 * }
 * 
 * 【优点】
 * 1. 效率高：环形缓冲区减少内存拷贝
 * 2. 实时性好：信号量通知机制快速响应
 * 3. 适合高速数据：可以批量处理
 */

/*------------------------------------------------------------------------------
 * 4.4 改进方案3：保留原代码但增加保护（最小改动）
 *----------------------------------------------------------------------------*/
/*
 * 如果不想大改代码，可以增加以下保护措施：
 * 
 * void USART1_IRQHandler(void)
 * {
 *     OSIntEnter();
 *     
 *     static uint8_t RxState = 0;
 *     static uint8_t pRxPacket = 0;
 *     
 *     if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
 *     {
 *         uint8_t RxData = USART_ReceiveData(USART1);
 *         
 *         if (RxState == 0)
 *         {
 *             if (RxData == '[' && Serial_RxFlag == 0)
 *             {
 *                 RxState = 1;
 *                 pRxPacket = 0;
 *             }
 *         }
 *         else if (RxState == 1)
 *         {
 *             if (RxData == ']')
 *             {
 *                 RxState = 0;
 *                 Serial_RxPacket[pRxPacket] = '\0';
 *                 Serial_RxFlag = 1;
 *             }
 *             else
 *             {
 *                 // 【改进1】添加溢出保护
 *                 if (pRxPacket < sizeof(Serial_RxPacket) - 1)
 *                 {
 *                     Serial_RxPacket[pRxPacket] = RxData;
 *                     pRxPacket++;
 *                 }
 *                 else  // 缓冲区满，重置状态
 *                 {
 *                     RxState = 0;
 *                     pRxPacket = 0;
 *                 }
 *             }
 *         }
 *         
 *         USART_ClearITPendingBit(USART1, USART_IT_RXNE);
 *     }
 *     
 *     OSIntExit();
 * }
 * 
 * // 任务中使用临界区保护
 * void protocol_task(void *p_arg)
 * {
 *     OS_ERR err;
 *     char local_buffer[100];
 *     
 *     while (1)
 *     {
 *         if (Serial_RxFlag == 1)
 *         {
 *             // 【改进2】进入临界区，防止中断修改数据
 *             OS_CRITICAL_ENTER();
 *             strcpy(local_buffer, Serial_RxPacket);
 *             Serial_RxFlag = 0;  // 清除标志
 *             OS_CRITICAL_EXIT();
 *             
 *             // 处理数据（在临界区外，不影响中断）
 *             printf("Received: %s\r\n", local_buffer);
 *         }
 *         
 *         OSTimeDly(10, OS_OPT_TIME_DLY, &err);
 *     }
 * }
 * 
 * 【缺点】
 * - 临界区会关闭中断，影响实时性
 * - 仍然存在数据覆盖风险（如果任务处理慢）
 * - 不是最佳实践
 */

/*------------------------------------------------------------------------------
 * 4.5 方案对比与选择建议
 *----------------------------------------------------------------------------*/
/*
 * +------------------+------------+------------+------------+------------+
 * |      特性        | 原始代码   | 方案1(队列)| 方案2(环形)| 方案3(临界)|
 * +------------------+------------+------------+------------+------------+
 * | 线程安全         | ❌ 不安全  | ✅ 安全    | ✅ 安全    | ⚠️  基本安全|
 * | 数据丢失风险     | ⚠️  中等   | ✅ 低      | ✅ 低      | ⚠️  中等   |
 * | 实时性           | ✅ 好      | ✅ 好      | ✅ 很好    | ⚠️  一般   |
 * | 代码复杂度       | ✅ 简单    | ⚠️  中等   | ❌ 复杂    | ✅ 简单    |
 * | 符合RTOS设计     | ❌ 不符合  | ✅ 符合    | ✅ 符合    | ❌ 不符合  |
 * | 可扩展性         | ❌ 差      | ✅ 好      | ✅ 好      | ❌ 差      |
 * | 改动工作量       | -          | ⚠️  中等   | ❌ 大      | ✅ 小      |
 * +------------------+------------+------------+------------+------------+
 * 
 * 【推荐选择】
 * 
 * 1. 新项目或有时间重构：选择方案1（消息队列）
 *    - 最符合uC-OS3设计理念
 *    - 代码清晰易维护
 *    - 线程安全有保障
 * 
 * 2. 高速数据场景（如传感器数据流）：选择方案2（环形缓冲区）
 *    - 效率最高
 *    - 实时性最好
 * 
 * 3. 时间紧急或只是学习：选择方案3（临界区保护）
 *    - 改动最小
 *    - 能快速解决问题
 *    - 但不推荐用于产品
 * 
 * 4. 原始代码：不推荐继续使用
 *    - 存在多任务安全隐患
 *    - 不符合RTOS最佳实践
 */

/*******************************************************************************
 * 五、常见问题与解决方案
 ******************************************************************************/

/*------------------------------------------------------------------------------
 * 问题1：源码中的中文注释显示乱码
 *----------------------------------------------------------------------------*/
/*
 * 原因：
 * - 文件编码不匹配（源文件可能是GBK编码，IDE使用UTF-8打开）
 * 
 * 解决方法：
 * 方法1：修改Keil编码设置
 *   - Edit -> Configuration -> Editor -> Encoding
 *   - 选择 "Chinese GB2312" 或 "System Default"
 *   - 重新打开文件
 * 
 * 方法2：转换文件编码
 *   - 使用Notepad++打开文件
 *   - 编码 -> 转为UTF-8编码（带BOM）
 *   - 保存
 * 
 * 方法3：查看本文档开头的"乱码注释翻译对照表"
 */

/*------------------------------------------------------------------------------
 * 问题2：printf无输出
 *----------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 * 问题2：printf无输出
 *----------------------------------------------------------------------------*/
/*
 * 原因：
 * 1. 未勾选 MicroLIB
 * 2. 未实现fputc函数
 * 3. 串口未初始化
 * 
 * 解决：
 * - Keil: Options -> Target -> Use MicroLIB (勾选)
 * - 包含 #include <stdio.h>
 * - 确保调用了USART_Config()
 */

/*------------------------------------------------------------------------------
 * 问题3：多任务printf输出乱码
 *----------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 * 问题3：多任务printf输出乱码
 *----------------------------------------------------------------------------*/
/*
 * 原因：
 * - 多个任务同时调用printf，数据交错
 * 
 * 解决：
 * - 在fputc中使用互斥锁保护（本项目已实现）
 * - 或者只在一个任务中使用printf
 */

/*------------------------------------------------------------------------------
 * 问题4：接收数据丢失
 *----------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 * 问题4：接收数据丢失
 *----------------------------------------------------------------------------*/
/*
 * 原因：
 * 1. 队列满（发送速度 > 处理速度）
 * 2. 中断优先级配置错误
 * 
 * 解决：
 * - 增大USART_RX_QUEUE_SIZE
 * - 提高接收任务优先级
 * - 检查NVIC优先级配置（必须>OS临界区优先级）
 */

/*------------------------------------------------------------------------------
 * 问题5：系统卡死或进入HardFault
 *----------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 * 问题5：系统卡死或进入HardFault
 *----------------------------------------------------------------------------*/
/*
 * 原因：
 * 1. 中断中未调用OSIntEnter/OSIntExit
 * 2. 中断中使用了阻塞API（如OSMutexPend）
 * 3. 栈溢出
 * 
 * 解决：
 * - 确保中断函数格式正确
 * - 中断中只使用POST类API
 * - 增大任务栈大小
 */

/*------------------------------------------------------------------------------
 * 问题6：波特率不准确
 *----------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 * 问题6：波特率不准确
 *----------------------------------------------------------------------------*/
/*
 * 原因：
 * - 系统时钟配置错误
 * 
 * 解决：
 * - 检查SystemInit()中的时钟配置
 * - 使用示波器测量实际波特率
 * - 常用波特率：9600、115200、460800
 */

/*******************************************************************************
 * 六、进阶优化方案
 ******************************************************************************/

/*******************************************************************************
 * 六、进阶优化方案
 ******************************************************************************/

/*------------------------------------------------------------------------------
 * 优化1：使用DMA发送（提高效率）
 *----------------------------------------------------------------------------*/
/*
 * 步骤：
 * 1. 配置DMA通道（USART1_TX -> DMA2_Stream7_Channel4）
 * 2. 使能USART的DMA发送请求
 * 3. 发送时启动DMA传输，中断中释放信号量
 * 4. 任务等待信号量确认发送完成
 * 
 * 优点：
 * - CPU不参与数据搬运，效率高
 * - 适合大量数据发送
 */

/*------------------------------------------------------------------------------
 * 优化2：使用DMA接收（减少中断次数）
 *----------------------------------------------------------------------------*/
/*
 * 步骤：
 * 1. 配置DMA循环模式接收到缓冲区
 * 2. 使能USART空闲中断（IDLE）
 * 3. 空闲中断中计算接收长度并处理
 * 
 * 优点：
 * - 减少中断次数（原来每字节1次，现在每帧1次）
 * - 适合接收不定长数据包
 */

/*------------------------------------------------------------------------------
 * 优化3：实现环形缓冲区（替代队列）
 *----------------------------------------------------------------------------*/
/*
 * 优点：
 * - 减少内存碎片
 * - 提高实时性
 * 
 * 实现：
 * typedef struct {
 *     uint8_t buf[256];
 *     uint16_t head;
 *     uint16_t tail;
 * } RingBuffer_t;
 */

/*******************************************************************************
 * 七、完整配置检查清单
 ******************************************************************************/

/*******************************************************************************
 * 七、完整配置检查清单
 ******************************************************************************/

/*
 * [ ] 1. 硬件连接
 *     - PA9(TX) 连接 USB转TTL的RX
 *     - PA10(RX) 连接 USB转TTL的TX
 *     - GND共地
 * 
 * [ ] 2. 时钟配置
 *     - RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
 *     - RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
 * 
 * [ ] 3. GPIO配置
 *     - PA9: GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP
 *     - PA10: GPIO_Mode_AF, GPIO_PuPd_UP
 *     - GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
 *     - GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
 * 
 * [ ] 4. USART参数配置
 *     - USART_BaudRate = 115200
 *     - USART_WordLength = USART_WordLength_8b
 *     - USART_StopBits = USART_StopBits_1
 *     - USART_Parity = USART_Parity_No
 *     - USART_Mode = USART_Mode_Rx | USART_Mode_Tx
 * 
 * [ ] 5. 中断配置
 *     - USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
 *     - NVIC抢占优先级 > 4（uC-OS3要求）
 * 
 * [ ] 6. uC-OS3对象创建
 *     - OSMutexCreate(&USART_Mutex, ...);
 *     - OSQCreate(&USART_Rx_Queue, ...);
 * 
 * [ ] 7. 中断函数格式
 *     - OSIntEnter(); 在函数开头
 *     - OSIntExit(); 在函数结尾
 *     - 不使用阻塞API
 * 
 * [ ] 8. Keil配置（使用printf时）
 *     - Options -> Target -> Use MicroLIB (勾选)
 * 
 * [ ] 9. 上位机配置
 *     - 波特率：115200
 *     - 数据位：8
 *     - 停止位：1
 *     - 校验：无
 * 
 * [ ] 10. 文件编码检查（解决乱码）
 *     - Edit -> Configuration -> Editor -> Encoding 设置为 GB2312
 *     - 或使用Notepad++转换为UTF-8编码
 */

/*******************************************************************************
 * 八、测试代码示例
 ******************************************************************************/

/*
// 在main.c的bsp_init()中调用
void bsp_init(void)
{
    USART_Config();  // 初始化串口
    // ... 其他外设初始化
}

// 创建测试任务
void test_uart_task(void *p_arg)
{
    OS_ERR err;
    uint32_t count = 0;
    
    while (1)
    {
        // 测试发送
        printf("Test count: %d\r\n", count);
        
        // 测试接收（非阻塞查询）
        uint8_t rx = USART_Recv_Byte(10);  // 超时10ms
        if (rx != 0xFF)
        {
            printf("Received: %c\r\n", rx);
        }
        
        count++;
        OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
    }
}
*/

/*******************************************************************************
 * 八、测试代码示例
 ******************************************************************************/

/*
// 在main.c的bsp_init()中调用
void bsp_init(void)
{
    USART_Config();  // 初始化串口
    // ... 其他外设初始化
}

// 创建测试任务
void test_uart_task(void *p_arg)
{
    OS_ERR err;
    uint32_t count = 0;
    
    while (1)
    {
        // 测试发送
        printf("Test count: %d\r\n", count);
        
        // 测试接收（非阻塞查询）
        uint8_t rx = USART_Recv_Byte(10);  // 超时10ms
        if (rx != 0xFF)
        {
            printf("Received: %c\r\n", rx);
        }
        
        count++;
        OSTimeDly(1000, OS_OPT_TIME_DLY, &err);
    }
}
*/

/*******************************************************************************
 * 九、参考资料
 ******************************************************************************/

/*
 * 1. STM32F4xx中文参考手册 - 第25章 通用同步异步收发器(USART)
 * 2. uC-OS3官方文档 - Kernel API Reference
 * 3. STM32F4标准外设库用户手册 - stm32f4xx_usart.c
 * 4. Cortex-M4权威指南 - 中断优先级配置
 */

/*******************************************************************************
 * 十、uC-OS3消息队列深度学习与实战应用
 ******************************************************************************/

/*------------------------------------------------------------------------------
 * 10.1 消息队列基础概念
 *----------------------------------------------------------------------------*/
/*
 * 【什么是消息队列】
 * ==================
 * 消息队列（Message Queue）是一种任务间通信机制，用于在任务之间传递数据。
 * 
 * 核心特点：
 * 1. FIFO（先进先出）或LIFO（后进先出）
 * 2. 可以传递任意类型的数据（通过void*指针）
 * 3. 支持阻塞等待（任务可以睡眠等待消息）
 * 4. 支持超时机制
 * 5. 线程安全（OS内部有保护机制）
 * 
 * 【与其他通信机制的对比】
 * ========================
 * 
 * +------------------+------------------+------------------+------------------+
 * |      特性        |   消息队列       |   信号量         |   邮箱           |
 * +------------------+------------------+------------------+------------------+
 * | 数据传递         | ✅ 可以传递数据  | ❌ 只传递信号    | ✅ 传递1个消息   |
 * | 缓冲能力         | ✅ 多个消息      | ❌ 计数值        | ❌ 只能1个       |
 * | 适用场景         | 数据流传输       | 同步/互斥        | 单个事件通知     |
 * | 内存开销         | 中等             | 小               | 小               |
 * +------------------+------------------+------------------+------------------+
 * 
 * 【消息队列的工作原理】
 * ====================
 * 
 *   发送端（中断/任务）              消息队列                接收端（任务）
 *   ┌──────────┐                  ┌──────────┐           ┌──────────┐
 *   │          │  OSQPost()       │ [msg1]   │           │          │
 *   │  中断    │─────────────────>│ [msg2]   │           │  任务1   │
 *   │          │                  │ [msg3]   │<──────────│          │
 *   └──────────┘                  │ [msg4]   │ OSQPend() └──────────┘
 *                                 │ [...]    │
 *   ┌──────────┐                  │          │           ┌──────────┐
 *   │          │  OSQPost()       │          │           │          │
 *   │  任务2   │─────────────────>│          │           │  任务3   │
 *   │          │                  │          │<──────────│          │
 *   └──────────┘                  └──────────┘ OSQPend() └──────────┘
 *                                      ↑
 *                                      │
 *                                 队列大小限制
 *                              (USART_RX_QUEUE_SIZE)
 * 
 * 工作流程：
 * 1. 发送端调用 OSQPost() 将消息放入队列尾部
 * 2. 如果队列满，OSQPost() 返回错误
 * 3. 接收端调用 OSQPend() 从队列头部取出消息
 * 4. 如果队列空，OSQPend() 会阻塞等待（或超时返回）
 * 5. 当有新消息到达，OS会唤醒等待的任务
 */

/*------------------------------------------------------------------------------
 * 10.2 uC-OS3消息队列API详解
 *----------------------------------------------------------------------------*/
/*
 * 【API 1：创建消息队列】
 * ======================
 * 
 * void OSQCreate(OS_Q      *p_q,        // 队列控制块指针
 *                CPU_CHAR  *p_name,     // 队列名称（用于调试）
 *                OS_MSG_QTY max_qty,    // 队列最大消息数量
 *                OS_ERR    *p_err);     // 错误码返回
 * 
 * 参数说明：
 * - p_q: 指向队列控制块（需要预先定义全局变量）
 * - p_name: 队列名称字符串，方便调试识别
 * - max_qty: 队列能容纳的最大消息数量
 * - p_err: 返回错误码
 *   - OS_ERR_NONE: 创建成功
 *   - OS_ERR_CREATE_ISR: 不能在中断中创建
 *   - OS_ERR_ILLEGAL_CREATE_RUN_TIME: 运行时不允许创建
 * 
 * 使用示例：
 * OS_Q my_queue;
 * OS_ERR err;
 * OSQCreate(&my_queue, "My Queue", 100, &err);
 * if (err != OS_ERR_NONE) {
 *     // 处理错误
 * }
 * 
 * 注意事项：
 * 1. 必须在任务中调用，不能在中断中调用
 * 2. 通常在初始化任务或 USART_Config() 中创建
 * 3. 队列控制块必须是全局变量或静态变量
 * 
 * 
 * 【API 2：发送消息到队列（FIFO方式）】
 * ======================================
 * 
 * void OSQPost(OS_Q      *p_q,          // 队列控制块指针
 *              void      *p_void,       // 消息指针（要发送的数据）
 *              OS_MSG_SIZE msg_size,    // 消息大小（字节数）
 *              OS_OPT     opt,          // 选项
 *              OS_ERR    *p_err);       // 错误码返回
 * 
 * 参数说明：
 * - p_q: 队列控制块指针
 * - p_void: 消息内容（可以是指针、整数等）
 * - msg_size: 消息大小（通常用于统计，实际传递的是指针）
 * - opt: 发送选项
 *   - OS_OPT_POST_FIFO: 放到队列尾部（先进先出）
 *   - OS_OPT_POST_LIFO: 放到队列头部（后进先出）
 *   - OS_OPT_POST_ALL: 发送给所有等待任务
 * - p_err: 返回错误码
 *   - OS_ERR_NONE: 发送成功
 *   - OS_ERR_Q_MAX: 队列已满
 *   - OS_ERR_MSG_POOL_EMPTY: 消息池空
 * 
 * 使用示例（发送整数）：
 * uint8_t data = 0x55;
 * OS_ERR err;
 * OSQPost(&USART_Rx_Queue, (void*)data, sizeof(uint8_t), 
 *         OS_OPT_POST_FIFO, &err);
 * if (err == OS_ERR_Q_MAX) {
 *     // 队列满，处理丢包
 * }
 * 
 * 使用示例（发送指针）：
 * typedef struct {
 *     uint8_t cmd;
 *     uint8_t data[10];
 * } MyMsg_t;
 * 
 * MyMsg_t *p_msg = (MyMsg_t*)malloc(sizeof(MyMsg_t));
 * p_msg->cmd = 0x01;
 * OSQPost(&my_queue, (void*)p_msg, sizeof(MyMsg_t), 
 *         OS_OPT_POST_FIFO, &err);
 * 
 * 注意事项：
 * 1. 可以在中断和任务中调用
 * 2. 如果发送指针，接收端负责释放内存
 * 3. 发送小数据（如uint8_t）可以直接转为void*，不需要malloc
 * 
 * 
 * 【API 3：从队列接收消息（阻塞等待）】
 * ======================================
 * 
 * void *OSQPend(OS_Q       *p_q,        // 队列控制块指针
 *               OS_TICK     timeout,    // 超时时间（OS Tick）
 *               OS_OPT      opt,        // 选项
 *               OS_MSG_SIZE *p_msg_size,// 返回消息大小
 *               CPU_TS      *p_ts,      // 返回时间戳
 *               OS_ERR      *p_err);    // 错误码返回
 * 
 * 参数说明：
 * - p_q: 队列控制块指针
 * - timeout: 超时时间
 *   - 0: 永久等待
 *   - n: 等待n个OS Tick（1 Tick = 1ms，如果OS_CFG_TICK_RATE_HZ=1000）
 * - opt: 接收选项
 *   - OS_OPT_PEND_BLOCKING: 阻塞等待
 *   - OS_OPT_PEND_NON_BLOCKING: 非阻塞（立即返回）
 * - p_msg_size: 返回消息大小（可以传NULL）
 * - p_ts: 返回时间戳（可以传NULL）
 * - p_err: 返回错误码
 *   - OS_ERR_NONE: 接收成功
 *   - OS_ERR_TIMEOUT: 超时
 *   - OS_ERR_PEND_ISR: 不能在中断中调用
 * 
 * 返回值：
 * - 成功：返回消息指针（void*）
 * - 失败：返回NULL
 * 
 * 使用示例（接收整数）：
 * OS_ERR err;
 * void *msg = OSQPend(&USART_Rx_Queue, 1000, OS_OPT_PEND_BLOCKING, 
 *                     NULL, NULL, &err);
 * if (err == OS_ERR_NONE) {
 *     uint8_t data = (uint8_t)msg;  // 转换回整数
 *     printf("Received: 0x%02X\r\n", data);
 * } else if (err == OS_ERR_TIMEOUT) {
 *     printf("Timeout!\r\n");
 * }
 * 
 * 使用示例（接收指针）：
 * MyMsg_t *p_msg = (MyMsg_t*)OSQPend(&my_queue, 0, OS_OPT_PEND_BLOCKING, 
 *                                    NULL, NULL, &err);
 * if (err == OS_ERR_NONE) {
 *     printf("CMD: 0x%02X\r\n", p_msg->cmd);
 *     free(p_msg);  // 使用完后释放内存
 * }
 * 
 * 注意事项：
 * 1. 只能在任务中调用，不能在中断中调用
 * 2. 阻塞等待时，任务会进入睡眠，不占用CPU
 * 3. 如果接收的是指针，记得释放内存
 * 
 * 
 * 【API 4：查询队列状态（非阻塞）】
 * =================================
 * 
 * OS_MSG_QTY OSQPendAbort(OS_Q   *p_q,      // 队列控制块指针
 *                         OS_OPT  opt,      // 选项
 *                         OS_ERR *p_err);   // 错误码返回
 * 
 * 作用：中止所有等待该队列的任务
 * 
 * 
 * 【API 5：清空队列】
 * ==================
 * 
 * OS_MSG_QTY OSQFlush(OS_Q   *p_q,      // 队列控制块指针
 *                     OS_ERR *p_err);   // 错误码返回
 * 
 * 作用：清空队列中的所有消息
 * 返回值：清空前队列中的消息数量
 */

/*------------------------------------------------------------------------------
 * 10.3 消息队列在串口中的完整实现（方案1详细代码）
 *----------------------------------------------------------------------------*/
/*
 * 【步骤1：修改 usart.h 头文件】
 * ==============================
 * 
 * 在 usart.h 中添加：
 * 
 * // 协议解析任务相关定义
 * #define PROTOCOL_TASK_PRIO      10          // 任务优先级
 * #define PROTOCOL_TASK_STK_SIZE  512         // 任务栈大小
 * 
 * extern OS_TCB   Protocol_Task_TCB;          // 任务控制块
 * extern CPU_STK  Protocol_Task_STK[];        // 任务栈
 * 
 * // 函数声明
 * void uart_protocol_task(void *p_arg);       // 协议解析任务
 * 
 * 
 * 【步骤2：修改 usart.c 源文件】
 * ==============================
 * 
 * // ========== 全局变量定义 ==========
 * OS_MUTEX  USART_Mutex;
 * OS_Q      USART_Rx_Queue;
 * 
 * // 协议解析任务
 * OS_TCB   Protocol_Task_TCB;
 * CPU_STK  Protocol_Task_STK[PROTOCOL_TASK_STK_SIZE];
 * 
 * // 统计变量（可选）
 * uint32_t uart_rx_count = 0;      // 接收字节计数
 * uint32_t uart_rx_error = 0;      // 队列满错误计数
 * 
 * 
 * // ========== 初始化函数（保持不变） ==========
 * void USART_Config(void)
 * {
 *     OS_ERR err;
 *     
 *     // 创建互斥锁
 *     OSMutexCreate(&USART_Mutex, "USART1 Mutex", &err);
 *     
 *     // 创建消息队列
 *     OSQCreate(&USART_Rx_Queue, "USART1 Rx Queue", USART_RX_QUEUE_SIZE, &err);
 *     
 *     // 硬件配置
 *     USART_HW_Config();
 * }
 * 
 * 
 * // ========== 中断函数（简化版） ==========
 * void USART1_IRQHandler(void)
 * {
 *     OS_ERR err;
 *     OSIntEnter();
 *     
 *     if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
 *     {
 *         uint8_t rx_data = USART_ReceiveData(USART1);
 *         
 *         // 发送到队列（FIFO方式）
 *         OSQPost(&USART_Rx_Queue, (void*)rx_data, sizeof(uint8_t), 
 *                 OS_OPT_POST_FIFO, &err);
 *         
 *         // 统计
 *         if (err == OS_ERR_NONE) {
 *             uart_rx_count++;
 *         } else {
 *             uart_rx_error++;  // 队列满，丢包
 *         }
 *         
 *         USART_ClearITPendingBit(USART1, USART_IT_RXNE);
 *     }
 *     
 *     OSIntExit();
 * }
 * 
 * 
 * // ========== 协议解析任务 ==========
 * void uart_protocol_task(void *p_arg)
 * {
 *     OS_ERR err;
 *     uint8_t rx_state = 0;      // 状态机：0=等待包头，1=接收数据
 *     uint8_t rx_buffer[100];    // 接收缓冲区（任务私有）
 *     uint8_t rx_index = 0;      // 接收位置
 *     
 *     (void)p_arg;  // 避免编译警告
 *     
 *     printf("Protocol task started\r\n");
 *     
 *     while (1)
 *     {
 *         // 从队列读取1字节（阻塞等待，超时1秒）
 *         void *msg = OSQPend(&USART_Rx_Queue, 1000, OS_OPT_PEND_BLOCKING, 
 *                             NULL, NULL, &err);
 *         
 *         if (err == OS_ERR_NONE)  // 接收成功
 *         {
 *             uint8_t rx_data = (uint8_t)msg;
 *             
 *             // 状态机解析协议
 *             switch (rx_state)
 *             {
 *                 case 0:  // 等待包头 '['
 *                     if (rx_data == '[')
 *                     {
 *                         rx_state = 1;
 *                         rx_index = 0;
 *                     }
 *                     break;
 *                 
 *                 case 1:  // 接收数据
 *                     if (rx_data == ']')  // 包尾
 *                     {
 *                         rx_buffer[rx_index] = '\0';  // 添加结束符
 *                         rx_state = 0;
 *                         
 *                         // 处理接收到的数据包
 *                         printf("Received packet: [%s]\r\n", rx_buffer);
 *                         
 *                         // 命令解析
 *                         if (strcmp((char*)rx_buffer, "LED1") == 0)
 *                         {
 *                             LED1 = !LED1;
 *                             printf("LED1 toggled\r\n");
 *                         }
 *                         else if (strcmp((char*)rx_buffer, "LED0") == 0)
 *                         {
 *                             LED0 = !LED0;
 *                             printf("LED0 toggled\r\n");
 *                         }
 *                         else if (strcmp((char*)rx_buffer, "STATUS") == 0)
 *                         {
 *                             printf("System running OK\r\n");
 *                             printf("RX Count: %d, Error: %d\r\n", 
 *                                    uart_rx_count, uart_rx_error);
 *                         }
 *                         else
 *                         {
 *                             printf("Unknown command: %s\r\n", rx_buffer);
 *                         }
 *                     }
 *                     else  // 数据内容
 *                     {
 *                         if (rx_index < sizeof(rx_buffer) - 1)  // 防止溢出
 *                         {
 *                             rx_buffer[rx_index++] = rx_data;
 *                         }
 *                         else  // 缓冲区满，重置状态
 *                         {
 *                             rx_state = 0;
 *                             printf("Buffer overflow!\r\n");
 *                         }
 *                     }
 *                     break;
 *             }
 *         }
 *         else if (err == OS_ERR_TIMEOUT)  // 超时
 *         {
 *             // 超时处理（可选）
 *             // 例如：重置状态机，防止状态卡死
 *             if (rx_state != 0) {
 *                 rx_state = 0;
 *                 printf("Protocol timeout, reset state\r\n");
 *             }
 *         }
 *     }
 * }
 * 
 * 
 * 【步骤3：在 task.c 中创建协议解析任务】
 * ========================================
 * 
 * 在 start_task() 函数中添加：
 * 
 * void start_task(void *p_arg)
 * {
 *     OS_ERR err;
 *     // ... 其他初始化代码
 *     
 *     // 创建协议解析任务
 *     OSTaskCreate(&Protocol_Task_TCB,
 *                  "Protocol Task",
 *                  uart_protocol_task,
 *                  (void *)0,
 *                  PROTOCOL_TASK_PRIO,
 *                  &Protocol_Task_STK[0],
 *                  PROTOCOL_TASK_STK_SIZE / 10,
 *                  PROTOCOL_TASK_STK_SIZE,
 *                  0,
 *                  0,
 *                  (void *)0,
 *                  OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
 *                  &err);
 *     
 *     // ... 其他任务创建
 * }
 * 
 * 
 * 【步骤4：测试】
 * ==============
 * 
 * 1. 编译下载程序
 * 2. 打开串口助手（115200, 8N1）
 * 3. 发送测试命令：
 *    - 发送：[LED1]  -> LED1翻转
 *    - 发送：[LED0]  -> LED0翻转
 *    - 发送：[STATUS] -> 显示系统状态
 *    - 发送：[HELLO]  -> 显示"Unknown command"
 */

/*------------------------------------------------------------------------------
 * 10.4 消息队列进阶应用：传递结构体指针
 *----------------------------------------------------------------------------*/
/*
 * 【应用场景】
 * ===========
 * 当需要传递复杂数据（如传感器数据、命令包）时，可以传递结构体指针
 * 
 * 【实现步骤】
 * ===========
 * 
 * // ========== 定义数据结构 ==========
 * typedef struct {
 *     uint8_t  cmd;           // 命令类型
 *     uint8_t  len;           // 数据长度
 *     uint8_t  data[50];      // 数据内容
 *     uint32_t timestamp;     // 时间戳
 * } UartMsg_t;
 * 
 * // 消息池（预分配内存，避免频繁malloc/free）
 * #define MSG_POOL_SIZE 10
 * UartMsg_t msg_pool[MSG_POOL_SIZE];
 * uint8_t msg_pool_index = 0;
 * OS_MUTEX msg_pool_mutex;  // 保护消息池
 * 
 * 
 * // ========== 消息池管理函数 ==========
 * UartMsg_t* msg_alloc(void)
 * {
 *     OS_ERR err;
 *     UartMsg_t *p_msg = NULL;
 *     
 *     OSMutexPend(&msg_pool_mutex, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
 *     
 *     if (msg_pool_index < MSG_POOL_SIZE) {
 *         p_msg = &msg_pool[msg_pool_index++];
 *     }
 *     
 *     OSMutexPost(&msg_pool_mutex, OS_OPT_POST_NONE, &err);
 *     
 *     return p_msg;
 * }
 * 
 * void msg_free(UartMsg_t *p_msg)
 * {
 *     OS_ERR err;
 *     
 *     OSMutexPend(&msg_pool_mutex, 0, OS_OPT_PEND_BLOCKING, NULL, &err);
 *     
 *     if (msg_pool_index > 0) {
 *         msg_pool_index--;
 *     }
 *     
 *     OSMutexPost(&msg_pool_mutex, OS_OPT_POST_NONE, &err);
 * }
 * 
 * 
 * // ========== 中断中发送结构体 ==========
 * void USART1_IRQHandler(void)
 * {
 *     OS_ERR err;
 *     OSIntEnter();
 *     
 *     if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
 *     {
 *         static uint8_t rx_state = 0;
 *         static UartMsg_t *p_current_msg = NULL;
 *         
 *         uint8_t rx_data = USART_ReceiveData(USART1);
 *         
 *         switch (rx_state)
 *         {
 *             case 0:  // 等待包头
 *                 if (rx_data == 0xAA) {
 *                     p_current_msg = msg_alloc();
 *                     if (p_current_msg != NULL) {
 *                         rx_state = 1;
 *                     }
 *                 }
 *                 break;
 *             
 *             case 1:  // 接收命令
 *                 p_current_msg->cmd = rx_data;
 *                 rx_state = 2;
 *                 break;
 *             
 *             case 2:  // 接收长度
 *                 p_current_msg->len = rx_data;
 *                 p_current_msg->timestamp = OSTimeGet(&err);
 *                 rx_state = 3;
 *                 break;
 *             
 *             case 3:  // 接收数据
 *                 // ... 接收数据到 p_current_msg->data[]
 *                 // 接收完成后发送到队列
 *                 OSQPost(&USART_Rx_Queue, (void*)p_current_msg, 
 *                         sizeof(UartMsg_t), OS_OPT_POST_FIFO, &err);
 *                 rx_state = 0;
 *                 break;
 *         }
 *         
 *         USART_ClearITPendingBit(USART1, USART_IT_RXNE);
 *     }
 *     
 *     OSIntExit();
 * }
 * 
 * 
 * // ========== 任务中接收结构体 ==========
 * void uart_protocol_task(void *p_arg)
 * {
 *     OS_ERR err;
 *     
 *     while (1)
 *     {
 *         UartMsg_t *p_msg = (UartMsg_t*)OSQPend(&USART_Rx_Queue, 0, 
 *                                                OS_OPT_PEND_BLOCKING, 
 *                                                NULL, NULL, &err);
 *         
 *         if (err == OS_ERR_NONE)
 *         {
 *             printf("CMD: 0x%02X, Len: %d, Time: %d\r\n", 
 *                    p_msg->cmd, p_msg->len, p_msg->timestamp);
 *             
 *             // 处理数据
 *             switch (p_msg->cmd)
 *             {
 *                 case 0x01:
 *                     // 处理命令1
 *                     break;
 *                 case 0x02:
 *                     // 处理命令2
 *                     break;
 *             }
 *             
 *             // 释放消息
 *             msg_free(p_msg);
 *         }
 *     }
 * }
 */

/*------------------------------------------------------------------------------
 * 10.5 消息队列性能优化技巧
 *----------------------------------------------------------------------------*/
/*
 * 【技巧1：合理设置队列大小】
 * ==========================
 * 
 * 队列大小计算公式：
 * Queue_Size = (接收速率 × 最大处理延迟) + 安全余量
 * 
 * 示例：
 * - 波特率：115200 bps
 * - 每字节传输时间：约 87 us（8位数据+1起始+1停止=10位）
 * - 每秒最多接收：115200 / 10 = 11520 字节
 * - 任务处理延迟：最坏情况 100ms
 * - 需要缓冲：11520 × 0.1 = 1152 字节
 * - 加上安全余量：1152 × 1.5 = 1728 字节
 * - 建议设置：2048 字节（2的幂次，便于管理）
 * 
 * 
 * 【技巧2：批量处理提高效率】
 * ===========================
 * 
 * void uart_protocol_task(void *p_arg)
 * {
 *     OS_ERR err;
 *     uint8_t batch_buffer[64];  // 批量缓冲区
 *     uint8_t batch_count = 0;
 *     
 *     while (1)
 *     {
 *         // 批量读取（最多64字节）
 *         while (batch_count < sizeof(batch_buffer))
 *         {
 *             void *msg = OSQPend(&USART_Rx_Queue, 10,  // 超时10ms
 *                                 OS_OPT_PEND_BLOCKING, NULL, NULL, &err);
 *             
 *             if (err == OS_ERR_NONE) {
 *                 batch_buffer[batch_count++] = (uint8_t)msg;
 *             } else {
 *                 break;  // 超时或队列空，开始处理
 *             }
 *         }
 *         
 *         // 批量处理
 *         if (batch_count > 0) {
 *             process_batch_data(batch_buffer, batch_count);
 *             batch_count = 0;
 *         }
 *     }
 * }
 * 
 * 
 * 【技巧3：队列满时的处理策略】
 * =============================
 * 
 * 策略1：丢弃新数据（默认）
 * void USART1_IRQHandler(void)
 * {
 *     OS_ERR err;
 *     OSQPost(&USART_Rx_Queue, (void*)rx_data, 1, OS_OPT_POST_FIFO, &err);
 *     if (err == OS_ERR_Q_MAX) {
 *         // 队列满，新数据丢弃
 *         uart_rx_drop_count++;
 *     }
 * }
 * 
 * 策略2：丢弃旧数据（覆盖）
 * void USART1_IRQHandler(void)
 * {
 *     OS_ERR err;
 *     OSQPost(&USART_Rx_Queue, (void*)rx_data, 1, OS_OPT_POST_FIFO, &err);
 *     if (err == OS_ERR_Q_MAX) {
 *         // 队列满，丢弃最旧的数据
 *         void *old_msg = OSQPend(&USART_Rx_Queue, 0, 
 *                                 OS_OPT_PEND_NON_BLOCKING, NULL, NULL, &err);
 *         // 重新发送新数据
 *         OSQPost(&USART_Rx_Queue, (void*)rx_data, 1, OS_OPT_POST_FIFO, &err);
 *     }
 * }
 * 
 * 策略3：流控（发送XOFF）
 * void USART1_IRQHandler(void)
 * {
 *     OS_ERR err;
 *     OS_MSG_QTY msg_qty;
 *     
 *     // 检查队列使用率
 *     msg_qty = OSQPend(&USART_Rx_Queue, 0, OS_OPT_PEND_NON_BLOCKING, 
 *                       NULL, NULL, &err);
 *     
 *     if (msg_qty > USART_RX_QUEUE_SIZE * 0.8) {  // 超过80%
 *         USART_SendData(USART1, 0x13);  // 发送XOFF
 *     }
 * }
 * 
 * 
 * 【技巧4：多优先级队列】
 * ======================
 * 
 * 对于有优先级需求的场景，可以使用多个队列：
 * 
 * OS_Q high_priority_queue;   // 高优先级队列（紧急命令）
 * OS_Q normal_queue;          // 普通队列（数据）
 * 
 * void uart_protocol_task(void *p_arg)
 * {
 *     OS_ERR err;
 *     
 *     while (1)
 *     {
 *         // 优先处理高优先级队列
 *         void *msg = OSQPend(&high_priority_queue, 0, 
 *                             OS_OPT_PEND_NON_BLOCKING, NULL, NULL, &err);
 *         
 *         if (err == OS_ERR_NONE) {
 *             // 处理高优先级消息
 *             process_high_priority(msg);
 *         } else {
 *             // 处理普通队列
 *             msg = OSQPend(&normal_queue, 100, 
 *                          OS_OPT_PEND_BLOCKING, NULL, NULL, &err);
 *             if (err == OS_ERR_NONE) {
 *                 process_normal(msg);
 *             }
 *         }
 *     }
 * }
 */

/*------------------------------------------------------------------------------
 * 10.6 消息队列调试技巧
 *----------------------------------------------------------------------------*/
/*
 * 【调试技巧1：添加统计信息】
 * ==========================
 * 
 * typedef struct {
 *     uint32_t post_count;    // 发送计数
 *     uint32_t pend_count;    // 接收计数
 *     uint32_t drop_count;    // 丢包计数
 *     uint32_t max_usage;     // 最大使用量
 * } QueueStats_t;
 * 
 * QueueStats_t queue_stats = {0};
 * 
 * // 在中断中更新
 * void USART1_IRQHandler(void)
 * {
 *     OS_ERR err;
 *     OS_MSG_QTY current_qty;
 *     
 *     OSQPost(&USART_Rx_Queue, (void*)rx_data, 1, OS_OPT_POST_FIFO, &err);
 *     
 *     if (err == OS_ERR_NONE) {
 *         queue_stats.post_count++;
 *         
 *         // 更新最大使用量
 *         // current_qty = ... (需要通过其他方式获取)
 *         if (current_qty > queue_stats.max_usage) {
 *             queue_stats.max_usage = current_qty;
 *         }
 *     } else {
 *         queue_stats.drop_count++;
 *     }
 * }
 * 
 * // 在任务中打印统计
 * void print_queue_stats(void)
 * {
 *     printf("Queue Statistics:\r\n");
 *     printf("  Post: %d\r\n", queue_stats.post_count);
 *     printf("  Pend: %d\r\n", queue_stats.pend_count);
 *     printf("  Drop: %d\r\n", queue_stats.drop_count);
 *     printf("  Max Usage: %d / %d\r\n", 
 *            queue_stats.max_usage, USART_RX_QUEUE_SIZE);
 * }
 * 
 * 
 * 【调试技巧2：使用断言检查】
 * ==========================
 * 
 * #define ASSERT_QUEUE_OK(err) \
 *     if (err != OS_ERR_NONE) { \
 *         printf("Queue Error: %d at %s:%d\r\n", err, __FILE__, __LINE__); \
 *     }
 * 
 * // 使用
 * OSQPost(&USART_Rx_Queue, (void*)data, 1, OS_OPT_POST_FIFO, &err);
 * ASSERT_QUEUE_OK(err);
 * 
 * 
 * 【调试技巧3：Keil调试器查看队列】
 * ==================================
 * 
 * 在Keil调试模式下：
 * 1. View -> Watch Windows -> Watch 1
 * 2. 添加变量：USART_Rx_Queue
 * 3. 展开查看：
 *    - NbrEntries: 当前消息数量
 *    - NbrEntriesMax: 最大消息数量
 *    - InPtr: 写指针
 *    - OutPtr: 读指针
 */

/*------------------------------------------------------------------------------
 * 10.7 消息队列常见问题与解决
 *----------------------------------------------------------------------------*/
/*
 * 【问题1：队列频繁满，数据丢失】
 * ===============================
 * 
 * 原因：
 * - 队列太小
 * - 任务处理速度慢
 * - 任务优先级低，被其他任务抢占
 * 
 * 解决：
 * 1. 增大队列大小（USART_RX_QUEUE_SIZE）
 * 2. 提高协议解析任务优先级
 * 3. 优化任务处理逻辑，减少耗时操作
 * 4. 使用批量处理提高效率
 * 
 * 
 * 【问题2：内存泄漏】
 * ==================
 * 
 * 原因：
 * - 发送指针到队列后，接收端忘记释放内存
 * 
 * 解决：
 * 1. 使用消息池代替malloc/free
 * 2. 确保每个OSQPend后都有对应的free
 * 3. 使用内存检测工具（如Valgrind）
 * 
 * 
 * 【问题3：任务卡死在OSQPend】
 * ============================
 * 
 * 原因：
 * - 永久等待（timeout=0）但队列一直为空
 * - 中断未正常工作
 * 
 * 解决：
 * 1. 使用超时机制（timeout > 0）
 * 2. 检查中断是否正常触发
 * 3. 检查OSQPost是否成功
 * 
 * 
 * 【问题4：数据顺序错乱】
 * ======================
 * 
 * 原因：
 * - 误用OS_OPT_POST_LIFO（后进先出）
 * 
 * 解决：
 * - 串口数据必须使用OS_OPT_POST_FIFO（先进先出）
 */

/*******************************************************************************
 * 文档结束
 * 
 * 如有疑问，请参考项目源码：
 * - Drivers/BSP/usart/usart.h
 * - Drivers/BSP/usart/usart.c
 ******************************************************************************/
