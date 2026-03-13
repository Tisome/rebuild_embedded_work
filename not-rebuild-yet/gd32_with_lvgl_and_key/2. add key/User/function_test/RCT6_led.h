#ifndef __TEST_RCT6_LED_H
#define __TEST_RCT6_LED_H

#define LED_GPIO_PORT GPIOA
#define LED_GPIO_PIN GPIO_PIN_0
#define LED_CLK_GPIO_PORT RCU_GPIOA

#define LED_LIGHT RESET
#define LED_NOT_LIGHT SET

#define LED_ENABLE()                                 \
    do                                               \
    {                                                \
        gpio_bit_reset(LED_GPIO_PORT, LED_GPIO_PIN); \
    } while (0)

#define LED_DISABLE()                              \
    do                                             \
    {                                              \
        gpio_bit_set(LED_GPIO_PORT, LED_GPIO_PIN); \
    } while (0)

#define LED_TOGGLE()                                                       \
    do                                                                     \
    {                                                                      \
        if (gpio_output_bit_get(LED_GPIO_PORT, LED_GPIO_PIN) == LED_LIGHT) \
        {                                                                  \
            LED_DISABLE();                                                 \
        }                                                                  \
        else                                                               \
        {                                                                  \
            LED_ENABLE();                                                  \
        }                                                                  \
    } while (0)

void LED_Init(void);

#endif
