#include "bsp.h"
#include "./LED/led.h"
#include "./usart/usart.h"
#include "./key/key.h"
#include "./exti/exti.h"
void bsp_init(void)
{
    exti_config();//配置中断
    led_init();             /* led初始化 */
    USART_Config();
    KEY_Init();
}
