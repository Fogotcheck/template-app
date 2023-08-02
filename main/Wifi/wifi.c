#include "wifi.h"

static const char *TAG = "Wifi\t";

static EventGroupHandle_t s_wifi_event_group;
int CountRetryConneck = 0;
int s_active_interfaces = 0;

esp_err_t NvsInit(void)
{
    esp_err_t ret = nvs_flash_init_partition("nvs");
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize NVS partition: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_ERROR_CHECK(esp_netif_init());
    return ret;
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
    {
        esp_wifi_connect();
        break;
    }
    case WIFI_EVENT_STA_DISCONNECTED:
    {
        if (CountRetryConneck < ESP_MAXIMUM_RETRY)
        {
            /*если так не нравится удали if*/
            esp_wifi_connect();
            CountRetryConneck++;
            WifiControlCallBack((uint32_t)event_id, CountRetryConneck);
            // ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        // ESP_LOGI(TAG, "connect to the AP fail");
        break;
    }
    case IP_EVENT_STA_GOT_IP:
    {
        // ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        //  ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        WifiControlCallBack((uint32_t)event_id, 0);
        CountRetryConneck = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    }
    case WIFI_EVENT_AP_STACONNECTED:
    {

        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        WifiControlCallBack((uint32_t)event_id, event->aid);
        // ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
        //          MAC2STR(event->mac), event->aid);
        break;
    }
    case WIFI_EVENT_AP_STADISCONNECTED:
    {

        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        WifiControlCallBack((uint32_t)event_id, event->aid);
        // ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
        //          MAC2STR(event->mac), event->aid);
        break;
    }
    default:
        WifiControlCallBack((uint32_t)event_id, 0);
        break;
    }
}

esp_err_t WifiInitSta(void)
{
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    if (NvsInit())
    {
        return ESP_ERR_INVALID_STATE;
    }
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID_STA_MODE,
            .password = ESP_WIFI_PASSWORD_STA_MODE,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "init end\r\n");
    s_wifi_event_group = xEventGroupCreate();
    return ESP_OK;
}

esp_err_t WifiInitSoftAP(void)
{
    if (NvsInit())
    {
        return ESP_ERR_INVALID_STATE;
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    esp_netif_dhcps_stop(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"));
    esp_netif_ip_info_t ip_info = {
        .ip = {
            .addr = ipaddr_addr(ESP_AP_IP_ADDRESS)},
        .netmask = {.addr = ipaddr_addr(ESP_AP_MASK_ADDRESS)},
        .gw = {.addr = ipaddr_addr(ESP_AP_GW_ADDRESS)}};
    esp_netif_set_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);
    esp_netif_dhcps_start(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"));
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = ESP_WIFI_SSID,
            .ssid_len = strlen(ESP_WIFI_SSID),
            .channel = ESP_WIFI_CHANNEL,
            .password = ESP_WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .required = false,
            },
        },
    };
    if (strlen(ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "init end\r\n");
    return ESP_OK;
}

esp_err_t WifiStatusStaControl(void)
{
    wifi_ap_record_t info;
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_sta_get_ap_info(&info));
    return ESP_OK;
}

esp_err_t WifiDeinitSta(void)
{
    esp_err_t ret = ESP_OK;
    // Остановка WiFi
    ret = esp_wifi_stop();
    if (ret != ESP_OK)
    {
        return ret;
    }
    esp_netif_destroy_default_wifi(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"));
    // Удаление обработчиков событий WiFi
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler);
    esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler);

    // Удаление группы событий WiFi
    vEventGroupDelete(s_wifi_event_group);

    return ret;
}

esp_err_t WifiDeinitAp(void)
{
    esp_err_t ret = ESP_OK;

    // Остановка WiFi
    ret = esp_wifi_stop();
    if (ret != ESP_OK)
    {
        return ret;
    }
    // Удаление интерфейса WiFi в режиме AP:
    esp_netif_destroy_default_wifi(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"));

    // Удаление обработчиков событий WiFi
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler);

    return ret;
}

__attribute__((weak)) void WifiControlCallBack(uint32_t event_id, uint32_t value)
{
    ESP_LOGI(TAG, "TEST MODE event_id::%ld,value::%ld", event_id, value);
}