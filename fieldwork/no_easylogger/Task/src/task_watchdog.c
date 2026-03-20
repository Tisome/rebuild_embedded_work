#include "task_manager.h"

#include "elog.h"

#define LOG_TAG "watchdog"
#define LOG_LVL ELOG_LVL_VERBOSE

static TaskHandle_t task_watchdog_handle = NULL;
static IWDG_HandleTypeDef hiwdg = {0};

/*
 * LSI 约 40kHz
 * timeout ≈ (Reload + 1) * Prescaler / LSI
 *
 * 这里选择：
 * Prescaler = 64
 * Reload    = 1249
 * timeout ≈ (1250 * 64) / 40000 = 2.0s
 */
static void watchdog_iwdg_init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
    hiwdg.Init.Reload = 1249;

    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        log_e("IWDG init failed");
        while (1)
        {
        }
    }

    log_i("IWDG started, timeout about 2s");
}

static void task_watchdog(void *pvParameter)
{
    (void)pvParameter;

    watchdog_iwdg_init();

    while (1)
    {
        if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK)
        {
            log_e("IWDG refresh failed");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

TaskHandle_t get_watchdog_task_handle(void)
{
    return task_watchdog_handle;
}

void do_create_watchdog_task(void)
{
    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate(task_watchdog,
                          "task_watchdog",
                          256,
                          NULL,
                          10,
                          &task_watchdog_handle);

    if (xReturn != pdPASS)
    {
        log_e("create watchdog task failed");
    }
}