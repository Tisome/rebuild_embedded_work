#ifndef MAIN_H
#define MAIN_H

#include "gd32f30x.h"
#include "systick.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "SEGGER_RTT.h"
#include "elog.h"

//TFT
#define TFT_DC_Pin GPIO_PIN_4
#define TFT_DC_GPIO_Port GPIOB

#define TFT_CS_Pin GPIO_PIN_15
#define TFT_CS_GPIO_Port GPIOA

#define TFT_RESET_Pin GPIO_PIN_13
#define TFT_RESET_GPIO_Port GPIOC

#define TFT_SCL_Pin GPIO_PIN_3
#define TFT_SCL_GPIO_Port GPIOB

#define TFT_SDA_Pin GPIO_PIN_5
#define TFT_SDA_GPIO_Port GPIOB

#define TFT_BL_Pin GPIO_PIN_9
#define TFT_BL_GPIO_Port GPIOB

#define TFT_DMA DMA1
#define TFT_DMA_CH DMA_CH1

//KEY
#define KEY0_GPIO_PORT                  GPIOB
#define KEY0_GPIO_PIN                   GPIO_PIN_0

#define KEY1_GPIO_PORT                  GPIOA
#define KEY1_GPIO_PIN                   GPIO_PIN_6

#define KEY2_GPIO_PORT                  GPIOB
#define KEY2_GPIO_PIN                   GPIO_PIN_1

#define KEY3_GPIO_PORT                  GPIOB
#define KEY3_GPIO_PIN                   GPIO_PIN_2


//E2PROM
#define EEPROM_I2C                      I2C0
#define EEPROM_I2C_GPIO_PORT            GPIOB
#define EEPROM_I2C_SCL_PIN              GPIO_PIN_6
#define EEPROM_I2C_SDA_PIN              GPIO_PIN_7

//GP22
#define GP22_CS_Pin GPIO_PIN_12
#define GP22_CS_GPIO_Port GPIOB


#define GP22_SCL_Pin GPIO_PIN_13
#define GP22_SCL_GPIO_Port GPIOB

#define GP22_MISO_Pin GPIO_PIN_14
#define GP22_MISO_GPIO_Port GPIOB

#define GP22_MOSI_Pin GPIO_PIN_15
#define GP22_MOSI_GPIO_Port GPIOB

#define GP22_START_Pin GPIO_PIN_12
#define GP22_START_GPIO_Port GPIOA


//DAC
#define DAC_FIRE_Pin GPIO_PIN_5
#define DAC_FIRE_GPIO_Port GPIOA

#define DAC_COM_Pin GPIO_PIN_4
#define DAC_COM_GPIO_Port GPIOA

//EMIT FIRE SWITCH 
#define FIRE_SEL_Pin GPIO_PIN_1
#define FIRE_SEL_GPIO_Port GPIOA

#define SIG_SEL_Pin GPIO_PIN_2
#define SIG_SEL_GPIO_Port GPIOA

//FLOW OUT SEL
#define FLOW_OUT_SEL_Pin GPIO_PIN_0
#define FLOW_OUT_SEL_GPIO_Port GPIOA

//Rs485 control
#define RS485_CONTROL_Pin GPIO_PIN_14
#define RS485_CONTROL_GPIO_Port GPIOC
#endif /* MAIN_H */
