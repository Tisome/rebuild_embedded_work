#include "elog.h"
#include "stdio.h"
#include "my_modbus.h"
// 更高效的逐字节CRC计算
uint16_t crc_update(uint16_t crc, uint8_t data) {
    crc ^= (uint16_t)data;
    for (int i = 0; i < 8; i++) {
        if (crc & 0x0001) {
            crc = (crc >> 1) ^ 0xA001;
        } else {
            crc >>= 1;
        }
    }
    return crc;
}