#ifndef __SNTP_H__
#define __SNTP_H__

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

#include "WiFi.h"

/**
 * @brief Inicializuje komponenty pro moznou synchronizaci casu s SNTP serverem.
 **/
void initialize_sntp();

/**
 * @brief Synchronizuje cas modulu s SNTP serverem.
 **/
void set_current_time();

/**
 * @brief Ziska cas posledni synchronizace.
 * @return Cas posledni synchrnizace.
 **/
time_t get_last_sync();

/**
 * @brief Ziska cas spusteni modulu.
 * @return Cas spusteni modulu.
 **/
time_t get_start_time();

#endif