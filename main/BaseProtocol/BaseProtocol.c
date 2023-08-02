#include "BaseProtocol.h"

void (*SlaveTransmitData)(uint8_t *data, uint16_t data_len);
void (*MasterTransmitData)(uint8_t *data, uint16_t data_len);
uint16_t (*crc_16)(uint8_t *buffer, uint16_t count);
static const char *TAG = "BASE_PROT\t";

BASE_BUFFER SlaveBuf, MasterBuf;

__attribute__((weak)) uint32_t BaseProtocolCallBack(uint32_t address, uint32_t value, uint8_t WR_val)
{
    ESP_LOGI(TAG, "TEST MODE Address::%ld,value::%ld", address, value);
    return 0xaabbccff;
}

BASE_ITEM_LISTS BaseLists[BASE_LISTS_SIZE];
void BaseProtSlaveInit(BASE_INIT init)
{
    SlaveTransmitData = init.TransmitData;
    crc_16 = init.crc_16;
    SlaveBuf.Data = init.buf;
    SlaveBuf.DataLen = init.bufSize;
    ESP_LOGI(TAG, "init end\r\n");
}
void BaseProtMasterInit(BASE_INIT init)
{
    MasterTransmitData = init.TransmitData;
    crc_16 = init.crc_16;
    MasterBuf.Data = init.buf;
    MasterBuf.DataLen = init.bufSize;
}

uint8_t BaseCheckHead(uint8_t **ptrHead, BASE_BUFFER buf)
{
    uint8_t *start = (uint8_t *)buf.Data;
    uint8_t *end = start + buf.DataLen;
    BASE_HEAD *head = (BASE_HEAD *)start;
    while (start < end)
    {
        if (crc_16(start, (sizeof(BASE_HEAD) - sizeof(uint16_t))) == head->crc)
        {
            *ptrHead = start;
            return BASE_SUCCSESS;
        }
        start++;
        head = (BASE_HEAD *)start;
    }
    return BASE_ERROR;
}

uint32_t XorData(uint32_t *buf, uint16_t size)
{
    uint32_t d_xor = 0;
    size /= sizeof(uint32_t);
    uint16_t i = 0;
    for (i = 0; i < size; i++)
    {
        d_xor ^= buf[i];
    }
    return d_xor;
}

uint8_t BaseCheckData(uint32_t *mess, uint16_t data_len)
{
    BASE_PACKET *pack = (BASE_PACKET *)mess;
    uint32_t d_xor = 0;
    uint32_t xor_buf = pack->data[pack->head.len / sizeof(uint32_t)];
    if ((d_xor = XorData(&pack->data[0], pack->head.len)) != xor_buf)
    {
        return d_xor;
    }
    return BASE_SUCCSESS;
}

void BaseProtokolSlaveHandler(void)
{
    static uint8_t count_errors;
    uint8_t *tmpMess;
    BASE_PACKET *pack = (BASE_PACKET *)SlaveBuf.Data;
    uint32_t tmpValue = 0;
    uint32_t address = 0;
    if (count_errors >= COUNT_ERRORS_MESS)
    {
        pack->head.crc = 0;
    }

    if (BaseCheckHead(&tmpMess, SlaveBuf) != BASE_SUCCSESS)
    {
        return;
    }
    pack = (BASE_PACKET *)tmpMess;
    if (BaseCheckData((uint32_t *)tmpMess, pack->head.len) != BASE_SUCCSESS)
    {
        count_errors++;
        return;
    }
    count_errors = 0;
    pack = (BASE_PACKET *)tmpMess;
    if ((pack->head.dest != MASTER_ADD) && (pack->head.dest != SLAVE_ADD) && (pack->head.dest != ALL_ADD))
    {
        pack->head.crc = 0;
        return;
    }
    SWAP(pack->head.dest, pack->head.source);
    pack->head.len = 0;
    pack->head.crc = 0;
    switch (pack->head.type)
    {
    case RANDOM_WRITE:
    {
        BaseProtocolCallBack(pack->data[0], pack->data[1], pack->head.type);
        break;
    }
    case RANDOM_READ:
    {
        tmpValue = BaseProtocolCallBack(pack->data[0], pack->data[1], pack->head.type);
        pack->data[1] = tmpValue;
        pack->head.len += 2;
        break;
    }
    case BLOCK_WRITE:
    {
        address = pack->data[0];
        for (uint16_t i = 0, j = 2; i < pack->data[1]; i++, j++)
        {
            BaseProtocolCallBack(address++, pack->data[j], pack->head.type);
        }
        break;
    }
    case BLOCK_READ:
    {
        address = pack->data[0];
        for (uint16_t i = 0, j = 2; i < pack->data[1]; i++, j++)
        {
            tmpValue = BaseProtocolCallBack(address++, pack->data[j], pack->head.type);
            pack->data[j] = tmpValue;
            pack->head.len++;
        }
        break;
    }
    default:
        break;
    }
    pack->head.type++;
    pack->head.tag++;
    pack->head.len *= sizeof(uint32_t);
    pack->head.crc = crc_16((uint8_t *)pack, sizeof(BASE_HEAD) - sizeof(uint16_t));
    pack->data[(pack->head.len / sizeof(uint32_t))] = XorData(&pack->data[0], pack->head.len);
    SlaveTransmitData((uint8_t *)pack, sizeof(BASE_HEAD) + pack->head.len + sizeof(uint32_t));
    pack->head.crc = 0;
    return;
}

uint8_t BaseProtocolAddItemList(BASE_ITEM_LISTS Item)
{
    static uint8_t ListCount;
    if (ListCount >= sizeof(BaseLists) / sizeof(BaseLists[0]))
    {
        ListCount = 0;
    }
    BaseLists[ListCount] = Item;
    ListCount++;
    return BASE_SUCCSESS;
}
uint8_t BaseFindNextItem(void)
{
    for (uint16_t i = 0; i < sizeof(BaseLists) / sizeof(BaseLists[0]); i++)
    {
        if (BaseLists[i].WRval != 0)
        {
            return i;
        }
    }
    return sizeof(BaseLists) / sizeof(BaseLists[0]) + 1;
}

void GenerateTrasmitPack(BASE_ITEM_LISTS Item)
{
    static uint16_t tag = 0;
    BASE_PACKET *SendPack = (BASE_PACKET *)MasterBuf.Data;
    SendPack->head.dest = Item.AddressSlave;
    SendPack->head.source = MASTER_ADD;
    SendPack->head.len = Item.CountRegs * sizeof(uint32_t) * 2;
    SendPack->head.type = Item.WRval;
    SendPack->head.tag = tag;
    SendPack->head.crc = crc_16((uint8_t *)SendPack, sizeof(BASE_HEAD) - sizeof(uint16_t));
    tag++;
    for (uint8_t i = 0, j = 0; i < Item.CountRegs; i += 2, j++)
    {
        SendPack->data[i] = (uint32_t)Item.AddressRegs;
        SendPack->data[i + 1] = Item.AddressRegs[j];
    }
    uint32_t * xor = (uint32_t *)(MasterBuf.Data + sizeof(BASE_HEAD) + SendPack->head.len);
    *xor = XorData(SendPack->data, SendPack->head.len);
    MasterTransmitData((uint8_t *)MasterBuf.Data, sizeof(BASE_HEAD) + SendPack->head.len + sizeof(*xor));
    SendPack->head.crc = 0;
}

uint8_t RecievAnswerPack(BASE_ITEM_LISTS *Item)
{
    uint8_t *tmpMess;
    if (BaseCheckHead(&tmpMess, MasterBuf) != BASE_SUCCSESS)
    {
        return BASE_ERROR;
    }
    BASE_PACKET *RecievPack = (BASE_PACKET *)tmpMess;
    if (BaseCheckData((uint32_t *)tmpMess, RecievPack->head.len) != BASE_SUCCSESS)
    {
        return BASE_ERROR;
    }
    Item->WRval = 0;
    return BASE_SUCCSESS;
}

void BaseProtocolMasterHandler(void)
{
    static uint8_t status = 0xff;
    static uint8_t CurrentItems = 0;
    switch (status)
    {
    case BASE_SEND:
    {
        GenerateTrasmitPack(BaseLists[CurrentItems]);
        status++;
        break;
    }
    case BASE_RECIEV:
    {
        uint32_t Ticks = xTaskGetTickCount();
        if (RecievAnswerPack(&BaseLists[CurrentItems]) == BASE_SUCCSESS)
        {
            status++;
        }
        else if ((Ticks - BaseLists[CurrentItems].SysTime) >= (3000 / portTICK_PERIOD_MS))
        {
            /*TODO: write BaseErrosHandler*/
            BaseLists[CurrentItems].WRval = 0;
            status++;
        }
        break;
    }
    default:
    {
        if ((CurrentItems = BaseFindNextItem()) < sizeof(BaseLists) / sizeof(BaseLists[0]))
        {
            status = BASE_SEND;
        }
        break;
    }
    }
}