#include "temperature.h"

static const adc1_channel_t channel = ADC_CHANNEL_6;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
static esp_adc_cal_characteristics_t adc_chars;

void init_temperature()
{
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);
    esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, &adc_chars);
}

void measure_temperature()
{
    while (1) 
    {
        uint32_t adc_reading = 0;
        for (int i = 0; i < NUMBER_OF_SAMPLES; i++) // multisampling
        {
            adc_reading += adc1_get_raw(channel);
        }

        adc_reading >>= SAMPLE_SHIFT; // prumer z multisampling
        
        // prevod prectene hodnoty na milivolty
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);

        float temp = (8.194 - (float)sqrt(67.141636 + 0.01048 * (1324.0 - (float)voltage))) / -0.00524 + 30;
        printf("Temperature: %f °C \tVoltage: %d mV\n", temp, voltage);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
