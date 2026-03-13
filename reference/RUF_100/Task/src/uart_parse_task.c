#include "uart_parse_task.h"
#include "my_modbus.h"
#include "my_modbus_handler.h"
#include "stdio.h"
#include "string.h"
static TaskHandle_t task_uart_parse_Handle = NULL;/* 创建任务句柄 */
QueueHandle_t queue_irq_rec_A = NULL;
static circular_buffer_t * g_circular_buffer_from_driver = NULL;

void uart_rec_task(void *argument)
{
    /* USER CODE BEGIN uart_rec_A_func */
    /* Infinite loop */
	static uint32_t receive_data = 0;
	log_i("uart_rec_A_func Init Success");
	queue_irq_rec_A = NULL;
	queue_irq_rec_A = xQueueCreate( 1, 4 );
	if ( NULL == queue_irq_rec_A)
	{
		log_i("queue_irq_rec_A Init failed");
		
	} else {
		log_i("queue_irq_rec_A Init Success");
		log_i("queue_irq_r ec_A = [%x]",queue_irq_rec_A);
	}
    
    g_circular_buffer_from_driver = get_circular_buffer();
    log_i("g_circular_buffer_from_driver = [%x]",g_circular_buffer_from_driver);
    
    // Modbus 帧状态变量
    // Modbus 解析器实例
    modbus_parser_t parser;
    memset(&parser, 0, sizeof(parser));
    parser.state = MODBUS_STATE_IDLE;
    
    for(;;)
    {
        // 获取当前时间
        uint32_t current_time = xTaskGetTickCount();
//		log_d("uart_parse_func Running\r\n");
		xQueueReceive(queue_irq_rec_A,&receive_data,portMAX_DELAY);
        //log_i("receive_data = [%x]",receive_data);
      
        //第零步、依次去将环形缓冲区的数据进行解析
        if ( NULL == g_circular_buffer_from_driver )
        {
            log_e("error pointer");
            return;
        }
//        // 超时检测（3.5字符时间）
    //    if (parser.state != MODBUS_STATE_IDLE && 
    //        (current_time - parser.last_char_time) > pdMS_TO_TICKS(MODBUS_TIMEOUT_MS))
    //    {
    //        log_w("Modbus frame timeout, resetting parser");
    //        parser.state = MODBUS_STATE_IDLE;
    //    }
        while( 0x00 != buffer_is_empty(g_circular_buffer_from_driver) )
        {
            uint8_t byte = 0;
            if (get_data(g_circular_buffer_from_driver, &byte) != 0) {
//                log_e("Failed to read from circular buffer");
                continue;
            }
            //log_e("Receive data = 0x%02X\r\n",byte);
            // 更新最后接收时间
            parser.last_char_time = current_time;
            
            // 状态机处理当前字节
            switch (parser.state)
            {
                case MODBUS_STATE_IDLE:
                    // 第一个字节必须是地址
                    if(byte != MODBUS_ADDRESS)
                    {
                        // 地址不匹配，忽略字节
                        // log_w("Address mismatch: 0x%02X (expected 0x%02X)", 
                        // byte, MODBUS_ADDRESS);
                        break;
                    }
                    parser.address = byte;
                    parser.calculated_crc = 0xFFFF; // 初始化CRC计算
                    parser.calculated_crc = crc_update(parser.calculated_crc, byte);
                    parser.state = MODBUS_STATE_ADDRESS;
                    // log_d("IDLE -> ADDRESS: 0x%02X", byte);
                    break;
                    
                case MODBUS_STATE_ADDRESS:
                    // 第二个字节是功能码
                    parser.function = byte;
                    parser.calculated_crc = crc_update(parser.calculated_crc, byte);
                    
                    // 根据功能码确定期望的数据长度
                    switch (parser.function) 
                    {
                        case MODBUS_READ_COILS:
                        case MODBUS_READ_DISCRETE_INPUTS:
                        case MODBUS_READ_HOLDING_REGISTERS:
                        case MODBUS_READ_INPUT_REGISTERS:
                        // 这些功能码后跟4字节（起始地址2字节 + 数量2字节）
                        parser.expected_length = 4;
                        break;
                            
                        case MODBUS_WRITE_SINGLE_COIL:
                        case MODBUS_WRITE_SINGLE_REGISTER:
                        // 这些功能码后跟4字节（地址2字节 + 值2字节）
                        parser.expected_length = 4;
                        break;
                            
                        case MODBUS_WRITE_MULTIPLE_REGISTERS:
                        // 前4字节是地址和数量，然后是字节数和数据
                        parser.expected_length = 5; // 初始长度
                        break;
                            
                        default:
                        parser.expected_length = 0;
                        break;
                    }
                    parser.data_length = 0;
                    parser.state = MODBUS_STATE_FUNCTION;
                    // log_d("ADDRESS -> FUNCTION: 0x%02X", byte);
                    break;
                case MODBUS_STATE_FUNCTION:
                    // 处理数据字节
                    // log_d("FUNCTION:expect _length %d",parser.expected_length);
                    parser.data[parser.data_length++] = byte;
                    parser.calculated_crc = crc_update(parser.calculated_crc, byte);
                    // log_d("FUNCTION:  id %d ,0x%02X",parser.data_length, byte);
                    // // 检查是否需要特殊处理（如写多个寄存器）
                    if (parser.function == MODBUS_WRITE_MULTIPLE_REGISTERS && 
                        parser.data_length == 5) {
                        // 第5个字节是字节计数
                        uint8_t byte_count = parser.data[4];
                        parser.expected_length = 5 + byte_count; // 4字节头 + 字节计数 + 数据
                    }
                    
                    // 检查是否收到所有期望数据
                    if (parser.data_length >= parser.expected_length) {
                        parser.state = MODBUS_STATE_CRC_LOW;
//                        log_d("FUNCTION -> CRC_LOW (data len=%d)", parser.data_length);
                    }
                    break;
                    
                case MODBUS_STATE_CRC_LOW:
                    // 接收CRC低字节
                    parser.crc = byte; // 低位在前
                    parser.state = MODBUS_STATE_CRC_HIGH;
                    // log_d("CRC_LOW: 0x%02X", byte);
                    break;
                    
                case MODBUS_STATE_CRC_HIGH:
                    // 接收CRC高字节并完成帧
                    parser.crc |= (byte << 8);
                    
                    // 验证CRC
                    if (parser.calculated_crc == parser.crc) {
                        // log_i("Valid frame received: Addr=0x%02X, Func=0x%02X", 
                        // parser.address, parser.function);
                        // 处理完整帧
                        process_modbus_frame(&parser);
                        // 重置状态机
                        parser.state = MODBUS_STATE_IDLE;
                        // log_d("Complete -> IDLE (resetting)");
                    } else {
//                        log_e("CRC error: Calc=0x%04X, Recv=0x%04X", 
//                        parser.calculated_crc, parser.crc);
//                        log_e("Frame error, resetting parser");
                        parser.state = MODBUS_STATE_IDLE;
//                        for(int i = 0;i <parser.data_length;i++)
//                        log_d(" parser.data[%d] = %d\r\n", i ,parser.data[i]);
                    }
                    break;

                default:
                    parser.state = MODBUS_STATE_IDLE;
                    break;
            }
         }
        vTaskDelay(pdMS_TO_TICKS(20)); // 适当延时
    }
}
void do_create_uart_parse_task(void)
{	
	BaseType_t xReturn = pdPASS;						/* 定义一个创建信息返回值，默认为pdPASS */
	/* 创建AppTaskCreate任务 */
	xReturn = xTaskCreate(uart_rec_task, "task_modbus", 512, NULL, 30, &task_uart_parse_Handle);      
}



