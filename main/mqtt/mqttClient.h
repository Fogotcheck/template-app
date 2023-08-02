#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_event.h"

#include "esp_log.h"
#include "mqtt_client.h"

typedef struct mqttClientBufRegs
{
    uint32_t *Data;
    uint16_t DataLen;
}REGS_BUFFER;


void mqttInitAndStart(uint32_t *RegsMap, uint16_t RegsMapSize);
void mqttCallBack(uint32_t address, uint32_t value);
#endif //__MQTT_CLIENT_H__
