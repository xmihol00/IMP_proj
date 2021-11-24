#ifndef __WIFI_H__
#define __WIFI_H__

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "esp_wifi_default.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_wifi.h"
#include "esp_err.h"

#define GPIO_LED_RED  2
#define CREDENTIAL_SIZE 96
#define CREDENTIAL_PASS 64
#define CREDENTIAL_NAME 32

typedef struct 
{
    char name[CREDENTIAL_NAME];
    char password[CREDENTIAL_PASS];
} credentials_t;


esp_err_t wifi_connect();

esp_err_t wifi_disconnect();

bool wifi_is_connected();

#endif