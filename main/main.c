#include "main.h"

volatile uint8_t BaseProtBuffer[BASE_PROTOKOL_BUF_SIZE];
static void BaseProtocolSlaveTask(void *pvParameters);
// static void BaseProtocolMasterTask(void *pvParameters);
static void BlinkTask(void *pvParameters);
// static void AdcConvTask(void *pvParameters);

//static const char *TAG = "Main\t";

// static TaskHandle_t s_task_handle;
// static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
// {
//     BaseType_t mustYield = pdFALSE;
//     vTaskNotifyGiveFromISR(s_task_handle, &mustYield);
//     return (mustYield == pdTRUE);
// }

// static adc_channel_t channel[1] = {ADC_CHANNEL_7};

void app_main(void)
{
    UsartInit((uint8_t *)BaseProtBuffer, sizeof(BaseProtBuffer));
    RegsMapInit();
    BASE_INIT BaseProt;
    BaseProt.buf = BaseProtBuffer;
    BaseProt.bufSize = sizeof(BaseProtBuffer);
    BaseProt.crc_16 = crc16;
    BaseProt.TransmitData = UsartTransmitData;
    //   BaseProtMasterInit(BaseProt);
    BaseProtSlaveInit(BaseProt);
    if (WifiInitSoftAP())
    {
        return;
    }

    mqttInitAndStart(RegsMap, sizeof(RegsMap));
    xTaskCreate(BaseProtocolSlaveTask, "BaseProtocolSlaveTask", 2048, NULL, 10, NULL);
    xTaskCreate(BlinkTask, "BlinkTask", 2048, NULL, 3, NULL);
    // xTaskCreate(BaseProtocolMasterTask, "BaseProtocolMasterTask", 4096, NULL, 5, NULL);
    //   xTaskCreate(AdcConvTask, "AdcConvTask", 4096, NULL, 4, NULL);
}

static void BaseProtocolSlaveTask(void *pvParameters)
{
    while (1)
    {
        BaseProtokolSlaveHandler();
        vTaskDelay(15 / portTICK_PERIOD_MS);
    }
}
static void BlinkTask(void *pvParameters)
{
    uint8_t LedState = 0;
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while (1)
    {
        gpio_set_level(BLINK_GPIO, LedState);
        LedState = LedState > 0 ? 0 : 1;
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

// static void AdcConvTask(void *pvParameters)
// {
//     esp_err_t ret;
//     uint32_t ret_num = 0;
//     uint8_t result[READ_LEN] = {0};
//     s_task_handle = xTaskGetCurrentTaskHandle();
//     adc_continuous_handle_t handle = NULL;
//     continuous_adc_init(channel, sizeof(channel) / sizeof(adc_channel_t), &handle);
//     adc_continuous_evt_cbs_t cbs = {
//         .on_conv_done = s_conv_done_cb,
//     };
//     ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
//     ESP_ERROR_CHECK(adc_continuous_start(handle));
//     while (1)
//     {
//         ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//         ret = adc_continuous_read(handle, result, READ_LEN, &ret_num, 0);
//         if (ret == ESP_OK)
//         {
//             ESP_LOGI("TASK", "ret is %x, ret_num is %" PRIu32, ret, ret_num);
//             for (int i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES)
//             {
//                 adc_digi_output_data_t *p = (void *)&result[i];
//                 ESP_LOGI(TAG, "RealData Unit: %d, Channel: %d, Value: %f", 1, p->type1.channel, (float)p->type1.data);
//                 /*лучше использовать родные Два БИХ-фильтра, а не мою недоделку*/
//                 ESP_LOGI(TAG, "FiltData Unit: %d, Channel: %d, Value: %f", 1, p->type1.channel, AlphaBettaFiltr((float)p->type1.data));
//                 // esp_adc_cal_raw_to_voltage();
//             }
//             vTaskDelay(1);
//         }
//         else if (ret == ESP_ERR_TIMEOUT)
//         {
//             break;
//         }
//     }
//     ESP_ERROR_CHECK(adc_continuous_stop(handle));
//     ESP_ERROR_CHECK(adc_continuous_deinit(handle));
// }

// static void BaseProtocolMasterTask(void *pvParameters)
// {
//     BASE_ITEM_LISTS Item;
//     uint32_t RegsSlave[10] = {0};
//     RegsSlave[0] = 0xaabbccff;
//     Item.AddressSlave = SLAVE_ADD;
//     Item.CountRegs = 1;
//     Item.AddressRegs = &RegsSlave[0];
//     Item.SysTime = xTaskGetTickCount();
//     Item.WRval = RANDOM_READ;
//     BaseProtocolAddItemList(Item);

//     while (1)
//     {
//         BaseProtocolMasterHandler();
//         vTaskDelay(100 / portTICK_PERIOD_MS);
//         Item.SysTime = xTaskGetTickCount();
//         BaseProtocolAddItemList(Item);
//     }
// }