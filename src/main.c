
#include "UART.h"
#include "temperature.h"
#include "WiFi.h"
#include "SNTP.h"
#include "data.h"

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_ERROR); 
    
    init_data();
    init_uart();
    set_current_time();
    init_temperature();

    xTaskCreate(&measure_temperature, "measure_temperature", 4096, NULL, 5, NULL);
}
