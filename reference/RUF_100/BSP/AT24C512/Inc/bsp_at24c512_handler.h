/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_at24c512_driver.h
 * 
 * @par dependencies 
 * - bsp_at24c512_driver.h
 * - elog.h
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
 
#ifndef __BSP_AT24C512_HANDLER_H__  /* Avoid repeated inclusion */
#define __BSP_AT24C512_HANDLER_H__


//******************************** Includes *********************************//

#include "bsp_at24c512_driver.h"

#include <stdbool.h>
#include <stdio.h>

//******************************** Includes *********************************//


//******************************** Defines **********************************//

#define AT24C512_HANDLER_DEBUG                      /* Is in Debug mode         */

//******************************** Defines **********************************//


//******************************** Typedefs *********************************//

/*   Emulation of return case        */
typedef enum
{
  E2PROM_OK                = 0,     /* Operation completed successfully.  */
  E2PROM_ERROR             = 1,     /* Run-time error without case matched*/
  E2PROM_ERRORTIMEOUT      = 2,     /* Operation failed with timeout      */
  E2PROM_ERRORRESOURCE     = 3,     /* Resource not available.            */
  E2PROM_ERRORPARAMETER    = 4,     /* Parameter error.                   */
  E2PROM_ERRORNOMEMORY     = 5,     /* Out of memory.                     */
  E2PROM_ERRORISR          = 6,     /* Not allowed in ISR context         */
  E2PROM_RESERVED  = 0x7FFFFFFF     /* Reserved  May check the caller     */
} e2prom_status_t;

typedef enum 
{
    E2PROM_READ  = 0,
    E2PROM_WRITE = 1
} e2prom_data_operation_t;

/*   Class of Event                  */
typedef struct
{
    uint16_t                     memAddr;        /* Temperature Reference    */
    uint8_t                        *data;        /* Humidity Reference       */
    uint16_t                         len;        /* Lifetime of data         */
    e2prom_data_operation_t         type;        /* The Data mode            */
} e2prom_event_t;

//******************************** Typedefs *********************************//


//**************************** Interface Structs ****************************//

/*   Interface from OS layer         */
typedef struct
{
    void   ( *os_delay_ms    )( uint32_t const ms);    /* Delay Interface    */
    e2prom_status_t ( *os_queue_create)(            /* Queue_Create Inter.*/
                                uint32_t const item_num,  
                                uint32_t const item_size, 
                                void ** const queue_handle);
                                
    e2prom_status_t ( *os_queue_put   )(            /* Queue_Send.  Inter.*/
                                void  * const queue_handle, 
                                void * const item, 
                                uint32_t timeout);
    e2prom_status_t ( *os_queue_get   )( 
                                void *queue_handle,    /* Queue_Recv.  Inter.*/
                                void *msg, 
                                uint32_t timeout);
} e2prom_handler_os_interface_t;

/*  Input Interface when instantiated */
typedef struct
{
    iic_driver_interface_at24c512_t   *iic_driver_interface;   /* IIC Driver Instance */
    timebase_interface_t       *timebase_interface;   /* Timebase            */
    e2prom_handler_os_interface_t *os_interface;   /* OS Interface        */
    yield_interface_t             *yield_interface;   /* CPU Yield Interface */
} e2prom_input_arg_t;

//**************************** Interface Structs ****************************//


//******************************** Classes **********************************//

/*  Handler Class Bsp Handler        */
typedef struct bsp_at24c512_handler
{
    /* Interface passed to driver layer */
    timebase_interface_t          *timebase_interface;
    iic_driver_interface_at24c512_t      *iic_driver_interface;
    yield_interface_t                *yield_interface;
    
    /* Interface from OS layer          */
    e2prom_handler_os_interface_t    *os_interface;

    /* Instance of driver instance      */
    bsp_at24c512_driver_t               *pat24c512_instance;

    /* Handler of event queue           */
    void                          *event_queue_handle;
    
} bsp_at24c512_handler_t;

//******************************** Classes **********************************//


//******************************** APIs *************************************//

/**
 * @brief A thread function that handles the temperature and humidity events.
 *
 * This function continuously checks for temperature and humidity updates and
 * sends them to the event queue. It also updates the last temperature and 
 * humidity.
 */
void e2prom_handler_thread(void *argument);

/**
 * @brief Initializes the temperature and humidity handler instance.
 *
 * This function takes in a handler instance and input 
 * arguments to initialize the temperature and humidity handler. It sets up the
 * necessary interfaces, creates a queue for handling events, and initializes 
 * the last temperature and humidity timestamps.
 *
 * @param handler_instance A pointer to the temperature and 
 *                         humidity handler instance.
 * @param input_arg        A pointer to the input arguments containing 
 *                         the required interfaces.
 *
 * @return An 8-bit unsigned integer representing the 
 *         status of the initialization.
 *         - 0: Initialization successful.
 *         - Non-zero: Initialization failed.
 */
e2prom_status_t bsp_temp_humi_xxx_handler_inst(
                        bsp_at24c512_handler_t *handler_instance,
                        e2prom_input_arg_t *input_arg);


/**
 * @brief Reads the temperature and humidity from the AHT21 sensor.
 *
 * This function sends an event to the AHT21 to read the temperature and
 * humidity. When the temperature and humidity data is ready, the 
 * callback function or provided pointers for temp. & humi. will be updated.
 * 
 * @param[in] event A pointer to the event made by the caller
 *
 * @return An 8-bit signed integer representing the status of the event passed.
 *         - 0: Successfully queued in the bsp handler.
 *         - Non-zero: Failed to pass.
 */
e2prom_status_t bsp_e2prom_write (uint16_t memAddr, uint8_t *data,   uint16_t size);
e2prom_status_t bsp_e2prom_read  (uint16_t memAddr, uint8_t *buffer, uint16_t size);

void Test_EEPROM(void);
//******************************** APIs *************************************//


#endif /* __EC_BSP_TEMP_HUMI_XXX_HANDLER_H__ */
