#ifndef __MODBUS_PARSE_H
#define __MODBUS_PARSE_H

extern QueueHandle_t g_modbus_parse_queue;

void task_modbus_parse(void *parameter);

#endif