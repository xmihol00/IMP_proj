
#ifndef __UART_H__
#define __UART_H__

#include <ctype.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"
#include "esp32/rom/uart.h"

#include "data.h"
#include "SNTP.h"
#include "WiFi.h"
#include "file.h"

#define ACTIVE_UART UART_NUM_0                  // definuje aktivni zarizeni UART
#define UART_BAUD_RATE 115200                   // pouzity baud rate pro prenos seriovou komunikaci
#define BUFFER_SIZE (sizeof(uint8_t) << 8)      // velikost bufferu pro prijem a odesilani zprav
#define NO_WIFI 0                               // definuje, ze aktualne neprobiha konfigurace WiFi
#define WIFI_UNAME 1                            // definuje, ze aktualne probiha konfigurace SSID WiFi
#define WIFI_PASSWORD 2                         // definuje, ze aktualne probiha konfigurace hesla WiFi

/**
 * @brief Inicializuje HW zarizeni UART.
 **/
void init_uart();

/**
 * @brief Vytisne aktualni stav zarizeni.
 **/
void print_status();

/**
 * @brief Vypise informaci zpravu s namerenou teplotou.
 * @param measurment obsahuje namerenou hodnotu a casove razitko.
 **/
void uart_log_measurment(measurment_t *measurment);


/**
 * @brief Vytiskne namerenou teplotou.
 * @param measurment obsahuje namerenou hodnotu a casove razitko.
 **/
void uart_print_measurment(measurment_t *measurment);

/**
 * @brief Vytiskne textovy retezec.
 **/
void uart_print_string(const char *string);

#endif
