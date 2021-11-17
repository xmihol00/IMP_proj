
#include "SNTP.h"

static time_t last_sync = 0;
static time_t start_time = 0;

static void initialize_sntp()
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

void set_current_time()
{
    sntp_servermode_dhcp(1);

    initialize_sntp();

    int retry = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < 15) 
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    setenv("TZ", "CET-1CEST,M3.5.0/02,M10.5.0/03", 1);
    tzset();
    time(&last_sync);
    if (start_time == 0)
    {
        start_time = last_sync;
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
