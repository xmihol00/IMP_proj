//=================================================================================================================
// Soubor:      main.c
// Projekt:     VUT, FIT, IMP, Mereni teploty
// Datum:       2. 12. 2021
// Autor:       David Mihola
// Kontakt:     xmihol00@stud.fit.vutbr.cz
// Popis:       Hlavni funkce programu s inicializacemi.
//=================================================================================================================

#include "UART.h"
#include "temperature.h"
#include "WiFi.h"
#include "SNTP.h"
#include "data.h"
#include "HTTP.h"
#include "file.h"

void app_main(void)
{
    // vypnuti logovani
    esp_log_level_set("*", ESP_LOG_ERROR); 
    
    // inicializace vsech casti systemu
    init_data();
    init_uart();
    init_file_system();
    wifi_connect();
    initialize_sntp();
    set_current_time();
    start_webserver();
    init_temperature();

    // vypis stavu zarizeni po inizializaci
    print_status();

    // registrace funkce pro mereni teploty
    xTaskCreate(&measure_temperature, "measure_temperature", 4096, NULL, 6, NULL);
}
