#ifndef __DATA_H__
#define __DATA_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define SECONDS 300
#define MINUTES 60
#define HOURS   24
#define DAYS    365

#define SEC_IN_MIN 60
#define SEC_IN_HOUR 3600
#define SEC_IN_DAY 86400
#define MINS_IN_HOUR 60
#define HOURS_IN_DAY 24

typedef struct 
{
    float temperature;
    time_t time;
}
measurment_t;

typedef struct 
{
    measurment_t *seconds;
    uint16_t seconds_pos;

    measurment_t *minutes;
    uint8_t minutes_pos;

    measurment_t *hours;
    uint8_t hours_pos;

    measurment_t *days;
    uint16_t days_pos;
}
collected_data_t;

bool init_data();

void store_measurment(float temperature);

uint8_t set_log_interval(uint32_t interval, char unit);

uint8_t print_samples(uint16_t count, char unit);

void print_log_interval(char *buff);

#endif