#ifndef __WATCHDOG_H
#define __WATCHDOG_H

#include <stdint.h>

void watchdog_init_hw(void);
void watchdog_feed(void);

void watchdog_kick_algo(void);
void watchdog_kick_ui(void);

void watchdog_task(void *pvParameters);

#endif