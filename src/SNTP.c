
#include "SNTP.h"

static void initialize_sntp()
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

void set_current_time()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    sntp_servermode_dhcp(1);
    ESP_ERROR_CHECK(wifi_connect());

    initialize_sntp();

    int retry = 0;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < 15) 
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    setenv("TZ", "CET-1CEST,M3.5.0/02,M10.5.0/03", 1);
    tzset();

    ESP_ERROR_CHECK(wifi_disconnect());
}
