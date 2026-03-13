#include "my_modbus_handler.h"
#include "usart.h"
#include "data.h"
#include "main.h"

#define M3_TO_L  (1000.0)
#define M3_TO_ML (1000000.0)
#define M3H_TO_LMIN (1000.0 / 60.0)
#define M3H_TO_MLMIN (1000000.0 / 60.0)
#define M3H_TO_LH (1000.0)

// 全局Modbus数据存储
uint8_t discrete_inputs[DISCRETE_INPUT_SIZE];  // 离散输入(只读)
uint8_t coils[COIL_SIZE/8];                      // 线圈(读写)
uint16_t input_registers[INPUT_REG_SIZE];         // 输入寄存器(只读)
uint16_t holding_registers[HOLDING_REG_SIZE];     // 保持寄存器(读写)
// 物理量存储
extern double g_flow_info;          // 上电启动至今的累计流量
//流速和流量
extern float flow_rate_avg;
    // 更安全的浮点数转换方法
typedef union 
{
    float f;
    uint32_t u;
} float_converter_t;

float g_correction_points[8] = {1,2,3,4,5,6,7,8};
float g_correction_factor[8] = {8,7,6,5,4,3,2,1};
uint16_t g_equipment_number[2] = {0xABCD,0x1234};
// 初始化Modbuequipment numbers数据
void init_modbus_data() {
    // 初始化离散输入
    memset(discrete_inputs, 0, sizeof(discrete_inputs));
    // 只读线圈 (地址0x0000-0x0007)
    discrete_inputs[0] = 0x01;  // 示例: 第0位=1(系统正常)
    
    // 初始化线圈
    memset(coils, 0, sizeof(coils));
    
    // 初始化输入寄存器
    memset(input_registers, 0, sizeof(input_registers));
    
    // 初始化保持寄存器
    memset(holding_registers, 0, sizeof(holding_registers));
    
    float_converter_t converter;
    //瞬时流量
    float temp;
    converter.f = flow_rate_avg;
    holding_registers[9] = (converter.u >> 16) & 0xFFFF;  // 高16位
    holding_registers[8] = converter.u & 0xFFFF;          // 低16位
    
    switch (g_parameters.total_unit)
    {
    case 0:
        temp = g_flow_info; 
        break;
    case 1:
        temp = g_flow_info * M3_TO_L; 
        break;
    case 2:
        temp = g_flow_info * M3_TO_ML;
        break;
    default:
        break;
    }
    // 流量
    converter.f = temp;
    holding_registers[11] = (converter.u >> 16) & 0xFFFF;
    holding_registers[10] = converter.u & 0xFFFF;
    
    // 固定误差
    converter.f = g_parameters.fixed_error_bias;
    holding_registers[13] = (converter.u >> 16) & 0xFFFF;
    holding_registers[12] = converter.u & 0xFFFF;
    
    //流速单位
    holding_registers[20] = g_parameters.flow_rate_unit;  // 高16位
    
    // 流量单位
    holding_registers[21] = g_parameters.total_unit;  // 高16位
    
    // 表格 修正点
    for(int i = 0;i < 8; i++)
    {
        converter.f =  0;  
        holding_registers[50+i*2]   = (converter.u >> 16) & 0xFFFF;
        holding_registers[50+i*2+1] = converter.u & 0xFFFF;
        
    }
    
    // 表格 修正系数
    for(int i = 0;i < 8; i++)
    {
        converter.f =  0;  
        holding_registers[66+i*2]   = (converter.u >> 16) & 0xFFFF;
        holding_registers[66+i*2+1] = converter.u & 0xFFFF;
        
    }
    
    // 序列号
    for(int i = 0;i < 2; i++)
    {
        holding_registers[82+i]   = 0;   
    }
     //保留空间
    for(int i = 0;i < 16; i++)
    {
        holding_registers[22+i]   = 0;   
    }
}

// 更新物理量到输入寄存器
void update_input_registers() 
{
//    float_converter_t converter;
//    // 将浮点数转换为32位整数(大端)
//    float f =100.10;
//    // 流速
//    converter.f = g_flow_rate;
//    input_registers[0] = (converter.u >> 16) & 0xFFFF;  // 高16位
//    input_registers[1] = converter.u & 0xFFFF;          // 低16位
//    
//    // 流量
//    converter.f = g_flow;
//    input_registers[2] = (converter.u >> 16) & 0xFFFF;
//    input_registers[3] = converter.u & 0xFFFF;
//    
//    // 历史总流量
//    converter.f = g_flow_info;
//    input_registers[4] = (converter.u >> 16) & 0xFFFF;
//    input_registers[5] = converter.u & 0xFFFF;
//    
//    // 年总流量
//    converter.f = g_flow_every_year;
//    input_registers[6] = (converter.u >> 16) & 0xFFFF;
//    input_registers[7] = converter.u & 0xFFFF;
//    
//    // 月流量
//    converter.f = g_flow_every_month;
//    input_registers[8] = (converter.u >> 16) & 0xFFFF;
//    input_registers[9] = converter.u & 0xFFFF;
//    
//     // 日流量
//    converter.f = g_flow_every_day;
//    input_registers[10] = (converter.u >> 16) & 0xFFFF;
//    input_registers[11] = converter.u & 0xFFFF;
}
// 更新参数到保持寄存器
void update_holding_registers_read() 
{
    float_converter_t converter;
    //瞬时流量
    //瞬时流量
    float temp;

    converter.f = flow_rate_avg;
    holding_registers[9] = (converter.u >> 16) & 0xFFFF;  // 高16位
    holding_registers[8] = converter.u & 0xFFFF;          // 低16位
    
    switch (g_parameters.total_unit)
    {
    case 0:
        temp = g_flow_info; 
        break;
    case 1:
        temp = g_flow_info * M3_TO_L; 
        break;
    case 2:
        temp = g_flow_info * M3_TO_ML;
        break;
    default:
        break;
    }
    // 流量
    converter.f = temp;
    holding_registers[11] = (converter.u >> 16) & 0xFFFF;
    holding_registers[10] = converter.u & 0xFFFF;
    
    // 误差
    converter.f = g_parameters.fixed_error_bias;
    holding_registers[13] = (converter.u >> 16) & 0xFFFF;
    holding_registers[12] = converter.u & 0xFFFF;
    
    // 将浮点数转换为32位整数(大端)
//    float f =100.10;
    //流速单位
    holding_registers[20] = g_parameters.flow_rate_unit;  // 高16位
    
    // 流量单位
    holding_registers[21] = g_parameters.total_unit;  // 高16位
    
    // 表格 修正点
    for(int i = 0;i < 8; i++)
    {
        converter.f =  g_correction_points[i];  
        holding_registers[50+i*2]   = (converter.u >> 16) & 0xFFFF;
        holding_registers[50+i*2+1] = converter.u & 0xFFFF;
        
    }
    
    // 表格 修正系数
    for(int i = 0;i < 8; i++)
    {
        converter.f =  g_correction_factor[i];  
        holding_registers[66+i*2]   = (converter.u >> 16) & 0xFFFF;
        holding_registers[66+i*2+1] = converter.u & 0xFFFF;
        
    }
    
    // 序列号
    for(int i = 0;i < 2; i++)
    {
        holding_registers[82+i]   = g_equipment_number[i];   
    }
     //保留空间
    for(int i = 0;i < 16; i++)
    {
        holding_registers[22+i]   = 0;   
    }
}
// 更新参数到保持寄存器
void update_holding_registers_write() 
{
    float_converter_t converter;
    //流速单位
    g_parameters.flow_rate_unit = holding_registers[20];  // 高16位
    // 流量单位
    g_parameters.total_unit = holding_registers[21];  // 高16位
    //固定误差偏置
    converter.u = (holding_registers[13] << 16) | holding_registers[12];
    g_parameters.fixed_error_bias    = converter.f; // 转换为浮点数

    // 表格 修正点
    for(int i = 0;i < 8; i++)
    {
        converter.u = (holding_registers[50 + i*2] << 16) | holding_registers[50 + i*2 + 1];
        g_correction_points[i] = converter.f; // 转换为浮点数
    }
    
    // 表格修正系数
    for(int i = 0;i < 8; i++)
    {
        converter.u = (holding_registers[66 + i*2] << 16) | holding_registers[66 + i*2 + 1];
        g_correction_factor[i] = converter.f; // 转换为浮点数
    }
    
    // 序列号
    for(int i = 0;i < 2; i++)
    {
        g_equipment_number[i] =  holding_registers[82+i];
    }
     //保留空间
    for(int i = 0;i < 16; i++)
    {
        holding_registers[22+i]   = 0;   
    }
}
// 处理读线圈请求
void handle_read_coils(modbus_parser_t *parser) {
    uint16_t start_addr = (parser->data[0] << 8) | parser->data[1];
    uint16_t coil_count = (parser->data[2] << 8) | parser->data[3];
    
    // 检查地址范围
    if (start_addr < COIL_START_ADDR || start_addr + coil_count > COIL_START_ADDR + COIL_SIZE) {
        send_exception_response(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    // 计算实际索引
    uint16_t start_index = start_addr - COIL_START_ADDR;
    uint8_t byte_count = (coil_count + 7) / 8;
    uint8_t response[256];
    
    // 构建响应
    response[0] = byte_count;
    memset(&response[1], 0, byte_count);
    
    // 复制线圈状态
    for (int i = 0; i < coil_count; i++) {
        if (coils[(start_index + i) / 8] & (1 << ((start_index + i) % 8))) {
            response[1 + i/8] |= (1 << (i % 8));
        }
    }
    
    // 发送响应
    send_modbus_response(parser->address, parser->function, 
                         response, byte_count + 1);
}

// 处理写单个线圈请求
void handle_write_single_coil(modbus_parser_t *parser) 
{
    uint16_t coil_addr = (parser->data[0] << 8) | parser->data[1];
    uint16_t coil_value = (parser->data[2] << 8) | parser->data[3];
    
    // 检查地址范围
    if (coil_addr < 0x2000 || coil_addr >= COIL_START_ADDR + COIL_SIZE) {
        send_exception_response(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    // 检查值是否合法
    if (coil_value != 0x0000 && coil_value != 0xFF00) 
    {
        send_exception_response(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }
    
    // 计算索引
    uint16_t index = coil_addr - 0x2000;
    uint8_t byte_index = index / 8;
    uint8_t bit_mask = 1 << (index % 8);
    
    // 更新线圈状态
    if (coil_value == 0xFF00) {
        coils[byte_index] |= bit_mask;
    } else {
        coils[byte_index] &= ~bit_mask;
    }
    
    // 发送响应(回显相同数据)
    send_modbus_response(parser->address, parser->function, 
                         parser->data, 4);
}

// 处理读保持寄存器请求
void handle_read_holding_registers(modbus_parser_t *parser) {
    uint16_t start_addr = (parser->data[0] << 8) | parser->data[1];
    uint16_t reg_count = (parser->data[2] << 8) | parser->data[3];
    
    // 检查地址范围
    if (start_addr < HOLDING_REG_START || 
        (start_addr + reg_count) > (HOLDING_REG_START + HOLDING_REG_SIZE)) {
        send_exception_response(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
     // 转换为相对地址
    uint16_t relative_start = start_addr - 0;
    uint8_t response[128];
    response[0] = reg_count * 2;
    
    
    update_holding_registers_read();
    // 复制寄存器数据
    for (int i = 0; i < reg_count; i++) {
        response[1 + i*2] = (holding_registers[relative_start + i] >> 8) & 0xFF;
        response[2 + i*2] = holding_registers[relative_start + i] & 0xFF;
    }
    
    send_modbus_response(parser->address, parser->function, response, 1 + reg_count*2);
}

// 处理写单个保持寄存器请求
void handle_write_single_register(modbus_parser_t *parser) {
    uint16_t reg_addr = (parser->data[0] << 8) | parser->data[1];
    uint16_t reg_value = (parser->data[2] << 8) | parser->data[3];
    
    // 检查地址范围
    if (reg_addr < HOLDING_REG_START || 
        reg_addr > (HOLDING_REG_START + HOLDING_REG_SIZE)) {
        send_exception_response(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    // 转换为相对地址
    uint16_t relative_addr = reg_addr - HOLDING_REG_START;
    // 序列号特殊处理(加密)
    if (relative_addr >= 82 && relative_addr <= 84) {
//        reg_value = encrypt_serial(reg_value);
    }
    // 更新寄存器
    holding_registers[relative_addr] = reg_value;
    update_holding_registers_write();
    // 发送响应(回显相同数据)
    send_modbus_response(parser->address, parser->function, 
                         parser->data, 4);
}

// 处理写多个保持寄存器请求
void handle_write_multiple_registers(modbus_parser_t *parser) {
    uint16_t start_addr = (parser->data[0] << 8) | parser->data[1];
    uint16_t reg_count = (parser->data[2] << 8) | parser->data[3];
    uint8_t byte_count = parser->data[4];
    
    // 检查地址范围
    if (start_addr< HOLDING_REG_START || 
        start_addr + reg_count >= (HOLDING_REG_START + HOLDING_REG_SIZE) ||
        byte_count != reg_count * 2) {
        send_exception_response(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
         // 转换为相对地址
    uint16_t relative_start = start_addr;
    // for(int i = 0; i < 15; i++)
    // {
    //     log_d("i = %d,parser->data[%x]",i,parser->data[i]);
    // }
    // 更新寄存器
    for (int i = 0; i < reg_count; i++) {

        log_d("parser->data=%d,parser->data=%d",parser->data[5 + i*2],parser->data[6 + i*2]);
        uint16_t value = (parser->data[5 + i*2] << 8) | parser->data[6 + i*2];
        // 序列号特殊处理(加密)
        if (relative_start + i >= 82 && relative_start + i < 84) {
            // 此处添加加密逻辑
        }
        
        holding_registers[relative_start + i] = value;
        log_d("holding_registers[%d] = %f",relative_start + i,value);
    }
    update_holding_registers_write();
    // 发送响应(只回显地址和数量)
    send_modbus_response(parser->address, parser->function, 
                         parser->data, 4);
}

// 处理读输入寄存器请求
void handle_read_input_registers(modbus_parser_t *parser) {
    uint16_t start_addr = (parser->data[0] << 8) | parser->data[1];
    uint16_t reg_count = (parser->data[2] << 8) | parser->data[3];
    // 检查地址范围
    if (start_addr < INPUT_REG_START || start_addr + reg_count > INPUT_REG_START + INPUT_REG_SIZE) {
        send_exception_response(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    // 更新物理量寄存器
    update_input_registers();
    //获取相对位置
    uint16_t relative_start = start_addr - INPUT_REG_START;
    // 构建响应
    uint8_t response[128];
    response[0] = reg_count * 2;
    
    // 复制寄存器数据
    for (int i = 0; i < reg_count; i++) {
        response[1 + i*2] = (input_registers[relative_start + i] >> 8) & 0xFF;
        response[2 + i*2] = input_registers[relative_start + i] & 0xFF;
    }
    
    // 发送响应
    send_modbus_response(parser->address, parser->function, 
                         response, 1 + reg_count * 2);
}

// 处理读离散输入请求
void handle_read_discrete_inputs(modbus_parser_t *parser) {
    uint16_t start_addr = (parser->data[0] << 8) | parser->data[1];
    uint16_t input_count = (parser->data[2] << 8) | parser->data[3];
    
    // 检查地址范围
    if (start_addr < DISCRETE_INPUT_START || start_addr + input_count > DISCRETE_INPUT_START + DISCRETE_INPUT_SIZE) {
        send_exception_response(parser, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    uint8_t byte_count = (input_count + 7) / 8;
    uint8_t response[256];
    
    // 构建响应
    response[0] = byte_count;
    memset(&response[1], 0, byte_count);
    
    // 复制离散输入状态
    for (int i = 0; i < input_count; i++) {
        if (discrete_inputs[(start_addr + i) / 8] & (1 << ((start_addr + i) % 8))) {
            response[1 + i/8] |= (1 << (i % 8));
        }
    }
    
    // 发送响应
    send_modbus_response(parser->address, parser->function, 
                         response, byte_count + 1);
}

// 在process_modbus_frame中添加功能码处理
void process_modbus_frame(modbus_parser_t *parser) {
    // 检查地址是否匹配
    if (parser->address != MODBUS_ADDRESS && MODBUS_ADDRESS == 256) {
        // log_i("Address mismatch: %02X (expected %02X)", 
        //       parser->address, MODBUS_ADDRESS);
        return;
    }
    
    // 根据功能码处理
    switch (parser->function) {
        case MODBUS_READ_COILS:
            handle_read_coils(parser);
            break;
            
        case MODBUS_READ_DISCRETE_INPUTS:
            handle_read_discrete_inputs(parser);
            break;
            
        case MODBUS_READ_HOLDING_REGISTERS:
            handle_read_holding_registers(parser);
            break;
            
        case MODBUS_READ_INPUT_REGISTERS:
            handle_read_input_registers(parser);
            break;
            
        case MODBUS_WRITE_SINGLE_COIL:
            handle_write_single_coil(parser);
            break;
            
        case MODBUS_WRITE_SINGLE_REGISTER:
            handle_write_single_register(parser);
            break;
            
        case MODBUS_WRITE_MULTIPLE_REGISTERS:
            handle_write_multiple_registers(parser);
            break;
            
        default:
            // send_exception_response(parser, MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
            // log_w("Unsupported function: %02X", parser->function);
            break;
    }
}

// 发送Modbus响应
void send_modbus_response(uint8_t address, uint8_t function, 
                         uint8_t *data, uint8_t data_len) {
    uint8_t frame[128];
    uint16_t crc = 0xFFFF;
    int frame_len = 0;
    
    // 构建响应帧
    frame[frame_len++] = address;
    frame[frame_len++] = function;
    
    for (int i = 0; i < data_len; i++) {
        frame[frame_len++] = data[i];
    }
    
    // 计算CRC
    for (int i = 0; i < frame_len; i++) {
        crc = crc_update(crc, frame[i]);
    }
    
    frame[frame_len++] = crc & 0xFF;
    frame[frame_len++] = (crc >> 8) & 0xFF;
    
    // 通过UART发送响应
    gpio_bit_set(RS485_CONTROL_GPIO_Port,RS485_CONTROL_Pin);
    usart0_send_bytes(frame, frame_len);
    for(int i = 0; i < 10000; i++) __NOP();
    gpio_bit_reset(RS485_CONTROL_GPIO_Port,RS485_CONTROL_Pin);
}

// 发送异常响应
void send_exception_response(modbus_parser_t *parser, uint8_t exception_code) {
    uint8_t response[2];
    
    response[0] = parser->function | 0x80;  // 设置异常标志
    response[1] = exception_code;
    
    send_modbus_response(parser->address, response[0], &response[1], 1);
}