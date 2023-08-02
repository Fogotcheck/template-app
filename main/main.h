#ifndef __MAIN_H__
#define __MAIN_H__
#include <stdio.h>
#include "esp_log.h"
#include "FreeRTOS/freertos.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "Usart/usart.h"
#include "BaseProtocol/BaseProtocol.h"
#include "CRC_polinoms/crcPolinoms.h"
#include "Wifi/wifi.h"
#include "mqtt/mqttClient.h"
#include "ADC/myadc.h"
#include "AlphaBettaFilt/calman.h"

enum REGS_MAP
{
    REG_MAP_ADDRESS_REG_0,
    REG_MAP_COUNT_REGS,
    REG_MAP_COUNT_READ_WRITE,
    REG_MAP_COUNT_ERR,
    REG_MAP_COUNT_WIFI_CONNECT,
    REG_MAP_COUNT_WIFI_DISCONNECT,
    REG_MAP_EVENT_EXCEPTION,
    COUNT_REGS,// mast be last!!!
};

#define BLINK_GPIO GPIO_NUM_2

extern uint32_t RegsMap[COUNT_REGS];

void RegsMapInit(void);


#endif //__MAIN_H__