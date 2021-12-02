#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

//=================================================================================================================
// Soubor:      temperature.h
// Projekt:     VUT, FIT, IMP, Mereni teploty
// Datum:       2. 12. 2021
// Autor:       David Mihola
// Kontakt:     xmihol00@stud.fit.vutbr.cz
// Popis:       Deklarace pro soubor temperature.c
//=================================================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF        1100                    // udava referenci napeti pri prevodu pomoci AD prevodniku
#define SAMPLE_SHIFT        6                       // udava hodnotu pro posun pri prumerovani
#define NUMBER_OF_SAMPLES   1 << SAMPLE_SHIFT       // udava pocet namerenych vzorku

/**
 * @brief Inicializuje modul pro mereni teploty.
 **/ 
void init_temperature();

/**
 * @brief Provadi mereni teploty v 1 s intervalech v nekonecne smycce.
 **/
void measure_temperature();

#endif