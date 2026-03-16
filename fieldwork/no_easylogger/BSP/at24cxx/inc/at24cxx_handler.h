#ifndef __AT24CXX_HANDLER_H
#define __AT24CXX_HANDLER_H

#include "at24cxx_driver.h"

typedef enum {
    E2PROM_OK              = 0,
    E2PROM_BUSY            = 1,
    E2PROM_TIMEOUT         = 2,
    E2PROM_ERROR           = 3,
    E2PROM_ERROR_RESOURCE  = 4,
    E2PROM_ERROR_PARAMETER = 5,
    E2PROM_ERROR_NO_MEMORY = 6,
    E2PROM_RESERVED        = 7
} e2prom_status_t;

typedef enum {
    E2PROM_READ  = 0,
    E2PROM_WRITE = 1
} e2prom_data_operation_t;

typedef struct
{
    uint16_t mem_addr;
    uint8_t *buf;
    uint16_t len;
    e2prom_data_operation_t event;
} e2prom_event_t;

typedef struct
{
    iic_driver_t *iic_driver_instance;
    at24cxx_dev_info_t *dev_info_instance;
    at24cxx_driver_t *at24cxx_driver_instance;
    e2prom_handler_os_interface_t *os_interface_instance;
} e2prom_input_arg_t;

typedef struct
{
    void (*os_delay_ms)(uint32_t const ms);
    e2prom_status_t (*os_queue_create)(uint32_t const item_num,
                                       uint32_t const item_size,
                                       void **const queue_handle);
    e2prom_status_t (*os_queue_give)(void *const queue_handle,
                                     void *const item,
                                     uint32_t timeout);
    e2prom_status_t (*os_queue_take)(void *queue_handle,
                                     void *buf,
                                     uint32_t timeout);
} e2prom_handler_os_interface_t;

typedef struct
{
    at24cxx_handler_t *at24cxx_handler_instance;
    e2prom_handler_os_interface_t *os_interface_instance;
} e2prom_handler_t;

#endif