
#ifndef __UART_H__
#define __UART_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"
#include "esp32/rom/uart.h"

#define ACTIVE_UART UART_NUM_0
#define UART_BAUD_RATE 115200
#define BUFFER_SIZE 128

void init_uart();

#endif
