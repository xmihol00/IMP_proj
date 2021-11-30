
#include "UART.h"
#include "temperature.h"
#include "WiFi.h"
#include "SNTP.h"
#include "data.h"
#include "HTTP.h"
#include "file.h"

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_ERROR); 
    
    init_data();
    init_uart();
    init_file_system();
    wifi_connect();
    initialize_sntp();
    set_current_time();
    start_webserver();
    init_temperature();

    print_status();

    xTaskCreate(&measure_temperature, "measure_temperature", 4096, NULL, 6, NULL);
}
