#include "at24cxx_handler.h"
#include "elog.h"
#include "stdlib.h"

static e2prom_handler_t *e2prom_instance = NULL;

at24cxx_status_t at24cxx_handler_inst(at24cxx_handler_t *instance)
{
}

e2prom_status_t e2prom_handler_inst(e2prom_handler_t *instance,
                                    e2prom_input_arg_t *arg)
{
    e2prom_status_t ret = E2PROM_OK;

    if (instance == NULL)
    {
        log_e("e2prom handler is null");
        return E2PROM_ERROR_RESOURCE;
    }

    if (instance->at24cxx_handler_instance == NULL)
    {
        log_e("e2prom handler -> at24cxx_driver is null");
        return E2PROM_ERROR_RESOURCE;
    }

    if (instance->os_interface_instance == NULL)
    {
        log_e("e2prom handler -> os_interface_handler is null");
        return E2PROM_ERROR_RESOURCE;
    }

    if (arg->at24cxx_driver_instance == NULL)
    {
        log_e("arg -> at24cxx_driver_instance is null");
        return E2PROM_ERROR_RESOURCE;
    }

    if (arg->dev_info_instance == NULL)
    {
        log_e("arg -> dev_info_instance is null");
        return E2PROM_ERROR_RESOURCE;
    }

    if (arg->iic_driver_instance == NULL)
    {
        log_e("arg -> iic_driver_instance is null");
        return E2PROM_ERROR_RESOURCE;
    }

    if (arg->os_interface_instance == NULL)
    {
        log_e("arg -> os_interface_instance is null");
        return E2PROM_ERROR_RESOURCE;
    }

    instance->at24cxx_handler_instance->p_iic_driver_instance = arg->iic_driver_instance;
    instance->at24cxx_handler_instance->p_at24cxx_dev_info = arg->dev_info_instance;
    instance->at24cxx_handler_instance->p_at24cxx_driver_instance = arg->at24cxx_driver_instance;
    instance->os_interface_instance = arg->os_interface_instance;
}

void task_e2prom_handler(void *arg)
{
    e2prom_input_arg_t *input_arg;
    e2prom_event_t *e2prom_event = NULL;
    e2prom_status_t e2prom_ret = E2PROM_OK;
    e2prom_instance = malloc(sizeof(e2prom_handler_t));

    at24cxx_driver_t at24cxx_driver_instance;
    e2prom_handler_os_interface_t os_interface_instance;

    if (arg == NULL)
    {
        log_e("task e2prom handler argument error");
        return;
    }

    input_arg = (e2prom_input_arg_t *)arg;

    e2prom_instance->at24cxx_driver_instance = &at24cxx_driver_instance;
    e2prom_instance->os_interface_instance = &os_interface_instance;

    e2prom_handler_inst(e2prom_instance, arg);
}
