
#include "UART.h"
#include "temperature.h"
#include "WiFi.h"
#include "SNTP.h"

static char *TAG = "main";

void app_main(void)
{
    init_uart();
    
    set_current_time();
    time_t now;
    struct tm timeinfo;
    
    time(&now);

    char strftime_buf[64];

    setenv("TZ", "CET-1CEST,M3.5.0/02,M10.5.0/03", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Brno is: %s", strftime_buf);

    init_temperature();

    xTaskCreate(&measure_temperature, "measure_temperature", 2048, NULL, 5, NULL);
}
