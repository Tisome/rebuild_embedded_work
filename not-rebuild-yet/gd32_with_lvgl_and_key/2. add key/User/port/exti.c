#include "app_config.h"
#include "key.h"
#include "exti.h"
#include "FPGA_INT.h"

/*********************include预处理部分*********************/
#include "gd32f30x.h"

#include "exti.h"
#include "freertos_resources.h"

#include "FreeRTOS.h"
#include "semphr.h"

#if USE_KEY_SW1
#include "led.h"
#include <stdio.h>
#endif

/*********************include预处理部分*********************/

/*********************按键部分*********************/
#if USE_KEY_SW1

void KEY_SW1_exti_Init(void)
{
    rcu_periph_clock_enable(SW1_KEY_GPIO_CLK);
    rcu_periph_clock_enable(RCU_AF);

    gpio_init(SW1_KEY_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ,
              SW1_KEY_PIN);

    gpio_exti_source_select(SW1_KEY_EXTI_PORT_SOURCE, SW1_KEY_EXTI_PIN_SOURCE);

    exti_init(SW1_KEY_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(SW1_KEY_EXTI_LINE);
}

// SW1按键中断的handler
void EXTI5_9_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (exti_interrupt_flag_get(SW1_KEY_EXTI_LINE))
    {
        exti_interrupt_flag_clear(SW1_KEY_EXTI_LINE);

        /* 通知key任务当前数据已经接收完成 */
        xSemaphoreGiveFromISR(xSem_ISR, &xHigherPriorityTaskWoken);
    }
    /* 强制任务切换 */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void vkey_task(void *pvParameter)
{
    (void)pvParameter;
    while (1)
    {
        xSemaphoreTake(xSem_ISR, portMAX_DELAY);
        LED_TOGGLE();
        printf("KEY SW1 test\r\n");
    }
}

#endif

#if USE_4KEY

#define KEY_DEBOUNCE_MS 100
#define KEY_DEBOUNCE_TICKS pdMS_TO_TICKS(KEY_DEBOUNCE_MS)

static volatile TickType_t k1_last_tick = 0;
static volatile TickType_t k2_last_tick = 0;
static volatile TickType_t k3_last_tick = 0;
static volatile TickType_t k4_last_tick = 0;

void KEY_All_exti_Init(void)
{
    /* 使能 GPIOA 和 AF 时钟 */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_AF);

    /* PA1 PA2 PA3 PA4 配置为上拉输入 */
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ,
              K1_KEY_PIN | K2_KEY_PIN | K3_KEY_PIN | K4_KEY_PIN);

    /* EXTI 线映射到 GPIOA */
    gpio_exti_source_select(K1_KEY_EXTI_PORT_SOURCE, K1_KEY_EXTI_PIN_SOURCE);
    gpio_exti_source_select(K2_KEY_EXTI_PORT_SOURCE, K2_KEY_EXTI_PIN_SOURCE);
    gpio_exti_source_select(K3_KEY_EXTI_PORT_SOURCE, K3_KEY_EXTI_PIN_SOURCE);
    gpio_exti_source_select(K4_KEY_EXTI_PORT_SOURCE, K4_KEY_EXTI_PIN_SOURCE);

    /* 配置 EXTI：下降沿触发中断 */
    exti_init(K1_KEY_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_init(K2_KEY_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_init(K3_KEY_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_init(K4_KEY_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);

    /* 清中断标志 */
    exti_interrupt_flag_clear(K1_KEY_EXTI_LINE);
    exti_interrupt_flag_clear(K2_KEY_EXTI_LINE);
    exti_interrupt_flag_clear(K3_KEY_EXTI_LINE);
    exti_interrupt_flag_clear(K4_KEY_EXTI_LINE);

    k1_last_tick = 0;
    k2_last_tick = 0;
    k3_last_tick = 0;
    k4_last_tick = 0;
}

static void key_isr_handle(uint32_t exti_line, volatile TickType_t *last_tick, key_event_t evt)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (exti_interrupt_flag_get(exti_line) != RESET)
    {
        exti_interrupt_flag_clear(exti_line);

        TickType_t now = xTaskGetTickCountFromISR();

        if ((now - *last_tick) >= KEY_DEBOUNCE_TICKS)
        {
            *last_tick = now;
            (void)xQueueSendFromISR(xQueue_KeyEvent, &evt, &xHigherPriorityTaskWoken);
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void EXTI1_IRQHandler(void)
{
    key_isr_handle(EXTI_1, &k1_last_tick, KEY_EVENT_K1);
}

void EXTI2_IRQHandler(void)
{
    key_isr_handle(EXTI_2, &k2_last_tick, KEY_EVENT_K2);
}

void EXTI3_IRQHandler(void)
{
    key_isr_handle(EXTI_3, &k3_last_tick, KEY_EVENT_K3);
}

void EXTI4_IRQHandler(void)
{
    key_isr_handle(EXTI_4, &k4_last_tick, KEY_EVENT_K4);
}

#endif

/*********************按键部分*********************/

/*********************FPGA_INT部分*********************/
void FPGA_INT_exti_Init(void)
{
    rcu_periph_clock_enable(FPGA_INT_GPIO_CLK);
    rcu_periph_clock_enable(RCU_AF);

    gpio_init(FPGA_INT_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ,
              FPGA_INT_GPIO_PIN);
    gpio_exti_source_select(FPGA_INT_EXTI_PORT_SOURCE, FPGA_INT_EXTI_PIN_SOURCE);

    exti_init(FPGA_INT_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(FPGA_INT_EXTI_LINE);
}

// 在这里是给freeRTOS的信号量，等移植好再说
void FPGA_INT_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (exti_interrupt_flag_get(FPGA_INT_EXTI_LINE))
    {
        exti_interrupt_flag_clear(FPGA_INT_EXTI_LINE);

        xSemaphoreGiveFromISR(xSem_FPGA_INT, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
/*********************FPGA_INT部分*********************/

/*********************输出代码部分*********************/
void EXTI_Init(void)
{
#if USE_KEY_SW1
    KEY_SW1_exti_Init();
#endif
#if USE_4KEY
    KEY_All_exti_Init();
#endif
    FPGA_INT_exti_Init();
}
/*********************输出代码部分*********************/
