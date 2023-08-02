#ifndef __BaseProtocol_h__
#define __BaseProtocol_h__
#include <stdio.h>
#include "esp_log.h"
#include <string.h>
#include "FreeRTOS/freertos.h"
#include "freertos/task.h"

#define BASE_PROTOKOL_BUF_SIZE 1024
#define COUNT_ERRORS_MESS 255
#define BASE_SUCCSESS 0
#define BASE_ERROR 1
#define BASE_LISTS_SIZE 32

#define SWAP(a, b) \
    {              \
        a ^= b;    \
        b ^= a;    \
        a ^= b;    \
    }

typedef struct base_head
{
    uint32_t dest : 32;
    uint32_t source : 32;
    uint32_t len : 16;
    uint32_t type : 16;
    uint32_t tag : 16;
    uint32_t crc : 16;
} BASE_HEAD;

typedef struct BaseBuf
{
    volatile uint8_t * Data;
    uint16_t DataLen;
}BASE_BUFFER;


typedef struct BaseProtocolItemList
{
    uint32_t AddressSlave;
    uint32_t CountRegs;
    uint32_t *AddressRegs;
    uint32_t SysTime;
    uint8_t WRval;
} BASE_ITEM_LISTS;

typedef struct base_packet
{
    BASE_HEAD head;
    uint32_t data[];
} BASE_PACKET;

typedef struct BaseProtocolIni
{
    void (*TransmitData)(uint8_t *data, uint16_t data_len);
    uint16_t (*crc_16)(uint8_t *buffer, uint16_t count);
    volatile uint8_t *buf;
    uint16_t bufSize;
} BASE_INIT;

enum MASTER_DEFINES
{
    BASE_SEND = 1,
    BASE_RECIEV,

};

enum TYPE_MESSAGE
{
    RANDOM_WRITE = 3,
    RANDOM_READ = 5,
    BLOCK_WRITE = 7,
    BLOCK_READ = 9,
};

enum ADDRESS_LIST
{
    ALL_ADD = 0,
    MASTER_ADD = 0x12340000,
    SLAVE_ADD = 0x12340001,
};

void BaseProtSlaveInit(BASE_INIT init);
void BaseProtMasterInit(BASE_INIT init);
void BaseProtokolSlaveHandler(void);
uint8_t BaseProtocolAddItemList(BASE_ITEM_LISTS Item);
void BaseProtocolMasterHandler(void);

uint32_t BaseProtocolCallBack(uint32_t address, uint32_t value, uint8_t WR_val);
#endif //__BaseProtocol_h__