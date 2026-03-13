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

#ifndef BSP_GP22_REG_H
#define BSP_GP22_REG_H

#include "stdint.h"

/* 配置寄存器地址 (Configuration Registers) */
#define GP22_CONF_REG0               0x00 // 配置寄存器 0
#define GP22_CONF_REG1               0x01 // 配置寄存器 1
#define GP22_CONF_REG2               0x02 // 配置寄存器 2
#define GP22_CONF_REG3               0x03 // 配置寄存器 3
#define GP22_CONF_REG4               0x04 // 配置寄存器 4
#define GP22_CONF_REG5               0x05 // 配置寄存器 5
#define GP22_CONF_REG6               0x06 // 配置寄存器 6

/* 读寄存器地址 (Read Registers) */
#define  GP22_RES_REG0               0x00  // 测量结果寄存器 0
#define  GP22_RES_REG1               0x01  // 测量结果寄存器 1
#define  GP22_RES_REG2               0x02  // 测量结果寄存器 2
#define  GP22_RES_REG3               0x03  // 测量结果寄存器 3
#define  GP22_STAT_REG               0x04  // 状态寄存器
#define  GP22_REG1_REG               0x05  // 用来测试通信
#define GP22_PW1ST_REG               0x08  // 脉冲宽度测量寄存器

/* SPI 操作码 (SPI Opcodes) */       
#define GP22_WRITE_REG_CMD           0x80 // 写入配置寄存器 (0x8x)
#define GP22_READ_REG_CMD            0xB0 // 读取寄存器 (0xBx)
#define GP22_READ_ID_CMD             0xB7 // 读取 ID 寄存器
#define GP22_READ_PW1ST_CMD          0xB8 // 读取 PW1ST 寄存器
#define GP22_EEPROM_WRITE_CMD        0xC0 // 写配置到 EEPROM
#define GP22_EEPROM_LOAD_CMD         0xF0 // 从 EEPROM 加载配置
#define GP22_EEPROM_CMP_CMD          0xC5 // 比较配置与 EEPROM
#define GP22_INIT_CMD                0x70 // 初始化命令
#define GP22_POR_CMD                 0x50 // 上电复位命令
#define GP22_START_TOF_CMD           0x01 // 启动飞行时间测量
#define GP22_START_TEMP_CMD          0x02 // 启动温度测量
#define GP22_CAL_RES_CMD             0x03 // 校准振荡器
#define GP22_CAL_TDC_CMD             0x04 // 校准 TDC
#define GP22_START_TOF_RST_CMD       0x05 // 重启飞行时间测量
#define GP22_START_TEMP_RST_CMD      0x06 // 重启温度测量

/* 配置寄存器 0 位定义 (CONF_REG0, 0x00) */
#define GP22_ANZ_FIRE_BIT(x)        ((x) << 28)   // 发射脉冲数 (31-28)
#define GP22_DIV_FIRE_BIT(x)        ((x) << 24)   // 脉冲分频系数 (27-24)
#define GP22_ANZ_PER_CALRES_BIT(x)  ((x) << 22)   // 校准周期数 (23-22)
#define GP22_DIV_CLKHS_BIT(x)       ((x) << 20)   // 高速时钟分频 (21-20)
#define GP22_START_CLKHS_BIT(x)     ((x) << 18)   // 晶振启动间隔 (19-18)
#define GP22_ANZ_PORT_BIT(x)        ((x) << 17)   // 温度端口数 (17)
#define GP22_TCYCLE_BIT(x)          ((x) << 16)   // 温度测量周期 (16)
#define GP22_ANZ_FAKE_BIT(x)        ((x) << 15)   // 温度热身测量数 (15)
#define GP22_SEL_ECLK_TMP_BIT(x)    ((x) << 14)   // 温度时钟选择 (14)
#define GP22_CALIBRATE_BIT(x)       ((x) << 13)   // 校准使能 (13)
#define GP22_NO_CAL_AUTO_BIT(x)     ((x) << 12)   // 自动校准关闭 (12)
#define GP22_MESSB2_BIT(x)          ((x) << 11)   // 测量范围选择 (11)
#define GP22_NEG_STOP2_BIT(x)       ((x) << 10)   // STOP2 反向 (10)
#define GP22_NEG_STOP1_BIT(x)       ((x) << 9)    // STOP1 反向 (9)
#define GP22_NEG_START_BIT(x)       ((x) << 8)    // START 反向 (8)
#define GP22_ID0_BIT(x)             ((x) << 0)    // ID0 (7-0)

/* 配置寄存器 1 位定义 (CONF_REG1, 0x01) */
#define GP22_HIT2_BIT(x)            ((x) << 28)   // ALU 操作数 2 (31-28)
#define GP22_HIT1_BIT(x)            ((x) << 24)   // ALU 操作数 1 (27-24)
#define GP22_EN_FAST_INIT_BIT(x)    ((x) << 23)   // 快速初始化 (23)
#define GP22_RESERVED22(x)          ((x) << 22)   // 快速初始化 (23)
#define GP22_HITIN2_BIT(x)          ((x) << 19)   // 通道 2 预期脉冲数 (21-19)
#define GP22_HITIN1_BIT(x)          ((x) << 16)   // 通道 1 预期脉冲数 (18-16)
#define GP22_CURR32K_BIT(x)         ((x) << 15)   // 32K 晶振电流 (15)
#define GP22_SEL_START_FIRE_BIT(x)  ((x) << 14)   // START 触发选择 (14)
#define GP22_SEL_TSTO2_BIT(x)       ((x) << 11)   // EN_START 功能 (13-11)
#define GP22_SEL_TSTO1_BIT(x)       ((x) << 8)    // FIRE_IN 功能 (10-8)
#define GP22_ID1_BIT(x)             ((x) << 0)    // ID1 (7-0)

/* 配置寄存器 2 位定义 (CONF_REG2, 0x02) */
#define GP22_EN_INT_BIT(x)          ((x) << 29)   // 中断使能 (31-29)
#define GP22_RFEDGE2_BIT(x)         ((x) << 28)   // 通道 2 边沿敏感 (28)
#define GP22_RFEDGE1_BIT(x)         ((x) << 27)   // 通道 1 边沿敏感 (27)
#define GP22_DELVAL1_BIT(x)         ((x) << 8)    // 延迟值 1 (26-8)
#define GP22_ID2_BIT(x)             ((x) << 0)    // ID2 (7-0)

/* 配置寄存器 3 位定义 (CONF_REG3, 0x03) */
#define GP22_EN_AUTOCALC_MB2_BIT(x) ((x) << 31)   // 自动计算使能 (31)
#define GP22_EN_ERR_VAL_BIT(x)      ((x) << 29)   // 溢出错误使能 (29)
#define GP22_EN_FIRST_WAVE_BIT(x)   ((x) << 30)   // 第一波检测使能 (30)
#define GP22_SEL_TIMO_MB2_BIT(x)    ((x) << 27)   // 溢出时间选择 (28-27)
#define GP22_DELREL3_BIT(x)         ((x) << 20)   // 相对延迟 3 (25-20)
#define GP22_DELREL2_BIT(x)         ((x) << 14)   // 相对延迟 2 (19-14)
#define GP22_DELREL1_BIT(x)         ((x) << 8)    // 相对延迟 1 (13-8)
#define GP22_ID3_BIT(x)             ((x) << 0)    // ID3 (7-0)

/* 配置寄存器 4 位定义 (CONF_REG4, 0x04) */
#define GP22_DIS_PW_BIT(x)          ((x) << 16)   // 脉冲宽度测量关闭 (16)
#define GP22_EDGE_FW_BIT(x)         ((x) << 15)   // 第一波边沿敏感 (15)
#define GP22_OFFSRNG2_BIT(x)        ((x) << 14)   // 偏移范围 2 (14)
#define GP22_OFFSRNG1_BIT(x)        ((x) << 13)   // 偏移范围 1 (13)
#define GP22_OFFS_BIT(x)            ((x) << 8)    // 比较器偏移 (12-8)
#define GP22_ID4_BIT(x)             ((x) << 0)    // ID4 (7-0)

/* 配置寄存器 5 位定义 (CONF_REG5, 0x05) */
#define GP22_CONF_FIRE_BIT(x)       ((x) << 29)   // 脉冲输出配置 (31-29)
#define GP22_EN_STARTNOISE_BIT(x)   ((x) << 28)   // 启动噪声使能 (28)
#define GP22_DIS_PHASESHIFT_BIT(x)   ((x) << 27)   // 相位偏移关闭 (27)
#define GP22_REPEAT_FIRE_BIT(x)     ((x) << 24)   // 脉冲重复次数 (26-24)
#define GP22_PHFIRE_BIT(x)          ((x) << 8)    // 脉冲相位 (23-8)
#define GP22_ID5_BIT(x)             ((x) << 0)    // ID5 (7-0)

/* 配置寄存器 6 位定义 (CONF_REG6, 0x06) */
#define GP22_EN_ANALOG_BIT(x)       ((x) << 31)   // 模拟部分使能 (31)
#define GP22_NEG_STOP_TEMP_BIT(x)   ((x) << 30)   // 温度信号反向 (30)
#define GP22_DA_KORR_BIT(x)         ((x) << 25)   // 比较器偏移校正 (28-25)
#define GP22_TW2_BIT(x)             ((x) << 22)   // 电容充电时间 (23-22)
#define GP22_EN_INT_EEPROM_BIT(x)   ((x) << 21)   // EEPROM 中断使能 (21)
#define GP22_START_CLKHS2_BIT(x)    ((x) << 20)   // 高速时钟启动高位 (20)
#define GP22_CYCLE_TEMP_BIT(x)      ((x) << 18)   // 温度测量周期系数 (19-18)
#define GP22_CYCLE_TOF_BIT(x)       ((x) << 16)   // 飞行时间周期系数 (17-16)
#define GP22_HZ60_BIT(x)            ((x) << 15)   // 50/60Hz 选择 (15)
#define GP22_FIREO_DEF_BIT(x)       ((x) << 14)   // 脉冲默认电平 (14)
#define GP22_QUAD_RES_BIT(x)        ((x) << 13)   // 四精度模式 (13)
#define GP22_DOUBLE_RES_BIT(x)      ((x) << 12)   // 双精度模式 (12)
#define GP22_TEMP_PORTDIR_BIT(x)    ((x) << 11)   // 温度端口方向 (11)
#define GP22_ANZ_FIRE_HIGH_BIT(x)   ((x) << 8)    // 发射脉冲数高位 (10-8)
#define GP22_ID6_BIT(x)             ((x) << 0)    // ID6 (7-0)

/* 状态寄存器位定义 (STAT_REG, 0x04) */
#define GP22_EEPROM_EQ_CREG_BIT      (1 << 15)           // EEPROM 与配置一致 (15)
#define GP22_EEPROM_DED_BIT          (1 << 14)           // EEPROM 多重错误 (14)
#define GP22_EEPROM_ERROR_BIT        (1 << 13)           // EEPROM 错误 (13)
#define GP22_TEMP_SHORT_BIT          (1 << 12)           // 温度传感器短路 (12)
#define GP22_TEMP_OPEN_BIT           (1 << 11)           // 温度传感器开路 (11)
#define GP22_PRE_OVERFLOW_BIT        (1 << 10)           // 粗值计数溢出 (10)
#define GP22_TDC_OVERFLOW_BIT        (1 << 9)            // TDC 单元溢出 (9)
#define GP22_HIT_CNT2_BIT (x)        (((x) >> 6) & 0x07) // 通道 2 脉冲数 (8-6)
#define GP22_HIT_CNT1_BIT (x)        (((x) >> 3) & 0x07) // 通道 1 脉冲数 (5-3)
#define GP22_RES_PTR_BIT (x)         ((x) & 0x07)        // 结果指针 (2-0)

#endif /* BSP_GP22_REG_H */ 
