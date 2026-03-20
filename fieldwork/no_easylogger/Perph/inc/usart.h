// /**
//  ****************************************************************************************************
//  * @file        usart.h
//  * @author      ����ԭ���Ŷ�(ALIENTEK)
//  * @version     V1.0
//  * @date        2020-04-20
//  * @brief       ���ڳ�ʼ������(һ���Ǵ���1)��֧��printf
//  * @license     Copyright (c) 2020-2032, �������������ӿƼ����޹�˾
//  ****************************************************************************************************
//  * @attention
//  *
//  * ʵ��ƽ̨:����ԭ�� STM32F103������
//  * ������Ƶ:www.yuanzige.com
//  * ������̳:www.openedv.com
//  * ��˾��ַ:www.alientek.com
//  * �����ַ:openedv.taobao.com
//  *
//  * �޸�˵��
//  * V1.0 20211103
//  * ��һ�η���
//  *
//  ****************************************************************************************************
//  */

// #ifndef __USART_H
// #define __USART_H

// #include "stdio.h"
// #include "sys.h"
// #include "stm32f1xx_hal.h"

// /******************************************************************************************/
// /* ���� �� ���� ����
//  * Ĭ�������USART1��.
//  * ע��: ͨ���޸��⼸���궨��,����֧��USART1~UART5����һ������.
//  */
// #define USART_TX_GPIO_PORT GPIOA
// #define USART_TX_GPIO_PIN  GPIO_PIN_9
// #define USART_TX_GPIO_CLK_ENABLE()    \
//     do {                              \
//         __HAL_RCC_GPIOA_CLK_ENABLE(); \
//     } while (0) /* PA��ʱ��ʹ�� */

// #define USART_RX_GPIO_PORT GPIOA
// #define USART_RX_GPIO_PIN  GPIO_PIN_10
// #define USART_RX_GPIO_CLK_ENABLE()    \
//     do {                              \
//         __HAL_RCC_GPIOA_CLK_ENABLE(); \
//     } while (0) /* PA��ʱ��ʹ�� */

// #define USART_UX            USART1
// #define USART_UX_IRQn       USART1_IRQn
// #define USART_UX_IRQHandler USART1_IRQHandler
// #define USART_UX_CLK_ENABLE()          \
//     do {                               \
//         __HAL_RCC_USART1_CLK_ENABLE(); \
//     } while (0) /* USART1 ʱ��ʹ�� */

// /******************************************************************************************/

// #define USART_REC_LEN 200 /* �����������ֽ��� 200 */
// #define USART_EN_RX   1   /* ʹ�ܣ�1��/��ֹ��0������1���� */
// #define RXBUFFERSIZE  1   /* �����С */

// extern UART_HandleTypeDef g_uart1_handle; /* HAL UART��� */

// extern uint8_t g_usart_rx_buf[USART_REC_LEN]; /* ���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� */
// extern uint16_t g_usart_rx_sta;               /* ����״̬��� */
// extern uint8_t g_rx_buffer[RXBUFFERSIZE];     /* HAL��USART����Buffer */

// void usart_init(uint32_t bound); /* ���ڳ�ʼ������ */

// #endif

#ifndef __USART_H__
#define __USART_H__

#include "sys.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

void uart1_dma_init(uint8_t *uart_rx_buffer, uint32_t databuf_size);

void usart1_send_bytes(uint8_t *buf, uint16_t len);

#endif
