#include "myadc.h"

static const char *TAG = "ADC\t";


void continuous_adc_init(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle)
{
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config =
        {
            .max_store_buf_size = ADC_CONV_MODE_SIZE_BUF,
            .conv_frame_size = READ_LEN,
        };
    /*Эта функция может завершиться ошибкой из-за различных ошибок,
    таких как неверные аргументы, нехватка памяти и т. д.
    В частности, когда эта функция возвращает значение
    ESP_ERR_NOT_FOUND, это означает, что используется периферийное
    устройство SPI3.*/
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));
    /*
    Два БИХ-фильтра доступны, когда АЦП работает в непрерывном режиме.
    Чтобы создать фильтр ADC IIR, вы должны настроить
    adc_continuous_iir_filter_config_t и вызвать
    adc_new_continuous_iir_filter()*/
    adc_continuous_config_t dig_cfg =
        {
            .sample_freq_hz = SOC_ADC_SAMPLE_FREQ_THRES_LOW, // ожидаемая частота дискретизации АЦП в Гц.
            .conv_mode = ADC_CONV_MODE,                      // непрерывный режим преобразования.
            .format = ADC_OUTPUT_TYPE,                       // выходной формат преобразования - очень капризная штука подходит не ко всем платам
        };
    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = channel_num;
    for (int i = 0; i < channel_num; i++)
    {
        uint8_t unit = GET_UNIT(channel[i]);
        uint8_t ch = channel[i] & 0x7;
        adc_pattern[i].atten = ADC_ATTEN_DB_11; // аттенюатор 150 мВ ~ 2450 мВ
        adc_pattern[i].channel = ch;
        adc_pattern[i].unit = unit;
        adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

        ESP_LOGI(TAG, "adc_pattern[%d].atten is :%x", i, adc_pattern[i].atten);
        ESP_LOGI(TAG, "adc_pattern[%d].channel is :%x", i, adc_pattern[i].channel);
        ESP_LOGI(TAG, "adc_pattern[%d].unit is :%x", i, adc_pattern[i].unit);
    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    *out_handle = handle;
    /*Чтобы включить/отключить БИХ-фильтр АЦП, вы должны вызвать
    adc_continuous_iir_filter_enable()/
    adc_continuous_iir_filter_disable().*/   
}
