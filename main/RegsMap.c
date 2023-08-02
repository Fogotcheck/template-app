#include "main.h"

uint32_t RegsMap[COUNT_REGS];
static const char *TAG = "REGS_MAP\t";

void RegsMapInit(void)
{
    RegsMap[REG_MAP_ADDRESS_REG_0] = (uint32_t)&RegsMap[REG_MAP_ADDRESS_REG_0];
    RegsMap[REG_MAP_COUNT_REGS] = sizeof(RegsMap) / sizeof(RegsMap[REG_MAP_COUNT_REGS]);
    ESP_LOGI(TAG, "init end\r\n");
}

uint32_t BaseProtocolCallBack(uint32_t address, uint32_t value, uint8_t WR_val)
{
    if (address >= (sizeof(RegsMap) / sizeof(RegsMap[REG_MAP_COUNT_REGS])))
    {
        RegsMap[REG_MAP_COUNT_ERR]++;
        return 0xee;
    }
    uint32_t tmpReg = 0;
    switch (WR_val)
    {
    case RANDOM_WRITE:
    {
        RegsMap[address] = value;
        break;
    }
    case RANDOM_READ:
    {
        tmpReg = RegsMap[address];
        break;
    }
    case BLOCK_WRITE:
    {
        RegsMap[address] = value;
        break;
    }
    case BLOCK_READ:
    {
        tmpReg = RegsMap[address];
        break;
    }
    default:
        RegsMap[REG_MAP_COUNT_ERR]++;
        return 0xee;
    }
    RegsMap[REG_MAP_COUNT_READ_WRITE]++;
    return tmpReg;
}

void mqttCallBack(uint32_t address, uint32_t value)
{
    RegsMap[address] = value;
    printf("\r\nRegsMap[%ld]::0x%lx\r\n", address, value);
}

void WifiControlCallBack(uint32_t event_id, uint32_t value)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_DISCONNECTED:
    {
        RegsMap[REG_MAP_COUNT_WIFI_DISCONNECT]++;
        break;
    }
    case IP_EVENT_STA_GOT_IP:
    {
        RegsMap[REG_MAP_COUNT_WIFI_CONNECT]++;
        break;
    }
    case WIFI_EVENT_AP_STACONNECTED:
    {
        RegsMap[REG_MAP_COUNT_WIFI_CONNECT]++;
        break;
    }
    case WIFI_EVENT_AP_STADISCONNECTED:
    {
        RegsMap[REG_MAP_COUNT_WIFI_DISCONNECT]++;
        break;
    }
    default:
        RegsMap[REG_MAP_EVENT_EXCEPTION] = event_id;
        break;
    }
}
