/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_at24c512_driver.h
 * 
 * @par dependencies 
 * - stdio.h
 * - stdint.h
 * 
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

#ifndef __BSP_AT24C512_DRIVER_H__
#define __BSP_AT24C512_DRIVER_H__


//******************************** Includes *********************************//


#include <stdio.h>
#include <stdint.h>
#include "os_support.h"
//******************************** Includes *********************************//

//******************************** Defines **********************************//
#define HARDWARE_IIC

/*  函数返回状态枚举                    */
typedef enum
{
  AT24C512_OK                = 0,         /* Operation completed successfully.  */
  AT24C512_ERROR             = 1,         /* Run-time error without case matched*/
  AT24C512_ERRORTIMEOUT      = 2,         /* Operation failed with timeout      */
  AT24C512_ERRORRESOURCE     = 3,         /* Resource not available.            */
  AT24C512_ERRORPARAMETER    = 4,         /* Parameter error.                   */
  AT24C512_ERRORNOMEMORY     = 5,         /* Out of memory.                     */
  AT24C512_ERRORISR          = 6,         /* Not allowed in ISR context         */
  AT24C512_RESERVED          = 0x7FFFFFFF /* Reserved                           */
} at24c512_status_t;

//******************************** Defines **********************************//

//******************************** Declaring ********************************//

/* From Core Layer：  IIC Port        */
#ifndef HARDWARE_IIC     /* True : Software IIC */
typedef struct 
{
    at24c512_status_t (*pf_iic_init)          (void*);  /*   IIC init    interf.*/
    at24c512_status_t (*pf_iic_deinit)        (void*);  /*   IIC deinit  interf.*/
    at24c512_status_t (*pf_iic_start)         (void*);  /*   IIC start   interf.*/
    at24c512_status_t (*pf_iic_stop)          (void*);  /*   IIC stop    interf.*/
    at24c512_status_t (*pf_iic_wait_ack)      (void*);  /*   IIC w-ack   interf.*/
    at24c512_status_t (*pf_iic_send_ack)      (void*);  /*   IIC s-ack   interf.*/
    at24c512_status_t (*pf_iic_send_no_ack)   (void*);  /*   IIC s-n-ack interf.*/
    at24c512_status_t (*pf_iic_send_byte)     (void*,   /*   IIC s-byte  interf.*/
                                            const    uint8_t);
    at24c512_status_t (*pf_iic_receive_byte)  (void*,   /*   IIC r-byte  interf.*/
                                            uint8_t * const );  
    at24c512_status_t (*pf_critical_enter)     (void);  /* enter critical state.*/
    at24c512_status_t (*pf_critical_exit )     (void);  /* exit  critical state.*/
} iic_driver_interface_t;
#endif /* End of HARDWARE_IIC */

#ifdef HARDWARE_IIC     /* True : Hardware IIC */
typedef struct
{
    void *hi2c;             /* hi2c pointer to a I2C_HandleTypeDef structure */
    at24c512_status_t (*pf_iic_init)      (void *);   /* IIC init    interface */
    at24c512_status_t (*pf_iic_deinit)    (void *);   /* IIC deinit  interface */

    at24c512_status_t (*pf_iic_mem_write)(void *hi2c,
                                        uint16_t dst_address, 
                                        uint16_t mem_addr, 
                                        uint16_t mem_size, 
                                        uint8_t  *p_data, 
                                        uint16_t size, 
                                        uint32_t timeout);
    
    at24c512_status_t (*pf_iic_mem_read) (void *hi2c, 
                                        uint16_t dst_address, 
                                        uint16_t mem_addr, 
                                        uint16_t mem_size, 
                                        uint8_t  *p_data, 
                                        uint16_t size, 
                                        uint32_t timeout);
    // Use DMA to asynchronously read data from the I2C device's registers.                                    
    at24c512_status_t (*pf_iic_mem_read_dma) 
                                       (void *hi2c, 
                                        uint16_t dst_address, 
                                        uint16_t mem_addr, 
                                        uint16_t mem_size, 
                                        uint8_t  *p_data, 
                                        uint16_t size );
} iic_driver_interface_at24c512_t;
#endif //End of HARDWARE_IIC



/* Aht21_hal_driver instance class    */
typedef struct
{
    iic_driver_interface_at24c512_t *p_iic_driver_instance;
    timebase_interface_t     *p_timebase_instance;
    yield_interface_t           *p_yield_instance;

    at24c512_status_t (*pf_inst)(
                                       void * const      pat24c512_instance,
                     iic_driver_interface_at24c512_t * const piic_driver_instance,
                     timebase_interface_t   * const   ptimebase_instance,
                          yield_interface_t * const      pyield_instance);
         
    at24c512_status_t (*pf_init)         (void * const);   /*  AT24C512 init function */
    at24c512_status_t (*pf_deinit)       (void * const);   /*AT24C512 deinit function */
    at24c512_status_t (*pf_read_data)(void * const,        /*Read  DATA   function.*/
                                  uint16_t  const memaddr,
                                  uint8_t * const data,
                                  uint16_t  const len    );
    at24c512_status_t (*pf_write_data)     (void * const,  /*Write DATA    function*/
                                  uint16_t  const memaddr,
                                  uint8_t * const buffer,
                                  uint16_t  const len    );
} bsp_at24c512_driver_t;

/* Aht21_hal_driver instance class Inst. Function        */
at24c512_status_t at24c512_inst(                                          
                        bsp_at24c512_driver_t *          const paht21_instance,
                        iic_driver_interface_at24c512_t * const piic_driver_instance,
#ifdef OS_SUPPORTING
                        yield_interface_t *           const pyield_instance,
#endif //End of OS_SUPPORTING
                        timebase_interface_t *     const ptimebase_instance); 

//******************************** Declaring ********************************//


#endif /* end of __EC_BSP_AHT21_DRIVER_H__ */ 
