#ifndef __task_manager_h
#define __task_manager_h

/************************* freertos header file *************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

void do_create_start_task(void);

void do_create_display_task(void);
TaskHandle_t get_task_display_handle(void);

void do_create_algorithm_task(void);
TaskHandle_t get_task_algorithm_task_handle(void);

void do_create_key_task(void);
TaskHandle_t get_task_key_task_handle(void);

void do_create_spi_rx_task(void);
TaskHandle_t get_task_spi_rx_task_handle(void);

void do_create_bsp_e2prom_task(void);
TaskHandle_t get_task_bsp_e2prom_handle(void);

void do_create_bsp_uart_task(void);
TaskHandle_t get_task_bsp_usrt_handle(void);

void do_create_watchdog_task(void);
TaskHandle_t get_task_watchdog_handle(void);
