#include "main.h"
#include "rcu.h"
#include "gpio.h"
#include "dma.h"
#include "spi.h"
#include "dac.h"
#include "emit.h"
#include "task_manager.h"
#include "tim.h"
//lvgl 相关函数
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_demo_widgets.h"
#include"link_total.h"
int main(void)
{
    for(int i=0;i<1000;i++) __NOP(); // 硬件上电稳定延时

    app_elog_init();

    systick_config();

    rcu_config();

    gpio_config();

    spi_config();

    dac_config();

    tim6_delay_init();

    tim5_delay_init();

    lv_init();

    do_create_start_task();

}


