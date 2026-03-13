#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#if (SYS_TEST == 1)
#define RCT6 1
#define CGT6 0
#define USE_KEY_SW1 0
#define USE_4KEY 1
#define USE_FAKE_FPGA 1
#define USE_UART_LOG 1
#define USE_WATCHDOG 0
#else
#define RCT6 0
#define CGT6 1
#define USE_KEY_SW1 0
#define USE_FAKE_FPGA 0
#define USE_UART_LOG 0
#define USE_WATCHDOG 1
#endif

#endif
