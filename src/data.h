#ifndef __DATA_H__
#define __DATA_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define SECONDS 300         // velikost cyklickeho bufferu pro sekundove vzorky
#define MINUTES 720         // velikost cyklickeho bufferu pro minutove vzorky
#define HOURS   168         // velikost cyklickeho bufferu pro hodinove vzorky
#define DAYS    365         // velikost cyklickeho bufferu pro denni vzorky

#define SEC_IN_MIN 60       // pocet sekund v za minutu
#define SEC_IN_HOUR 3600    // pocet sekund za hodinu
#define SEC_IN_DAY 86400    // pocet sekeund za den
#define MINS_IN_HOUR 60     // pocet minut za hodinu
#define HOURS_IN_DAY 24     // pocet hodin za den

/**
 * @brief Predstavuje jedno mereni.
 **/
typedef struct 
{
    float temperature;      // namerena teplota
    time_t time;            // casove razitko
}
measurment_t;

/**
 * @brief Predstavuje 4 cyklicke vyrovnavaci pameti (buffer).
 **/
typedef struct 
{
    measurment_t *seconds;      // buffer pro sekundove vzorky
    uint16_t seconds_pos;       // udava pozici v s bufferu, ktera ma byt prepsana

    measurment_t *minutes;      // buffer pro minutove vzorky
    uint16_t minutes_pos;       // udava pozici v m bufferu, ktera ma byt prepsana

    measurment_t *hours;        // buffer pro hodinove vzorky
    uint16_t hours_pos;         // udava pozici v h bufferu, ktera ma byt prepsana

    measurment_t *days;         // buffer pro denni vzorky
    uint16_t days_pos;          // udava pozici v d bufferu, ktera ma byt prepsana
}
collected_data_t;

/**
 * @brief Inicializuje (alokuje pamet bufferu) moznost ukladani vysledku mereni.
 **/
void init_data();

/**
 * @brief Ulozi provedene mereni.
 * @param temperature teplota ve stupnich C
 **/
void store_measurment(float temperature);

/**
 * @brief Nastavi prodlevu mezi vypisem informujicich zprav.
 * @param interval velikost periody
 * @param unit jednotka periody
 * @return 0 pri uspechu, nenulova hodnota pri neuspechu
 **/
uint8_t set_log_interval(uint32_t interval, char unit);

/**
 * @brief Vytiskne vzorky mereni.
 * @param count pocet vytisknutych vzorku
 * @param unit jednotka udavajici casovy rozsah vzorku
 * @return 0 pri uspechu, nenulova hodnota pri neuspechu
 **/
uint8_t print_samples(uint16_t count, char unit);

/**
 * @brief Vytisne informujici zpravu o mereni.
 **/
void print_log_interval(char *buff);

/**
 * @brief Slouzi pro ziskani namrenych dat.
 * @return vsechna namerena data.
 **/
collected_data_t *get_data();

#endif