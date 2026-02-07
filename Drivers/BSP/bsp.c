#include "bsp.h"
#include "./LED/led.h"
#include "./usart/usart.h"
void bsp_init(void)
{
    led_init();             /* led初始化 */
    //USART_Config();
    
}
