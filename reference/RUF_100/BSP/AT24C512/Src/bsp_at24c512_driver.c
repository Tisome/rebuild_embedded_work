/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_at24c512_driver.h

 * @par dependencies 
 * - bsp_at24c512_driver.h
 * - elog.h
 * - i2c.h
 * - stm32f1xx_hal.h
 * @author rnz | Rvision
 * 
 * @brief Provide the HAL APIs of AT24C512 and corresponding opetions.
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.0 2025-7-16
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/
 

//******************************** Includes *********************************//
#include "bsp_at24c512_driver.h"
#include "elog.h"
#include "i2c.h"
#include "main.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define DEBUG
#define EEPROM_ADDR_WRITE               0xA0       // write address
#define EEPROM_ADDR_READ                0xA0       // read address
#define PAGE_SIZE                       32        // per page siez
#define AT24C512_NOT_INITED             0          // Not init. flag 
#define AT24C512_INITED                 1          // Not init. flag 
#define TIME_OUT_MS                     2000       // delay for reading and writing
#define EEPROM_MAX_ADDR                 0x0FFF

#define IS_INITED  (AT24C512_INITED == g_inited)   // if the device has been inited


#define AT24C512_WRITE_DATA(p_at24c512_instance, memAddr, p_data, len)\
                        p_at24c512_instance->p_iic_driver_instance->pf_iic_mem_write(\
                        p_at24c512_instance->p_iic_driver_instance->hi2c,\
                        EEPROM_ADDR_WRITE,\
                        memAddr,\
                        I2C_MEM_ADDR_16BIT,\
                        p_data,\
                        len,\
                        TIME_OUT_MS)

#define AT24C512_READ_DATA(p_at24c512_instance, memAddr, p_data, len)\
                        p_at24c512_instance->p_iic_driver_instance->pf_iic_mem_read(\
                        p_at24c512_instance->p_iic_driver_instance->hi2c,\
                        EEPROM_ADDR_READ,\
                        memAddr,\
                        I2C_MEM_ADDR_16BIT,\
                        p_data, \
                        len, \
                        TIME_OUT_MS)

//******************************** Defines **********************************//

//******************************** Variables ********************************//
static int8_t  g_inited       = AT24C512_NOT_INITED;    // Init. flag 

//******************************** Variables ********************************//

//******************************** Functions ********************************//

/**
 * @brief Function for reading AT24C512 Driver Layer Init.
 * 
 * @param[in] p_at24c512_instance : The pointer to object of bsp_at24c512_driver_t.
 * 
 * @return  at24c512_status_t.
 * 
 * */
static at24c512_status_t at24c512_init( bsp_at24c512_driver_t * const p_at24c512_instance)
{
    at24c512_status_t ret = AT24C512_OK;
    
    //log_d("at24c512_init start");
    
    if(NULL == p_at24c512_instance->p_iic_driver_instance            ||
       NULL == p_at24c512_instance->p_iic_driver_instance->pf_iic_init)
    {
        log_e("p_iic_driver_instance is NULL");
    }
    
    // Enter Critical Segmnet
#ifndef HARDWARE_IIC
    p_at24c512_instance->p_iic_driver_instance->pf_critical_enter();
#endif /* End of HARDWARE_IIC */

    // Init the IIC Interface  
    ret = p_at24c512_instance->p_iic_driver_instance->pf_iic_init(NULL);
    if(AT24C512_OK != ret) 
    {
        log_e("at24c512_init I2C init failed\r\n");
        return ret;
    }
    //log_d("at24c512_init iic_driver_init---------");
 
    //Exit Critical Segmnet
#ifndef HARDWARE_IIC
    p_at24c512_instance->p_iic_driver_instance->pf_critical_exit();
#endif /* End of HARDWARE_IIC */

    
    // Change the Flag of the internal flag
    g_inited = AT24C512_INITED;
    
    return AT24C512_OK;
}

/**
 * @brief Function for writing AT24C512 data from outside calling.
 * 
 * @param[in] p_at24c512_instance : The pointer to object of bsp_at24c512_driver_t.
 * @param[in] memAddr : The address that write
 * @param[in] data    : Writing data
 * @param[in] len     : The len of data
 * 
 * @return  at24c512_status_t.
 * 
 * */
static at24c512_status_t at24c512_write_data(\
                                   bsp_at24c512_driver_t * const p_at24c512_instance,\
                                       uint16_t memAddr, uint8_t *data, uint16_t len)
{
    bsp_at24c512_driver_t * at24c512_instance = p_at24c512_instance;
    at24c512_status_t ret  = AT24C512_OK;
    if(NULL ==at24c512_instance)
    {
        log_e("p_at24c512_instance is NULL\r\n");
        ret = AT24C512_ERRORPARAMETER;
        return ret;
    }
    uint16_t remaining     =         len;  
    if ((memAddr + len - 1) > EEPROM_MAX_ADDR) 
    {
        return AT24C512_ERRORNOMEMORY;
    }
    while (remaining > 0)
    {
        uint16_t page_offset     = memAddr % PAGE_SIZE;
        uint16_t space_in_page   = PAGE_SIZE - page_offset;
        uint16_t chunk           = (remaining > space_in_page) ?\
                                    space_in_page : remaining;

        if (NULL == at24c512_instance || NULL == at24c512_instance->p_iic_driver_instance) 
        {
            log_e("Invalid instance or I2C driver instance!");
            return AT24C512_ERRORPARAMETER;
        }
        ret = AT24C512_WRITE_DATA(at24c512_instance, memAddr, data, chunk);
        
        if (AT24C512_OK != ret) {
            log_e("EEPROM write after wait failed");
            return ret;
        }
        // at24c512_instance->p_yield_instance->pf_rtos_yield(100);
        delay_1ms(5);
        memAddr      += chunk;
        data         += chunk;
        remaining    -= chunk;
    }
    return ret;
}

static at24c512_status_t at24c512_read_data(\
                                   bsp_at24c512_driver_t * const p_at24c512_instance,\
                                       uint16_t memAddr, uint8_t *data, uint16_t len)
{
    bsp_at24c512_driver_t *  at24c512_instance = p_at24c512_instance;
    at24c512_status_t ret  = AT24C512_OK;
    if ((memAddr + len - 1) > EEPROM_MAX_ADDR) 
    {
        return AT24C512_ERRORNOMEMORY;
    }
    ret = AT24C512_READ_DATA(at24c512_instance, memAddr, data, len);
        
    return ret;
}

/**
 * @brief Function for reading AT24C512 Driver Layer Deinit.
 * 
 * @param[in] pat24c512_instance : The pointer to object of bsp_at24c512_driver_t.
 * 
 * @return  at24c512_status_t.
 * 
 * */
static at24c512_status_t at24c512_deinit(bsp_at24c512_driver_t * const p_at24c512_instance)
{
    g_inited = AT24C512_NOT_INITED;
    return AT24C512_OK;
}


/**
 * @brief Function for instantiate AT24C512 object.
 * 
 * @param[in] p_aht21_instance : The pointer to object of bsp_at24c512_driver_t.
 * @param[in] p_iic_driver_instance : The pointer to reference of \
                                                      iic_driver_interface_t.
 * @param[in] p_timebase_instance : The pointer to reference of \
                                                        timebase_interface_t.
 * @param[in] p_yield_instance : The pointer to reference of \
                                                           yield_interface_t.
 * 
 * @return  at24c512_status : [uint8_t] at24c512 status .
 * 
 * */
at24c512_status_t at24c512_inst(
                        bsp_at24c512_driver_t * const p_at24c512_instance,
                        iic_driver_interface_at24c512_t * const p_iic_driver_instance,
#ifdef OS_SUPPORTING
                        yield_interface_t * const p_yield_instance,
#endif //End of OS_SUPPORTING
                        timebase_interface_t * const p_timebase_instance
                        )
 {
     
    if( IS_INITED)
    {
        //Already initialized
        return AT24C512_ERRORRESOURCE;
    }
    
    // log_d("at24c512_inst start\r\n");
    uint8_t ret = 0;
    if(NULL == p_at24c512_instance || NULL == p_iic_driver_instance)
    {
        return AT24C512_ERRORPARAMETER;
    }


    p_at24c512_instance->p_iic_driver_instance = p_iic_driver_instance;
    p_at24c512_instance->p_timebase_instance = p_timebase_instance;
    p_at24c512_instance->p_yield_instance = p_yield_instance;

    p_at24c512_instance->pf_init = (at24c512_status_t (*)(void * const))\
                                                        at24c512_init;
    p_at24c512_instance->pf_deinit = (at24c512_status_t (*)(void * const))\
                                                        at24c512_deinit;
    
    p_at24c512_instance->pf_write_data = (at24c512_status_t (*)\
                               (void * const,uint16_t,uint8_t *,uint16_t))\
                                                       at24c512_write_data;
    p_at24c512_instance->pf_read_data = (at24c512_status_t (*)\
                               (void * const,uint16_t,uint8_t *,uint16_t))\
                                                        at24c512_read_data;
    /* call the init function */
    ret = at24c512_init(p_at24c512_instance);
    //log_d("at24c512_init ret = %d\r\n", ret);
    if(ret)
    {
        log_e("at24c512 init failed\r\n");
        return AT24C512_ERRORRESOURCE;

    }
    //log_d("at24c512_inst end\r\n");
    return AT24C512_OK;
 }   
 
 
 //******************************** Functions ********************************//
