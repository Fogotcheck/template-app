#include "usart.h"

static const char *TAG = "UART\t";
static QueueHandle_t uart0_queue;
volatile uint8_t *RX0Buf;
uint16_t RX0BufSize;
static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    for (;;)
    {
        if (xQueueReceive(uart0_queue, (void *)&event, (TickType_t)portMAX_DELAY))
        {
            bzero((uint8_t *)RX0Buf, RX0BufSize);
            switch (event.type)
            {
            case UART_DATA:
                uart_read_bytes(UART_NUM_0, (uint8_t *)RX0Buf, event.size, portMAX_DELAY);
                break;
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                uart_flush_input(UART_NUM_0);
                xQueueReset(uart0_queue);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                uart_flush_input(UART_NUM_0);
                xQueueReset(uart0_queue);
                break;
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            case UART_PATTERN_DET:
                break;
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
    vTaskDelete(NULL);
}

void UsartInit(uint8_t *Buffer, uint16_t BufferSize)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    RX0BufSize = BufferSize;
    RX0Buf = Buffer;
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, BufferSize * 2, BufferSize * 2, 20, &uart0_queue, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);
    ESP_LOGI(TAG, "init end\r\n");
}

void UsartTransmitData(uint8_t *Data, uint16_t DataLen)
{
    uart_write_bytes(UART_NUM_0, Data, DataLen);
}