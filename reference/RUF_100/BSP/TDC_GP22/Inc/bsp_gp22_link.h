#ifndef __GP22_LINK
#define __GP22_LINK

#include "main.h"
#include "spi.h"
#include "task.h"
#include "semphr.h"
#include "FreeRTOS.h"
#include "bsp_gp22_handler.h"

static void delay_ms_gp22(uint32_t ms)
{
    delay_1ms(ms); 
}

static delay_interface_t delay_interface_gp22 = {
    .delay_ns = NULL,
    .delay_ms = delay_ms_gp22
};
static gp22_status_t spi_init_gp22(void *spi_handle)
{
    if (spi_handle == NULL) return GP22_ERR_PARAM;
    
    return GP22_OK;
}

static gp22_status_t spi_deinit_gp22(void *spi_handle)
{
    if (spi_handle == NULL) return GP22_ERR_PARAM;

    return GP22_OK;
}

static gp22_status_t spi_write_cmd_gp22(void *spi_handle, uint8_t cmd)
{
    if (spi_handle == NULL) return GP22_ERR_PARAM;
    
    if (spi_send_data_fullduplex((uint32_t)spi_handle, &cmd, 1) != SPI_OK)
    {
        return GP22_ERR_COMM;
    }
    return GP22_OK;
}

static gp22_status_t spi_write_reg_gp22(void *spi_handle, uint8_t reg, uint32_t val)
{
    if (spi_handle == NULL) return GP22_ERR_PARAM;
    
    uint8_t tx_buf[5] = {reg, (val >> 24) & 0xFF, (val >> 16) & 0xFF, 
                        (val >> 8) & 0xFF, val & 0xFF};
    if (spi_send_data_fullduplex((uint32_t)spi_handle, tx_buf, 5) != SPI_OK)
    {
        log_e("spi_write_reg_gp22: SPI_Transmit failed");
        return GP22_ERR_COMM;
    }
    return GP22_OK;
}

static gp22_status_t spi_read_reg_gp22(void *spi_handle, uint32_t reg, uint8_t *val,int len)
{
    if (spi_handle == NULL || val == NULL || 0 == len) return GP22_ERR_PARAM;
    
    uint8_t tx_buf[1] = {reg};
    uint8_t rx_buf[8] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77};
    
    if (spi_send_data_fullduplex((uint32_t)spi_handle, tx_buf, 1) != SPI_OK)
    {
        return GP22_ERR_COMM;
    }
    
    if (spi_receive_data((uint32_t)spi_handle, rx_buf, len) != SPI_OK)
    {
        return GP22_ERR_COMM;
    }
    
    for(int i=0;i<len;i++)
    {
        val[i]=rx_buf[i];
    }
    return GP22_OK;
}

static gp22_status_t spi_cs_select_gp22(int select)
{
	if(0 == select)
	{
		gpio_bit_reset(GP22_CS_GPIO_Port, GP22_CS_Pin);
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
	}
	else
	{
		gpio_bit_set(GP22_CS_GPIO_Port, GP22_CS_Pin);
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
	}
    return GP22_OK;
}



static gp22_status_t os_queue_create_gp22(uint32_t item_num, uint32_t item_size, 
                                                            void **queue_handle)
{
    *queue_handle = xQueueCreate(item_num, item_size);
    return (*queue_handle != NULL) ? GP22_OK : GP22_ERROR;
}

static gp22_status_t os_queue_put_gp22(void *queue_handle, void *item, uint32_t timeout)
{
    if (xQueueSend((QueueHandle_t)queue_handle, item, timeout) == pdPASS)
        return GP22_OK;
    return GP22_ERR_TIMEOUT;
}

static gp22_status_t os_queue_put_gp22_isr(void *queue_handle, void *item, 
                                    BaseType_t *pxHigherPriorityTaskWoken)
{
    if (queue_handle == NULL || item == NULL) {
        return GP22_ERR_PARAM; 
    }

    BaseType_t result = xQueueSendFromISR(
        (QueueHandle_t)queue_handle,  
        item,                         
        pxHigherPriorityTaskWoken     
    );

    if (result == pdPASS) {
        return GP22_OK;               
    } else {
        return GP22_ERROR;   
    }
}

static gp22_status_t os_queue_get_gp22(void *queue_handle, void *item, uint32_t timeout)
{
    if (xQueueReceive((QueueHandle_t)queue_handle, item, timeout) == pdPASS)
        return GP22_OK;
    return GP22_ERR_TIMEOUT;
}

static gp22_status_t os_queue_delete_gp22(void *queue_handle)
{
    vQueueDelete((QueueHandle_t)queue_handle);
    return GP22_OK;
}

static gp22_status_t os_semaphore_create_mutex_gp22(void **mutex_handle)
{
    *mutex_handle = xSemaphoreCreateMutex();
    return (*mutex_handle != NULL) ? GP22_OK : GP22_ERROR;
}

static gp22_status_t os_semaphore_delete_mutex_gp22(void *mutex_handle)
{
    vSemaphoreDelete(mutex_handle);
    return GP22_OK;
}

static gp22_status_t os_semaphore_lock_mutex_gp22(void *mutex_handle)
{
    if (xSemaphoreTake(mutex_handle, portMAX_DELAY) == pdPASS)
        return GP22_OK;
    return GP22_ERROR;
}

static gp22_status_t os_semaphore_unlock_mutex_gp22(void *mutex_handle)
{
    if (xSemaphoreGive(mutex_handle) == pdPASS)
        return GP22_OK;
    return GP22_ERROR;
}


#endif /* __ST7789_LINK */
