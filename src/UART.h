
#ifndef __UART_H__
#define __UART_H__

#include <ctype.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"
#include "esp32/rom/uart.h"

#include "data.h"
#include "SNTP.h"

#define ACTIVE_UART UART_NUM_0
#define UART_BAUD_RATE 115200
#define BUFFER_SIZE (sizeof(uint8_t) << 8)

void init_uart();

void print_status();

void uart_log_measurment(measurment_t *measurment);

void uart_print_measurment(measurment_t *measurment);

void uart_print_string(const char *string);

#endif
