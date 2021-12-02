#ifndef __FILE_H__
#define __FILE_H__

//=================================================================================================================
// Soubor:      file.h
// Projekt:     VUT, FIT, IMP, Mereni teploty
// Datum:       2. 12. 2021
// Autor:       David Mihola
// Kontakt:     xmihol00@stud.fit.vutbr.cz
// Popis:       Deklarace pro soubor file.c
//=================================================================================================================

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_spiffs.h"

#include "WiFi.h"

#define INDEX_FILE_SIZE 10500       // udava maximalni velikost HTML stranky

/**
 * @brief Uchovava hlavni HTML stranku
 **/
typedef struct 
{
    char *data;         // HTML stranka
    uint16_t size;      // velikost v B
} main_page_t;

/**
 * @brief Inicializuje souborovy system a nacte pocatecni data z flash pameti.
 **/
void init_file_system();

/**
 * @brief Ziska hhlavni HTML stranku.
 * @return Hlavni HTML stranka.
 **/
main_page_t get_main_page();

/**
 * @brief Ulozi prihlasovaci udaje k WiFi do flash pameti.
 **/
void store_credentials();

#endif