#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#define SYS_TEST_MODE 1

#if SYS_TEST_MODE
#define USE_FAKE_FPGA 1
#define USE_UART_LOG 1
#define USE_WATCHDOG 0
#else
#define USE_FAKE_FPGA 0
#define USE_UART_LOG 0
#define USE_WATCHDOG 1
#endif

#endif

