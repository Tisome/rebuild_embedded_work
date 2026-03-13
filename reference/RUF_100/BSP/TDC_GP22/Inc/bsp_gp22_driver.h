/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_gp22_driver.h
 * 
 * @par dependencies 
 * - bsp_gp22_driver.h
 * - elog.h
 * - i2c.h
 * - stm32f1xx_hal.h
 * @author rnz | Rvision
 * 
 * @brief Provide the HAL APIs of TDC-GP22 and corresponding opetions.
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.0 2025-8-6
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/
#ifndef BSP_GP22_DRIVER_H
#define BSP_GP22_DRIVER_H

#include "bsp_gp22_reg.h"
#include "stdint.h"
#include "os_support.h"

extern double          g_tof_up[30];
extern double        g_tof_down[30];
extern int             sample_count;

#define OS_SUPPORTING


/* 状态返回值 */
typedef enum {
    GP22_OK             = 0,                                        /* 操作成功 */
    GP22_ERROR          = 1,                                        /* 通用错误 */
    GP22_ERR_TIMEOUT    = 2,                                        /* 超时错误 */
    GP22_ERR_PARAM      = 3,                                        /* 参数错误 */
    GP22_ERR_COMM       = 4                                         /* 通信错误 */
} gp22_status_t;

/* SPI接口函数指针结构体 */
typedef struct {
    void                                   *hspi;                  /* SPI 句柄 */
    gp22_status_t (*init)      (void *spi_handle);                  /*SPI初始化 */
    gp22_status_t (*deinit)    (void *spi_handle);               /* SPI反初始化 */
    gp22_status_t (*write_cmd)  (void *spi_handle,                 
                                     uint8_t cmd);                  /* 发送命令 */
    gp22_status_t (*write_reg) (void *spi_handle,                  
                       uint8_t reg, uint32_t val);                  /* 写寄存器 */
    gp22_status_t (*read_reg)   (void *spi_handle,                 
              uint8_t reg, uint32_t *val,int len);                  /* 读寄存器 */
    gp22_status_t (*cs_select)       (int select);                  /* 片选设置 */               
} spi_driver_interface_t;

/* 延时接口 */
typedef struct {
    void (*delay_ns)(uint32_t us);                                  /* 微秒延时 */
    void (*delay_ms)(uint32_t ms);                                  /* 毫秒延时 */
} delay_interface_t;                                          
                                                              

/* From OS Layer :       OS_Delay    */
#ifdef OS_SUPPORTING

typedef struct
{
    gp22_status_t (*os_queue_create)
                                    (uint32_t const item_num, 
                                    uint32_t const item_size, 
                                    void ** const queue_handle);
    gp22_status_t (*os_queue_put)
                                    (void * const queue_handle,
                                    void * const item, 
                                    uint32_t const timeout);
	gp22_status_t (*os_queue_put_isr)
									(void * const queue_handle,
									void * const item, 
									long * const HigherPriorityTaskWoken);
    gp22_status_t (*os_queue_get)
                                    (void * const queue_handle,
                                    void * const item, 
                                    uint32_t const timeout);
    gp22_status_t (*os_queue_delete)
                                    (void * const queue_handle);

    gp22_status_t (*os_semaphore_create_mutex) (void **mutex_handle);
    gp22_status_t (*os_semaphore_delete_mutex) (void * const mutex_handle);
    gp22_status_t (*os_semaphore_lock_mutex)   (void * const mutex_handle);
    gp22_status_t (*os_semaphore_unlock_mutex) (void * const mutex_handle);
} os_interface_t;
#endif

/* From OS   Layer：  OS_Delay        */
/* GP22测量数据结构体 */
typedef struct {
    /* 飞行时间测量 */    
    double                mea_tof_up[3];                      /* 上游飞行时间(μs) */
    double              mea_tof_down[3];                      /* 下游飞行时间(μs) */
    double                       tof_up;
    double                     tof_down;
    double                     tof_diff;                      
          /* 时差(μs) */
                                                                    
    /* 温度测量 */                                                    
    float                      temp_hot;                         /* 热水温度(°C) */
    float                     temp_cold;                          /* 冷水温度(°C)*/                               
    /* 信号质量 */                                     
    float                   sig_quality;                         /* 信号质量(0-1) */
    uint8_t                  bubble_det;        /* 气泡检测标志(0:无气泡,1:有气泡) */
    double                    empty_tube;            /* 空管检测标志(0:正常,1:空管) */
} gp22_data_t;

/* GP22驱动结构体 */
typedef struct bsp_gp22_driver {
    /* 接口指针 */
    spi_driver_interface_t                *spi_if;                      /* SPI接口 */
    delay_interface_t                   *delay_if;                     /* 延时接口 */
    timebase_interface_t             *timebase_if;                   /* 时间戳接口 */
                                                                   
    /* 硬件句柄 */                                                   
    void *spi_handle;                                                    /* SPI句柄 */
    
    /* 配置参数 */
    uint8_t meas_range;                     /* 测量范围(1:3.5ns-2.4μs, 2:500ns-4ms) */
    uint8_t res_mode;                           /* 分辨率模式(0:90ps,1:45ps,2:22ps) */
    
    /* 驱动函数指针 */
    gp22_status_t (*init)      (struct bsp_gp22_driver *driver);          /* 初始化 */
    gp22_status_t (*deinit)    (struct bsp_gp22_driver *driver);        /* 反初始化 */
    gp22_status_t (*config_reg) (struct bsp_gp22_driver *driver, 
                                     uint8_t reg, uint32_t val);      /* 配置寄存器 */
    gp22_status_t (*read_result)(struct bsp_gp22_driver *driver, 
                                    uint8_t reg, uint32_t *val);    /* 读结果寄存器 */
    gp22_status_t (*start_tof) (struct bsp_gp22_driver *driver);/* 启动飞行时间测量 */
    gp22_status_t (*start_temp)(struct bsp_gp22_driver *driver);    /* 启动温度测量 */
    
    gp22_status_t (*get_measure_data)
                                (struct bsp_gp22_driver *driver, 
                                             gp22_data_t *data);    /* 获取测量数据 */
                                
    gp22_status_t (*calibrate_tdc)  (struct bsp_gp22_driver *driver);   /* 校准TDC */
    gp22_status_t (*calibrate_clk)  (struct bsp_gp22_driver *driver);  /* 校准时钟 */
} bsp_gp22_driver_t;

/* 驱动初始化函数 */
gp22_status_t bsp_gp22_driver_inst(
                                bsp_gp22_driver_t *driver,
                           spi_driver_interface_t *spi_if,
                              delay_interface_t *delay_if,
                        timebase_interface_t *timebase_if,
                                        void *spi_handle);



#endif /* BSP_GP22_DRIVER_H */
