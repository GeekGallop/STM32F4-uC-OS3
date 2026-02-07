#ifndef __LED_H
#define __LED_H	

#include "sys.h"


#define LED0 PAout(0)       // PA0
#define LED1 PAout(1)       // PA1

void led_init(void);        //初始化


#endif
