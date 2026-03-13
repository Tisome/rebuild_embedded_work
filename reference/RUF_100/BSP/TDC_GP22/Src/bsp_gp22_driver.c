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
 

//******************************** Includes *********************************//
#include "bsp_gp22_driver.h"
#include "bsp_gp22_reg.h"
#include <string.h>
#include "elog.h"
#include "bsp_gp22_handler.h"
#include "main.h"
extern bsp_gp22_handler_t *g_gp22_handler;
#define GP22_LOOP_CHOESE_1       1
#define GP22_LOOP_CHOESE_2       2


double                g_tof_up[30];
double              g_tof_down[30];
int sample_count               = 0;
static int gp22_loop           = 1;



static gp22_status_t bsp_gp22_deinit(bsp_gp22_driver_t *driver) 
{
    if (!driver || !driver->spi_if || !driver->spi_if->deinit) 
    {
        return GP22_ERR_PARAM;
    }
    return driver->spi_if->deinit(driver->spi_handle);
}

static gp22_status_t bsp_gp22_config_reg(bsp_gp22_driver_t *driver,
                                         uint8_t reg, uint32_t val) 
{
    if (!driver || !driver->spi_if || !driver->spi_if->write_reg || 
                                        reg > GP22_CONF_REG6) 
    {
        log_e("GP22 Config Reg: Invalid parameters");
        return GP22_ERR_PARAM;
    }
    driver->spi_if->cs_select(0);  
    gp22_status_t   ret = driver->spi_if->write_reg(driver->spi_handle, 
                                        GP22_WRITE_REG_CMD | reg, val);
    driver->spi_if->cs_select(1); 
    // driver->delay_if->delay_ms(5); // 等待寄存器配置生效
    return ret;
}

static gp22_status_t bsp_gp22_read_result(bsp_gp22_driver_t *driver, uint8_t reg, uint32_t *val) {
    __disable_irq();
    if (NULL == driver || NULL == driver->spi_if || NULL == driver->spi_if->read_reg || NULL == val || reg > 8) {
        log_e("GP22 Read Result: Invalid parameters");
        __enable_irq();
        return GP22_ERR_PARAM;
    }

    gp22_status_t status = GP22_OK;
    uint8_t temp_val[4]; 

    driver->spi_if->cs_select(0);  

    status = driver->spi_if->read_reg(driver->spi_handle, GP22_READ_REG_CMD | reg, temp_val, 4);
    driver->spi_if->cs_select(1);  

    if (status != GP22_OK) {
        log_e("GP22 Read Result: SPI read error for reg 0x%02X", reg);
        __enable_irq();
        return status;
    }


    *val = (temp_val[0] << 24) | (temp_val[1] << 16) | (temp_val[2] << 8) | temp_val[3];

    __enable_irq();
    return GP22_OK;
}

static gp22_status_t bsp_gp22_read_id(bsp_gp22_driver_t *driver, uint8_t val[8]) 
{
    if (NULL == driver                   || NULL == driver->spi_if || 
        NULL == driver->spi_if->read_reg || NULL == val ) 
    {
        log_e("GP22 Read ID: Invalid parameters");
        return GP22_ERR_PARAM;
    }

    gp22_status_t status = GP22_OK;
    uint8_t rx_buf[8] = {0};

    driver->spi_if->cs_select(0);  
    status = driver->spi_if->read_reg(
        driver->spi_handle, 
        GP22_READ_ID_CMD,  
        rx_buf,            
        8      
    );
    driver->spi_if->cs_select(1);  

    if (status != GP22_OK) 
    {
        log_e("GP22 Read ID: SPI read error");
        return status;
    }
    memcpy(val, rx_buf, 8); 

    return GP22_OK;
}



static gp22_status_t bsp_gp22_start_tof(bsp_gp22_driver_t *driver) 
{  
    gp22_status_t    ret;

    if (!driver || !driver->spi_if || !driver->spi_if->write_cmd) 
    {
        return GP22_ERR_PARAM;
    }

    driver->spi_if->cs_select(0); 

    ret = driver->spi_if->write_cmd(driver->spi_handle, 
                                        GP22_INIT_CMD);
    driver->spi_if->cs_select(1);
    
    if (ret != GP22_OK) 
    {
        log_e("GP22: Failed to Init\n");
        return ret;
    }
    driver->spi_if->cs_select(0);
    ret = driver->spi_if->write_cmd(driver->spi_handle,
                                   GP22_START_TOF_CMD);

    driver->spi_if->cs_select(1);

    if (ret != GP22_OK) 
    {
        log_e("GP22: Failed to start TOF\n");
        return ret;
    }
    //测量开启前需上下拉启动测量
    gpio_bit_set(GP22_START_GPIO_Port,GP22_START_Pin);  
    gpio_bit_reset(GP22_START_GPIO_Port,GP22_START_Pin);
    return ret;
}

static gp22_status_t bsp_gp22_start_temp(bsp_gp22_driver_t *driver) 
{
    if (!driver || !driver->spi_if || !driver->spi_if->write_cmd) 
    {
        return GP22_ERR_PARAM;
    }

    driver->spi_if->cs_select(0); 
    gp22_status_t ret = driver->spi_if->write_cmd(driver->spi_handle, 
                                                GP22_START_TEMP_CMD);
    driver->spi_if->cs_select(1); 
    return ret;
}

static gp22_status_t bsp_gp22_calibrate_tdc(bsp_gp22_driver_t *driver) 
{
    if (!driver || !driver->spi_if || !driver->spi_if->write_cmd) 
    {
        return GP22_ERR_PARAM;
    }

    driver->spi_if->cs_select(0); 
    gp22_status_t ret = driver->spi_if->write_cmd(driver->spi_handle, 
                                                   GP22_CAL_TDC_CMD);
    driver->spi_if->cs_select(1); 

    return ret;
}

static inline gp22_status_t bsp_gp22_calibrate_clk(bsp_gp22_driver_t *driver) 
{
    if (!driver || !driver->spi_if || !driver->spi_if->write_cmd) 
    {
        return GP22_ERR_PARAM;
    }
    driver->spi_if->cs_select(0); 
    gp22_status_t ret = driver->spi_if->write_cmd(driver->spi_handle, 
                                                   GP22_CAL_RES_CMD);
    driver->spi_if->cs_select(1); 
    return ret;
}

static inline gp22_status_t bsp_gp22_get_measure_data(bsp_gp22_driver_t *driver, 
                                                       gp22_data_t *data) 
{
    // if (NULL == driver || NULL == data) 
    // {
    //     log_e("GP22 Get Measure Data: Invalid parameters");
    //     return GP22_ERR_PARAM;
    // }

    uint32_t res[2],stat;
    uint8_t id[8];
    gp22_status_t ret;

    // 读取状态寄存器检查错误
    // ret = bsp_gp22_read_result(driver, GP22_STAT_REG, &stat);
    // if (ret != GP22_OK) 
    // {
    //     return ret;
    // }
    //     log_d("GP22 Status Reg: 0x%x", stat);


    // bsp_gp22_read_id(driver,id);
    // for(int i = 0; i < 8; i++)
    // {
    //     log_d("GP22 ID Reg[%d]: 0x%x", i,id[i]);
    //     __NOP();
    //     __NOP();
    //     __NOP();
    //     __NOP();
    //     __NOP();
    // }
    // 检查空管和气泡
    // data->empty_tube = (stat & GP22_TDC_OVERFLOW_BIT) ? 1 : 0;
    
    //读取飞行时间测量结果
    if(GP22_LOOP_CHOESE_1 == gp22_loop)
    {
        for(int i = 0; i < 1; i++)
        {
            ret  =  bsp_gp22_read_result(driver, GP22_RES_REG0 + i, &res[0]);


            uint16_t integer_part         =          (res[0] >> 16) & 0xFFFF;  
            uint16_t fractional_part      =          (res[0] >>  0) & 0xFFFF;  
     
            double tmp_data               =                     integer_part+ 
                                                   fractional_part / 65536.0;
            data->mea_tof_up[i]           =                  tmp_data * 0.25;
 
        }             
        gp22_loop                         =               GP22_LOOP_CHOESE_2;

    }
    else if(GP22_LOOP_CHOESE_2 == gp22_loop)
    {
        for(int i = 0; i < 1; i++)
        {
            ret =   bsp_gp22_read_result(driver, GP22_RES_REG0 + i, &res[1]);

            uint16_t integer_part         =          (res[1] >> 16) & 0xFFFF; 
            uint16_t fractional_part      =          (res[1] >>  0) & 0xFFFF;  
        
            double tmp_data               =                     integer_part+ 
                                                   fractional_part / 65536.0;

            data->mea_tof_down[i]         =                  tmp_data * 0.25; 
        }

        gp22_loop                     =               GP22_LOOP_CHOESE_1;

        data->tof_diff =     data->mea_tof_up[0] - data->mea_tof_down[0]; 
        //滤掉卡周期时间
        while((data->tof_diff < 0.9      &&   data->tof_diff > 0.5) || 
              (data->tof_diff < -0.05  && data->tof_diff > -0.55)  )
        {
            if(data->tof_diff >   0.5)           data->mea_tof_down[0] += 0.5;
            if(data->tof_diff < -0.05)           data->mea_tof_down[0] -= 0.5;
            data->tof_diff =      data->mea_tof_up[0] - data->mea_tof_down[0];
        }

        data->tof_up                  =                  data->mea_tof_up[0]; 
        data->tof_down                =                data->mea_tof_down[0];
        data->tof_diff                =        data->tof_up - data->tof_down;   

            
        if(data->tof_diff > -0.05 && data->tof_diff < 0.4 && data->tof_up<250)
        {
            g_tof_up[sample_count % 30]   =                 data->tof_up;
            g_tof_down[sample_count % 30] =               data->tof_down;
            sample_count++;
        }
    }

    // 读取温度测量结果
    //ret  = bsp_gp22_read_result(driver, GP22_RES_REG2, &res[2]);
    // ret |= bsp_gp22_read_result(driver, GP22_RES_REG3, &res[3]);
    // if (ret != GP22_OK) 
    // {
    //     return ret;
    // }

    // // 温度转换（需要根据PT传感器类型计算）
    // data->temp_hot  = (float)res[2] * 0.01;  // 示例转换
    // data->temp_cold = (float)res[3] * 0.01;

    // 读取信号质量
    // ret = bsp_gp22_read_result(driver, GP22_PW1ST_REG, &pw1st);
    // if (ret != GP22_OK) 
    // {
    //     return ret;
    // }
    // data->sig_quality = (float)pw1st / 255.0f;
    // data->bubble_det  = (data->sig_quality < 0.3f) ? 1 : 0;
    return ret;
}
 static gp22_status_t bsp_gp22_reg_init(bsp_gp22_driver_t *driver) 
{
    gp22_status_t ret;


    ret =    bsp_gp22_config_reg(driver, GP22_CONF_REG0, 
        GP22_ANZ_FIRE_BIT(1)        | GP22_START_CLKHS_BIT(1)   | 
        GP22_ANZ_FAKE_BIT(1)        | GP22_SEL_ECLK_TMP_BIT(1)  |
        GP22_CALIBRATE_BIT(1)       | GP22_MESSB2_BIT(1)        |
        GP22_ID0_BIT(0x11)      
    );


    driver->delay_if->delay_ms(1);

    if (ret != GP22_OK) 
    {
        log_e("bsp_gp22_config_reg(GP22_CONF_REG0) failed");
        return ret;
    }

    ret =    bsp_gp22_config_reg(driver, GP22_CONF_REG1, 
        GP22_HIT2_BIT(2)            | GP22_HIT1_BIT(1)          | 
        GP22_RESERVED22(1)          | GP22_HITIN1_BIT(4)        |
        GP22_SEL_START_FIRE_BIT(0)  | GP22_ID1_BIT(0x22)
    );

    driver->delay_if->delay_ms(1);

    if (ret != GP22_OK) 
    {
        log_e("bsp_gp22_config_reg(GP22_CONF_REG1) failed");
        return ret;
    }

    ret =    bsp_gp22_config_reg(driver, GP22_CONF_REG2, 
        GP22_EN_INT_BIT(0x7)          | GP22_DELVAL1_BIT(0x1000) |
        GP22_ID2_BIT(0x33)
    );

    driver->delay_if->delay_ms(1);

    if (ret != GP22_OK) 
    {
        log_e("bsp_gp22_config_reg(GP22_CONF_REG2) failed");
        return ret;
    }

    ret =    bsp_gp22_config_reg(driver, GP22_CONF_REG3, 
        GP22_EN_AUTOCALC_MB2_BIT(1) | GP22_EN_ERR_VAL_BIT(1)    |  
        GP22_EN_FIRST_WAVE_BIT(1)   | GP22_SEL_TIMO_MB2_BIT(1)  |
        GP22_DELREL3_BIT(6)         | GP22_DELREL2_BIT(5)       |
        GP22_DELREL1_BIT(4)         | GP22_ID3_BIT(0x44)  
    );

    driver->delay_if->delay_ms(1);

    if (ret != GP22_OK) 
    {
        log_e("bsp_gp22_config_reg(GP22_CONF_REG3) failed");
        return ret;
    }

    ret =    bsp_gp22_config_reg(driver, GP22_CONF_REG4, 
        GP22_OFFSRNG2_BIT(1)        | GP22_OFFS_BIT(0x0F)       |
        GP22_ID4_BIT(0x55)
    );

    driver->delay_if->delay_ms(1);

    if (ret != GP22_OK) 
    {
        log_e("bsp_gp22_config_reg(GP22_CONF_REG4) failed");
        return ret;
    }

    ret =    bsp_gp22_config_reg(driver, GP22_CONF_REG5, 
        GP22_CONF_FIRE_BIT(2)    | GP22_ID5_BIT(0x66) 
    );

    driver->delay_if->delay_ms(1);

    if (ret != GP22_OK) 
    {
        log_e("bsp_gp22_config_reg(GP22_CONF_REG5) failed");
        return ret;
    }

    ret =    bsp_gp22_config_reg(driver, GP22_CONF_REG6, 
        GP22_EN_ANALOG_BIT(1)       | GP22_NEG_STOP_TEMP_BIT(1) | 
        GP22_TW2_BIT(3)             | GP22_FIREO_DEF_BIT(1)     |
        GP22_QUAD_RES_BIT(1)        | GP22_ID6_BIT(0x77)
    );
        
    driver->delay_if->delay_ms(1);

    if (ret != GP22_OK) 
    {
        log_e( "bsp_gp22_config_reg(GP22_CONF_REG6) failed");
        return ret;
    }

    return GP22_OK;
}
static gp22_status_t bsp_gp22_init(bsp_gp22_driver_t *driver) 
{
    gp22_status_t ret;

    gpio_bit_reset(GP22_START_GPIO_Port,GP22_START_Pin);

    if (!driver || !driver->spi_if || !driver->spi_if->init) 
    {
        return GP22_ERR_PARAM;
    }

    // 初始化SPI
    ret = driver->spi_if->init(driver->spi_handle);
    if (ret != GP22_OK) 
    {
        return ret;
    }

    // 上电复位
    driver->spi_if->cs_select(0); 
    ret = driver->spi_if->write_cmd(driver->spi_handle, GP22_POR_CMD);
    if (ret != GP22_OK) 
    {
        return ret;
    }
    driver->delay_if->delay_ms(1);

    driver->spi_if->cs_select(1); 

    // 初始化芯片
    driver->spi_if->cs_select(0); 
    ret = driver->spi_if->write_cmd(driver->spi_handle, GP22_INIT_CMD);
    if (ret != GP22_OK) 
    {
        return ret;
    }
    driver->spi_if->cs_select(1);
    driver->delay_if->delay_ms(1);

    // 配置默认寄存器
    // 测量范围2，使能校准，4MHz时钟
    ret = bsp_gp22_reg_init(driver);

    if (ret != GP22_OK) 
    {
        return ret;
    }
    // 根据分辨率模式配置
    // if (driver->res_mode == 0) 
    // {
    //     // 90ps
    //     ret = bsp_gp22_config_reg(driver, GP22_CONF_REG6, GP22_DOUBLE_RES_BIT(1));
    // } 
    // else if (driver->res_mode == 1) 
    // {
    //     // 双精度模式45ps
    //     ret = bsp_gp22_config_reg(driver, GP22_CONF_REG6, GP22_DOUBLE_RES_BIT(1));
    // } else if (driver->res_mode == 2) 
    // {
    //     // 四精度模式22ps
    //     ret = bsp_gp22_config_reg(driver, GP22_CONF_REG6, GP22_QUAD_RES_BIT(1));
    // }
    driver->spi_if->cs_select(0); 
    ret = driver->spi_if->write_cmd(driver->spi_handle, 
                                        GP22_INIT_CMD);
    driver->spi_if->cs_select(1); 
    
    return ret;
}
 gp22_status_t bsp_gp22_driver_inst(bsp_gp22_driver_t *driver,
    spi_driver_interface_t *spi_if,delay_interface_t *delay_if,
            timebase_interface_t *timebase_if,void *spi_handle) 
{
    if (NULL == driver || NULL == spi_if || NULL == delay_if || 
                     NULL == timebase_if || NULL == spi_handle) 
    {
        return GP22_ERR_PARAM;
    }

    memset(driver, 0, sizeof(bsp_gp22_driver_t));
    driver->spi_if            =                            spi_if;
    driver->delay_if          =                          delay_if;
    driver->timebase_if       =                       timebase_if;
    driver->spi_handle        =                        spi_handle;

    // 默认范围2
    driver->meas_range        =                                 2; 
    // 默认90ps分辨率
    driver->res_mode          =                                 2; 

    driver->init              =                     bsp_gp22_init;
    driver->deinit            =                   bsp_gp22_deinit;
    driver->config_reg        =               bsp_gp22_config_reg;
    driver->read_result       =              bsp_gp22_read_result;
    driver->start_tof         =                bsp_gp22_start_tof;
    driver->start_temp        =               bsp_gp22_start_temp;
    driver->get_measure_data  =         bsp_gp22_get_measure_data;
    driver->calibrate_tdc     =            bsp_gp22_calibrate_tdc;
    driver->calibrate_clk     =            bsp_gp22_calibrate_clk;

    if(bsp_gp22_init(driver) != GP22_OK) 
    {
        return GP22_ERROR;
    }

    return GP22_OK;
}





