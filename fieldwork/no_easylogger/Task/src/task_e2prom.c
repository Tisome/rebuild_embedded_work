#include "task_manager.h"

#include "at24cxx_driver.h"
#include "at24cxx_handler.h"
#include "iic.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

static TaskHandle_t task_e2prom_handle = NULL;

static void os_delay_ms_adapter(uint32_t const ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static e2prom_status_t os_queue_create_adapter(uint32_t const item_num,
                                               uint32_t const item_size,
                                               void **const queue_handle)
{
    if (queue_handle == NULL)
    {
        return E2PROM_ERROR_RESOURCE;
    }

    *queue_handle = xQueueCreate(item_num, item_size);
    return (*queue_handle == NULL) ? E2PROM_ERROR_NO_MEMORY : E2PROM_OK;
}

static e2prom_status_t os_queue_give_adapter(void *const queue_handle,
                                             void *const item,
                                             uint32_t timeout)
{
    if ((queue_handle == NULL) || (item == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    BaseType_t ok = xQueueSend((QueueHandle_t)queue_handle, item, timeout);
    return (ok == pdTRUE) ? E2PROM_OK : E2PROM_TIMEOUT;
}

static e2prom_status_t os_queue_take_adapter(void *queue_handle,
                                             void *buf,
                                             uint32_t timeout)
{
    if ((queue_handle == NULL) || (buf == NULL))
    {
        return E2PROM_ERROR_RESOURCE;
    }

    BaseType_t ok = xQueueReceive((QueueHandle_t)queue_handle, buf, timeout);
    return (ok == pdTRUE) ? E2PROM_OK : E2PROM_TIMEOUT;
}

static void task_e2prom_entry(void *pvParameter)
{
    (void)pvParameter;

    static iic_driver_t iic_driver;
    static at24cxx_driver_t at24cxx_driver;
    static at24cxx_dev_info_t at24cxx_dev;
    static e2prom_handler_os_interface_t os_if;
    static e2prom_input_arg_t input_arg;

    // iic_driver_init
    iic_driver_init(&iic_driver);
    (void)iic_driver.pf_iic_init(iic_driver.hi2c, (void *)E2PROM_IIC_INSTANCE, 100000U);

    // at24cxx_driver_init
    at24cxx_driver_init(&at24cxx_driver);

    // at24cxx_dev_info_init
    (void)at24cxx_driver.pf_init(&at24cxx_dev);

    // os_interface_init
    os_if.os_delay_ms = os_delay_ms_adapter;
    os_if.os_queue_create = os_queue_create_adapter;
    os_if.os_queue_give = os_queue_give_adapter;
    os_if.os_queue_take = os_queue_take_adapter;

    // input_arg_init
    input_arg.iic_driver_instance = &iic_driver;
    input_arg.dev_info_instance = &at24cxx_dev;
    input_arg.at24cxx_driver_instance = &at24cxx_driver;
    input_arg.os_interface_instance = &os_if;

    task_e2prom_handler(&input_arg);
}

TaskHandle_t get_bsp_e2prom_task_handle(void)
{
    return task_e2prom_handle;
}

void do_create_bsp_e2prom_task(void)
{
    BaseType_t xReturn = xTaskCreate(task_e2prom_entry,
                                     "task_e2prom",
                                     512,
                                     NULL,
                                     8,
                                     &task_e2prom_handle);
    (void)xReturn;
}
