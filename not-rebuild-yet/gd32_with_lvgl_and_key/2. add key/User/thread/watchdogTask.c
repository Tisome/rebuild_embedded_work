#include "watchdogTask.h"
#include "gd32f30x.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* 各任务心跳计数器 */
static volatile uint32_t g_algo_heartbeat = 0;
static volatile uint32_t g_ui_heartbeat = 0;

/* ====================== 硬件FWDGT ====================== */
void watchdog_init_hw(void)
{
    /*
     * IRC40K ≈ 40kHz
     * 预分频 64 后，计数频率 ≈ 625Hz
     *
     * 若 reload = 1250
     * timeout ≈ 1250 / 625 = 2.0s
     */

    fwdgt_write_enable();
    fwdgt_config(1250, FWDGT_PSC_DIV64);
    fwdgt_counter_reload();
    fwdgt_enable();
}

void watchdog_feed(void)
{
    fwdgt_counter_reload();
}

/* ====================== 心跳上报 ====================== */
void watchdog_kick_algo(void)
{
    g_algo_heartbeat++;
}

void watchdog_kick_ui(void)
{
    g_ui_heartbeat++;
}

/* ====================== 看门狗任务 ====================== */
void vWatchdog_task(void *pvParameters)
{
    uint32_t last_algo = 0;
    uint32_t last_ui = 0;

    const TickType_t period = pdMS_TO_TICKS(500);

    /* 给系统启动留一点时间 */
    vTaskDelay(pdMS_TO_TICKS(3000));
    printf("watchdog working\r\n");

    while (1)
    {
        uint32_t cur_algo = g_algo_heartbeat;
        uint32_t cur_ui = g_ui_heartbeat;

        bool algo_ok = (cur_algo != last_algo);
        bool ui_ok = (cur_ui != last_ui);

        last_algo = cur_algo;
        last_ui = cur_ui;

        /* 第一版建议先只把核心链路纳入硬判定 */
        if (algo_ok && ui_ok)
        {
            watchdog_feed();
        }
        else
        {
            watchdog_feed();
            printf("the watch dog need to be feed!\r\n");
            /* 故意不喂狗，等待FWDGT复位 */
        }

        vTaskDelay(period);
    }
}