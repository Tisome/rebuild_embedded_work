#include "link_total.h"
//LCD相关bsp参数
bsp_st7789_driver_t st7789_driver_instance;

basic_oper_driver_interface_t st7789_oper_instance = 
{
  .pf_spi_transmit = spi_transmit_data,
  .pf_spi_transmit_dma = spi_transmit_data_dma,
  .pf_write_cs_pin = gpio_write_cs_pin,
  .pf_write_dc_pin = gpio_write_dc_pin,
  .pf_write_reset_pin = gpio_write_reset_pin
};

st7789_spi_driver_interface_t st7789_spi_instance;

st7789_timebase_interface_t st7789_time_instance = 
{
  .pf_delay_no_os = delay_1ms,
  .pf_get_tick_count = get_delay_tick
};

st7789_yield_interface_t st7789_yield = 
{
  .pf_rtos_yield = vTaskDelay
};


//E2PROM相关bsp参数
iic_driver_interface_at24c512_t iic_driver_interface = 
{
    .hi2c                 =                EEPROM_I2C,       
    .pf_iic_init          =           iic_init_e2prom,
    .pf_iic_deinit        =         iic_deinit_e2prom,
    .pf_iic_mem_write     =             i2c_send_data,
    .pf_iic_mem_read      =          i2c_receive_data,                                 
    .pf_iic_mem_read_dma  =                      NULL
};
    
e2prom_handler_os_interface_t os_interface =  
{
    .os_delay_ms         = vTaskDelay,
    .os_queue_create     = os_queue_create_e2prom,
    .os_queue_put        = os_queue_put_e2prom,
    .os_queue_get        = os_queue_get_e2prom
};
    

e2prom_input_arg_t e2prom_input_arg = 
{
    .iic_driver_interface = &iic_driver_interface,
    .timebase_interface   = &timebase_interface,
    .os_interface         = &os_interface,
    .yield_interface      = &yield_interface
};


//GP22相关bsp参数

spi_driver_interface_t spi_driver_interface_gp22 = 
{
    .hspi              =                     (void*)SPI1,
    .init              =                   spi_init_gp22,
    .deinit            =                 spi_deinit_gp22,
    .write_cmd         =              spi_write_cmd_gp22,
    .write_reg         =              spi_write_reg_gp22,
    .read_reg          =               spi_read_reg_gp22,
    .cs_select         =              spi_cs_select_gp22
};


os_interface_t os_interface_gp22 = 
{
    .os_queue_create           =                     os_queue_create_gp22,
    .os_queue_put              =                        os_queue_put_gp22,
    .os_queue_put_isr          =                    os_queue_put_gp22_isr,  
    .os_queue_get              =                        os_queue_get_gp22,
    .os_queue_delete           =                     os_queue_delete_gp22,         
    .os_semaphore_create_mutex =           os_semaphore_create_mutex_gp22,
    .os_semaphore_delete_mutex =           os_semaphore_delete_mutex_gp22,
    .os_semaphore_lock_mutex   =             os_semaphore_lock_mutex_gp22,
    .os_semaphore_unlock_mutex =           os_semaphore_unlock_mutex_gp22
};

gp22_handler_input_args_t gp22_input_arg = 
{
    .spi_if                    =               &spi_driver_interface_gp22,
    .delay_if                  =                    &delay_interface_gp22,
    .timebase_if               =                      &timebase_interface,
    .os_if                     =                       &os_interface_gp22,
    .yield_if                  =                         &yield_interface
};
