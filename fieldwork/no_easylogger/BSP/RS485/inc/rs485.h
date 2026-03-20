#ifndef __RS485_H
#define __RS485_H

#include "sys.h"

#define RS485_CONTROL_GPIO_PORT GPIOC
#define RS485_CONTROL_GPIO_PIN  GPIO_PIN_14

void rs485_init(void);

#endif