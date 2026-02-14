#include "bsp.h"
#include "./LED/led.h"
#include "./usart/usart.h"
#include "./key/key.h"
#include "./exti/exti.h"
#include "./lcd/lcd.h"
#include "./lcd/lcd_init.h"
#include "./delay/delay.h"

void bsp_init(void)
{
    /* Configure NVIC priority group */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    /* Initialize delay function (168MHz) */
    delay_init(168);
    
    /* Initialize LCD with mutex */
    lcd_config();
    
    /* Configure external interrupt */
    exti_config();
    
    /* Initialize LED */
    led_init();
    
    /* Configure USART with mutex and queue */
    usart_config();
    
    /* Initialize KEY */
    KEY_Init();
}
