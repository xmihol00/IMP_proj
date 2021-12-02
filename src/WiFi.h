#ifndef __WIFI_H__
#define __WIFI_H__

//=================================================================================================================
// Soubor:      WiFi.h
// Projekt:     VUT, FIT, IMP, Mereni teploty
// Datum:       2. 12. 2021
// Autor:       David Mihola
// Kontakt:     xmihol00@stud.fit.vutbr.cz
// Popis:       Deklarace pro soubor WiFi.c
//=================================================================================================================

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

#define GPIO_LED_RED  2         // GPIO, na kterem se nachazi cervena LED dioda
#define CREDENTIAL_SIZE 96      // pocet znaku u prihlasovacich udaju k WiFi
#define CREDENTIAL_PASS 64      // pocet znaku hesla WiFi 
#define CREDENTIAL_NAME 32      // pocet znaku SSID WiFi

/**
 * @brief Struktura pro ulozeni uzivatelskych udaju.
 **/
typedef struct 
{
    char name[CREDENTIAL_NAME];         // SSID pro pipojeni k WiFi
    char password[CREDENTIAL_PASS];     // heslo pro pipojeni k WiFi
} credentials_t;

/**
 * @brief Pokusi se o pripojeni zarizeni k WiFi.
 **/
void wifi_connect();

/**
 * @brief Zarizeni od WiFi odpoji.
 **/
void wifi_disconnect();

/**
 * @brief Zkontroluje pripojeni k WiFi
 * @return true, pokud je zarizeni pripojeno, jinak false
 **/
bool wifi_is_connected();

#endif