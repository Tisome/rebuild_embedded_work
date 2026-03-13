#include "gd32f30x.h"
#include "FreeRTOS.h"
#include "app_config.h"
#include "task.h"
#include "key.h"
#include "FPGA_INT.h"

/* 统一定义：RTOS可用的IRQ优先级（>= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY） */
#define IRQ_PRIO_RTOS_SAFE   10U
#define IRQ_SUBPRIO          0U

// 不知道为什么，这里必须用CMSIS写法设置中断优先级才行，之后再说
void IRQ_Config(void)
{
    NVIC_SetPriorityGrouping(0);  // FreeRTOS 推荐

    NVIC_SetPriority(DMA0_Channel1_IRQn, 8);
    NVIC_EnableIRQ(DMA0_Channel1_IRQn);

    // FPGA_INT
    NVIC_SetPriority(FPGA_INT_GPIO_IRQn, 6);
    NVIC_EnableIRQ(FPGA_INT_GPIO_IRQn);

#if USE_KEY_SW1
    // SW1 KEY
    NVIC_SetPriority(SW1_KEY_EXTI_IRQn, 10);
    NVIC_EnableIRQ(SW1_KEY_EXTI_IRQn);
#endif

#if USE_4KEY
    NVIC_SetPriority(K1_KEY_EXTI_IRQn, 10);
    NVIC_EnableIRQ(K1_KEY_EXTI_IRQn);

    NVIC_SetPriority(K2_KEY_EXTI_IRQn, 10);
    NVIC_EnableIRQ(K2_KEY_EXTI_IRQn);

    NVIC_SetPriority(K3_KEY_EXTI_IRQn, 10);
    NVIC_EnableIRQ(K3_KEY_EXTI_IRQn);

    NVIC_SetPriority(K4_KEY_EXTI_IRQn, 10);
    NVIC_EnableIRQ(K4_KEY_EXTI_IRQn);
#endif
}


