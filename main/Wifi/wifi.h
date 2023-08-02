#ifndef __WIFI_H__
#define __WIFI_H__
#include <stdio.h>
#include <string.h>
#include <esp_wifi.h>
#include "esp_mac.h"
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "lwip/inet.h"
#include "FreeRTOS/freertos.h"
#include "freertos/event_groups.h"

#define ESP_WIFI_SSID_STA_MODE "HONOR"
#define ESP_WIFI_PASSWORD_STA_MODE "12345678"
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define ESP_MAXIMUM_RETRY 10

#define ESP_WIFI_SSID "CONFIG_ESP_WIFI_SSID"
#define ESP_WIFI_PASS "mypassword"
#define ESP_WIFI_CHANNEL 5
#define MAX_STA_CONN 10

#define ESP_AP_IP_ADDRESS "192.168.0.100"
#define ESP_AP_MASK_ADDRESS "255.255.255.0"
#define ESP_AP_GW_ADDRESS "192.168.0.100"

/*Функция инициализирует NVS
NVS — это такой раздел памяти контроллера,
в котором хранятся некоторые настройки или
свойства, так как это такая файловая система,
основанная на конструкциях ключ-значение*/
esp_err_t NvsInit(void);
/*Функция инициализирует wifi в режиме STA
Режимы работы WiFi контроллера — это станция (или кратко STA)*/
esp_err_t WifiInitSta(void);
/*Функция создаёт точку доступа - настраивает работу Wifi в режиме AP*/
esp_err_t WifiInitSoftAP(void);
/*Функция контролирует подключение к точке доступа в режиме STA*/
esp_err_t WifiStatusStaControl(void);
/*Функция деинициализации wifi в режиме STA*/
esp_err_t WifiDeinitSta(void);
/*Функция деинициализации wifi в режиме AP*/
esp_err_t WifiDeinitAp(void);
/*Функция обратного вызова для контроля состояния подключения Wi-fi*/
void WifiControlCallBack(uint32_t event_id, uint32_t value);
#endif //__WIFI_H__
