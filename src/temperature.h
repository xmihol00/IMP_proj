#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF        1100        
#define SAMPLE_SHIFT        6
#define NUMBER_OF_SAMPLES   1 << SAMPLE_SHIFT

void init_temperature();

void measure_temperature();

#endif