/**
 * @file    SPI_FPGA_ReceiverTask.c
 * @author  Ye Dingzai
 * @brief   SPI 接收 FPGA 数据任务（DMA方式）
 * @date    2026-02-02
 *
 * 功能说明：
 * 1) FPGA 在有一帧数据准备好后，通过 FPGA_INT 引脚触发中断通知 MCU。
 *    在 FPGA_INT 的 EXTI 中断服务函数（ISR）中会释放 xSem_FPGA_INT 信号量。
 *
 * 2) 本任务（vSPI_Rx_task）一直阻塞等待 xSem_FPGA_INT：
 *    - 一旦拿到信号量，说明 FPGA 提示“可以开始读数据”
 *    - 拉低片选 CS（NSS），选通 FPGA
 *    - 调用 HAL_SPI_Receive_DMA() 启动 DMA 接收固定长度的数据帧
 *    - 阻塞等待 DMA 完成信号量 xSem_SPI_Rx_Done
 *      （通常在 HAL_SPI_RxCpltCallback() 回调里释放该信号量）
 *    - DMA 接收完成后拉高 CS，结束本次 SPI 帧读
 *
 * 3) 将 DMA 接收缓冲区中的字节流拷贝到 rufx_raw_packet_t 结构体 packet 中，
 *    然后通过队列 xQueue_Rx_Index_Buf 发送给算法任务（algorithmTask）。
 *
 * 关键点说明：
 * - Rx_Index_Buf 是 SPI DMA 的接收缓冲区，DMA 会直接把 SPI 接收到的数据写入该数组。
 * - xQueueSend() 发送的是 packet 结构体“内容的拷贝”，不是指针。
 *   前提是创建队列时 item_size = sizeof(rufx_raw_packet_t)。
 * - 本版本按 8-bit 数据帧接收，RUF_X_PACKET_SIZE_BYTES 表示一帧数据的字节数。
 */

#include "gd32f30x.h"

#include "RUF_X_demo.h"
#include "SPI_RxTask.h"
#include "algorithmTask.h"

#include "spi.h"
#include "exti.h"
#include "dma.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include <string.h>

#include "freertos_resources.h"

uint8_t Rx_Index_Buf[RUF_X_PACKET_SIZE_BYTES]; // SPI DMA 接收缓冲区（按字节存放）
#define RX_INDEX_SIZE RUF_X_PACKET_SIZE_BYTES  // 本次 DMA 接收的字节数（与 FPGA 协议包长度一致）

void vSPI_Rx_task(void *pvParameter)
{
    (void)pvParameter;

    while (1)
    {
        // 1) 等待 FPGA_INT 中断对应的信号量（该信号量一般在 EXTI ISR 中释放）
        xSemaphoreTake(xSem_FPGA_INT, portMAX_DELAY);

        // 2) 拉低 CS，选通 FPGA，开始一次 SPI 帧读
        SPI0_CS_ENABLE();

        // 3) 启动 SPI DMA 接收：DMA 自动把 SPI RX 数据搬运到 Rx_Index_Buf
        GD32_SPI0_Receive_DMA(Rx_Index_Buf, RX_INDEX_SIZE);

        // 4) 等待 DMA 接收完成（通常在 HAL_SPI_RxCpltCallback() 中释放 xSem_SPI_Rx_Done）
        xSemaphoreTake(xSem_SPI_Rx_Done, portMAX_DELAY);

        // 5) 拉高 CS，结束本次 SPI 帧读
        SPI0_CS_DISABLE();

        // 6) 将接收缓冲区内容打包成协议结构体（按字节拷贝）
        rufx_raw_packet_t packet;
        memcpy(packet.bytes, Rx_Index_Buf, RUF_X_PACKET_SIZE_BYTES);

        // 7) 通过队列发送给算法任务：
        //    队列内部会拷贝 sizeof(packet) 字节，因此 packet 是局部变量也没关系
        xQueueSend(xQueue_Rx_Index_Buf, &packet, portMAX_DELAY);
    }
}
