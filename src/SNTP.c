//=================================================================================================================
// Soubor:      SNTP.c
// Projekt:     VUT, FIT, IMP, Mereni teploty
// Datum:       2. 12. 2021
// Autor:       David Mihola
// Kontakt:     xmihol00@stud.fit.vutbr.cz
// Popis:       Funkce pro synchronizaci casu s SNTP serverem.
//=================================================================================================================

#include "SNTP.h"

static time_t last_sync = 0;    // cas posledni synchronizace s SNTP serverem
static time_t start_time = 0;   // cas spusteni zarizeni

void initialize_sntp()
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");  // nastaveni serveru pro synchronizaci
    sntp_init();
}

void set_current_time()
{
    if (wifi_is_connected()) // synchronizace probehne pouze pokud je wifi pripojena
    {
        sntp_servermode_dhcp(1);

        int retry = 0;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < 15) // cekani na odpoved ze serveru
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        setenv("TZ", "CET-1CEST,M3.5.0/02,M10.5.0/03", 1); // nastaveni formatovani casu na stredoevropske casove pasmo
        tzset();
        time(&last_sync);
        if (start_time == 0) // nastaveni casu spusteni
        {
            start_time = last_sync;
        }
    }
}

time_t get_last_sync()
{
    return last_sync;
}

time_t get_start_time()
{
    return start_time;
}
