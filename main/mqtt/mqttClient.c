#include "mqttClient.h"

static const char *TAG = "MQTT\t";

REGS_BUFFER buf;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

__attribute__((weak)) void mqttCallBack(uint32_t address, uint32_t value)
{
    ESP_LOGI(TAG, "TEST MODE Address::%ld,value::0x%lx", address, value);
}

void PublicAllRegs(esp_mqtt_client_handle_t *client)
{
    for (uint8_t i = 0; i < buf.DataLen / sizeof(buf.Data[0]); i++)
    {
        char TopicName[32] = {0};
        char tmpReg[32] = {0};
        sprintf(TopicName, "/RegsMap/Reg[%3u]", i);
        sprintf(tmpReg, "val::0x%8lx", buf.Data[i]);
        esp_mqtt_client_publish(*client, TopicName, tmpReg, strlen(tmpReg), 1, 0);
    }
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    // ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        PublicAllRegs(&client);
        msg_id = esp_mqtt_client_subscribe(client, "/RegsMap/#", 1);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
    {
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        uint32_t Address = 0, Value = 0;
        sscanf(event->topic, "/RegsMap/Reg[%ld]", &Address);
        sscanf(event->data, "0x%lx", &Value);
        mqttCallBack(Address, Value);
        break;
    }
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqttInitAndStart(uint32_t *RegsMap, uint16_t RegsMapSize)
{
    buf.Data = RegsMap;
    buf.DataLen = RegsMapSize;
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://192.168.0.3:1883",
        .credentials.username = "mosquitto",
        .credentials.authentication.password = "123456",
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}