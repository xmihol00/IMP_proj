
#include "UART.h"
#include "temperature.h"
#include "WiFi.h"
#include "SNTP.h"
#include "data.h"
#include "HTTP.h"

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_ERROR); 
    
    init_data();
    init_uart();
    ESP_ERROR_CHECK(wifi_connect());
    set_current_time();
    start_webserver();
    init_temperature();

    print_status();

    xTaskCreate(&measure_temperature, "measure_temperature", 4096, NULL, 5, NULL);
}
