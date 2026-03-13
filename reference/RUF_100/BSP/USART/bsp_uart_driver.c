/******************************************************************************
 * Copyright (C) 2025 Rvision, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file bsp_uart_driver.h
 * 
 * @par dependencies 
 * - bsp_uart_driver.h
 * - stdio.h
 * @author rnz 
 * 
 * @brief Provide the HAL APIs of usart and corresponding opetions.
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.0 2025-7-10
 *
 * @note 1 tab == 4 spaces!
 * 
 * *****************************************************************************/
#include "bsp_uart_driver.h"
#include "stdio.h"


#define BUFFER_A 0
#define BUFFER_B 1

#define IRQ_SEND_TO_THREAD 0xA1A2A3A4
#define FRONT_SEND_TO_END  0xB1B2B3B4


#if 1 // circular buffer

uint8_t g_data_buffer = 0;

#endif // end of circular buffer
circular_buffer_t * g_circular_buffer_irq_thread = NULL;//.bss
static QueueHandle_t       queue_uart_irq_thread        = NULL;

extern QueueHandle_t queue_irq_rec_A;
/*
 *前端：
 *1、buffer是否已满-》停下串口
 *2、将当前数据就绪的事件发送给后端
*/
void uart_driver_func(void *argument)
{
    uint32_t receive_data = 0;
    
    //0. 创建环形缓冲区 alloc the ring buffer
    g_circular_buffer_irq_thread = create_empty_circular_buffer();
    if ( NULL == g_circular_buffer_irq_thread )
    {
        log_e("circular_buffer create failed");
    }
    

    
    // 1.创建队列，用于系统硬件（串口）向我传递消息
    queue_uart_irq_thread = xQueueCreate( 1, 4 );
    if ( NULL == queue_uart_irq_thread )
    {
        log_e("circular_buffer create failed");
    }
	
    
    // 2.启动串口接收
	
    //2.1 串口空闲中断接收+DMA半满全满
    usart0_dma_init(g_circular_buffer_irq_thread->data,CIRCULAR_BUFFER_SIZE);
	/* DEBUG USART */
    

    for(;;)
    {
//        log_d("uart_driver_func Running\r\n");
        //3、判断是否有中断发送来队列通知环形缓冲区已有数据
        xQueueReceive(queue_uart_irq_thread,&receive_data,portMAX_DELAY);
//        log_i("front recive data from irq = [%x]",receive_data);
        if( IRQ_SEND_TO_THREAD == receive_data )
        {
            //4、将当前数据就绪的事件发送给后端
            uint32_t send_to_end = FRONT_SEND_TO_END;
            BaseType_t ret_queue = pdTRUE;
            ret_queue = xQueueGenericSend(  queue_irq_rec_A, \
                                               &send_to_end, \
                                                          0, \
                                              queueOVERWRITE);
            if( pdTRUE != ret_queue)
            {
                log_e("send error");
                return;
            }
            
          //  log_d("front to end send success"); 

        }
    vTaskDelay(pdMS_TO_TICKS(5)); // 适当延时
    }
  /* USER CODE END uart_rec_A_func */
}


/**
 * @brief pass the pointer to eternal caller.
 * 
 *  
 * @param[in] void 
 * 
 * @return    circular_buffer_t : 
                        pointer to the circcular buffer
 * 
 * */
circular_buffer_t * get_circular_buffer(void)
{
        
    if ( NULL == g_circular_buffer_irq_thread )
    {
        return NULL;
    }
    
    return g_circular_buffer_irq_thread;
}



void dma_half_irq_callback(uint32_t number_of_data)
{
    //log_d("dma_half_irq = [%d]",number_of_data ); 
    /*比如,当前缓冲区长度为10（0~9)，
    head当前位置为32，产生半满说明数据已经到达4，
    于是32%5 = 2，
    所以将head添加到34即可（因为0~4中，它在2的位置，所以+2就到34）。*/
    uint32_t head_pos = 0;
    uint8_t ret = 0;
    //1.获取当前位置
    ret=get_head_pos(g_circular_buffer_irq_thread,&head_pos);
    if( 0x00 != ret)
    {
//        log_d("get head pos error" ); 
    }
//    log_d("head_pos = [%d]",head_pos ); 
    //2.获取进入半满中断时，数据已经到达的位置：(CIRCULAR_BUFFER_SIZE/2)-1
    uint32_t current_data_pos = (CIRCULAR_BUFFER_SIZE/2);
//    log_d("current_data_pos = [%d]",current_data_pos  ); 
    
    //3.对heap进行取余数
    uint32_t pos_in_buffer = head_pos%(CIRCULAR_BUFFER_SIZE/2);
//    log_d("pos_in_buffer = [%d]",pos_in_buffer); 
    
    //4.算出当前应该偏移的数量
    uint32_t move_pos = 0x00;
    if(current_data_pos < pos_in_buffer)
    {
        move_pos = (current_data_pos+CIRCULAR_BUFFER_SIZE) - pos_in_buffer;
//        log_d("move_pos_dam_complete = [%d]",move_pos);
    } 
    else
    {
        move_pos = current_data_pos - pos_in_buffer;
//        log_d("move_pos_normal = [%d]",move_pos);
    }
    
    //5.对head位置进行累加
    head_pos_increment(g_circular_buffer_irq_thread,move_pos);
    
    //6.对前端线程进行通知
    //TBD
    uint32_t send_to_thread = IRQ_SEND_TO_THREAD;
    BaseType_t ret_queue = pdTRUE;
    ret_queue = xQueueGenericSendFromISR( queue_uart_irq_thread, \
                                                 &send_to_thread,\
                                                            NULL,\
                                                   queueOVERWRITE);
//    log_d("dma_half_irq_callback ret_queue = [%d]",ret_queue); 
    //test:
//    ret=get_head_pos(g_circular_buffer_irq_thread,&head_pos);
//    if( 0x00 != ret)
//    {
//        log_d("get head pos error" ); 
//    }
//    log_d("head_pos = [%d]",head_pos ); 
//    
//    pos_in_buffer = head_pos%(CIRCULAR_BUFFER_SIZE/2);
//    log_d("pos_in_buffer = [%d]",pos_in_buffer); 
//    
}

void dma_comp_irq_callback(uint32_t number_of_data)
{
    //log_d("dma_comp_irq = [%d]",number_of_data ); 
    /*比如,当前缓冲区长度为10（0~9)，
    head当前位置为37，产生全满说明数据已经到达9，
    于是37%10 = 7，
    将head添加到39即可（因为0~9中，它在7的位置，所以+2就到39）。
    */
    uint32_t head_pos = 0;
    uint8_t ret = 0;
    
    //1.获取当前位置
    ret=get_head_pos(g_circular_buffer_irq_thread,&head_pos);
    if( 0x00 != ret)
    {
//        log_d("get head pos error" ); 
    }
//    log_d("[c]head_pos = [%d]",head_pos ); 
    //2.获取进入全满中断时，数据已经到达的位置：(CIRCULAR_BUFFER_SIZE)-1
    uint32_t current_data_pos = (CIRCULAR_BUFFER_SIZE);
//    log_d("[c]current_data_pos = [%d]",current_data_pos  ); 
    
    //3.对heap进行取余数
    uint32_t pos_in_buffer = head_pos%(CIRCULAR_BUFFER_SIZE);
//    log_d("[c]pos_in_buffer = [%d]",pos_in_buffer); 
    
    //4.算出当前应该偏移的数量
    uint32_t move_pos = 0x00;
    if(current_data_pos < pos_in_buffer)
    {
        move_pos = (current_data_pos+CIRCULAR_BUFFER_SIZE) - pos_in_buffer;
//        log_d("[c]move_pos_dam_complete = [%d]",move_pos);
    } 
    else
    {
        move_pos = current_data_pos - pos_in_buffer;
//        log_d("[c]move_pos_normal = [%d]",move_pos);
    }
    
    //5.对head位置进行累加
    head_pos_increment(g_circular_buffer_irq_thread,move_pos);
    
    //6.对前端线程通知
    //TBD
    uint32_t send_to_thread = IRQ_SEND_TO_THREAD;
    BaseType_t ret_queue = pdTRUE;
    ret_queue = xQueueGenericSendFromISR( queue_uart_irq_thread, \
                                                 &send_to_thread,\
                                                            NULL,\
                                                   queueOVERWRITE);
//    log_d("dma_comp_irq_callback ret_queue = [%d]",ret_queue); 
    //test:
    ret=get_head_pos(g_circular_buffer_irq_thread,&head_pos);
    if( 0x00 != ret)
    {
//        log_d("get head pos error" ); 
    }
//    log_d("[c]head_pos = [%d]",head_pos ); 
    
    pos_in_buffer = head_pos%(CIRCULAR_BUFFER_SIZE);
//    log_d("[c]pos_in_buffer = [%d]",pos_in_buffer); 
    
    
    
}
void uart_idle_irq_callback(uint32_t number_of_data)
{
    //Fix_DMA_Config();
//    log_d("uart_idle_irq = [%d]",number_of_data ); 
    /*
    比如,当前缓冲区长度为10（0~9)，
    head当前位置为35，
    产生空闲中断得到的Size数据为9,
    于是需要增加(9-1)-(35%10)= 3，
    将head添加到38即可。
    */
    uint32_t head_pos = 0;
    uint8_t ret = 0;
    
    //1.获取当前位置
    ret=get_head_pos(g_circular_buffer_irq_thread,&head_pos);
    if( 0x00 != ret)
    {
//        log_d("get head pos error" ); 
    }
//    log_d("[U]head_pos = [%d]",head_pos ); 
    //2.获取进入空闲中断时，数据已经到达的位置：number_of_data-1
    uint32_t current_data_pos = number_of_data;
//    log_d("[U]current_data_pos = [%d]",current_data_pos  ); 
    
    //3.对heap进行取余数
    uint32_t pos_in_buffer = head_pos%(CIRCULAR_BUFFER_SIZE);
//    log_d("[U]pos_in_buffer = [%d]",pos_in_buffer); 
    
    //4.算出当前应该偏移的数量
    uint32_t move_pos = 0x00;
    if(current_data_pos < pos_in_buffer)
    {
        move_pos = (current_data_pos+CIRCULAR_BUFFER_SIZE) - pos_in_buffer;
//        log_d("[U]move_pos_dam_complete = [%d]",move_pos);
    } 
    else
    {
        move_pos = current_data_pos - pos_in_buffer;
//        log_d("[U]move_pos_normal = [%d]",move_pos);
    }
    //5.对head位置进行累加
    head_pos_increment(g_circular_buffer_irq_thread,move_pos);
    //6.进行通知
    
    uint32_t send_to_thread = IRQ_SEND_TO_THREAD;
    BaseType_t ret_queue = pdTRUE;
    ret_queue = xQueueGenericSendFromISR( queue_uart_irq_thread, \
                                                 &send_to_thread,\
                                                            NULL,\
                                                   queueOVERWRITE);
//    log_d("uart_idle_irq_callback ret_queue = [%d]",ret_queue); 
    //test:
    ret=get_head_pos(g_circular_buffer_irq_thread,&head_pos);
    if( 0x00 != ret)
    {
//        log_d("get head pos error" ); 
    }
//    log_d("[U]head_pos = [%d]",head_pos ); 
    
    pos_in_buffer = head_pos%(CIRCULAR_BUFFER_SIZE);
//    log_d("[U]pos_in_buffer = [%d]",pos_in_buffer); 
    
    
}
