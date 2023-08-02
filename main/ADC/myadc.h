#ifndef __MY_ADC_H__
#define __MY_ADC_H__
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "soc/soc_caps.h"

#include "driver/gpio.h"
/*
- Некоторые из контактов ADC2 используются в качестве контактов
для обвязки (GPIO 0, 2, 15), поэтому их нельзя использовать
свободно. Так обстоит дело в следующих официальных комплектах разработки:

- ESP32 DevKitC: GPIO 0 нельзя использовать из-за внешних схем
автоматического программирования.

- ESP-WROVER-KIT: GPIO 0, 2, 4 и 15 нельзя использовать из-за
внешних подключений для разных целей. Поскольку модуль ADC2
также используется Wi-Fi, только один из них может получить
преимущество при совместном использовании, что означает, что
он adc2_get_raw()может быть заблокирован до тех пор, пока
Wi-Fi не остановится, и наоборот.

- Обратите внимание, что даже датчик Холла является внутренним
для ESP32, для чтения с него используются каналы 0 и 3 ADC1
(GPIO 36 и 39). Не подключайте к этим контактам ничего другого
и не меняйте их конфигурацию. В противном случае это может
повлиять на измерение сигнала низкого значения от датчика.

- ESP32: для данного канала adc2_config_channel_atten()необходимо
вызвать перед первым вызовом этой функции. Если Wi-Fi запускается
через esp_wifi_start(), эта функция всегда будет давать сбой с
ESP_ERR_TIMEOUT.
*/

#define ADC_CONV_MODE_SIZE_BUF 1024
#define READ_LEN 256

#define ADC_OUTPUT_TYPE ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define ADC_CONV_MODE ADC_CONV_BOTH_UNIT
#define GET_UNIT(x) ((x >> 3) & 0x1)

void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle);

#endif //__MY_ADC_H__
