#ifndef __BSP_LINK_TOTAL_H__
#define __BSP_LINK_TOTAL_H__

#include "bsp_st7789_link.h"
#include "bsp_at24c512_link.h"
#include "bsp_gp22_link.h"
//LCD相关bsp参数
extern bsp_st7789_driver_t st7789_driver_instance;

extern basic_oper_driver_interface_t st7789_oper_instance;

extern st7789_spi_driver_interface_t st7789_spi_instance;

extern st7789_timebase_interface_t st7789_time_instance;

extern st7789_yield_interface_t st7789_yield;


//E2PROM相关bsp参数
//代填充
extern iic_driver_interface_at24c512_t iic_driver_interface;
    
extern e2prom_handler_os_interface_t os_interface;
    

extern e2prom_input_arg_t e2prom_input_arg;


//GP22相关bsp参数

extern spi_driver_interface_t spi_driver_interface_gp22;


extern os_interface_t os_interface_gp22;

extern gp22_handler_input_args_t gp22_input_arg;

#endif

