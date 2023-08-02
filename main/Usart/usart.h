#ifndef __USART_H__
#define __USART_H__
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"


#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_3)
#define RX_BUF_SIZE 1024
#define TX_BUF_SIZE 1024

typedef struct UsartBuf
{
    uint8_t *ptr;
    uint16_t size;
}USART_BUF;


void UsartInit(uint8_t *Buffer, uint16_t BufferSize);
void UsartTransmitData(uint8_t *Data, uint16_t DataLen);
#endif //__USART_H__