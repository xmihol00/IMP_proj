//=================================================================================================================
// Soubor:      temperature.c
// Projekt:     VUT, FIT, IMP, Mereni teploty
// Datum:       2. 12. 2021
// Autor:       David Mihola
// Kontakt:     xmihol00@stud.fit.vutbr.cz
// Popis:       Funkce pro mereni teploty, obsahuje hlavni smycku programu.
//=================================================================================================================

#include "temperature.h"
#include "data.h"

#define ADC1 1  // definuje aktivni AD prevodnik, ADC2 nefunguje spolecne s WiFi

#if ADC1
    static const adc1_channel_t channel = ADC1_CHANNEL_6; // GPIO 34
#endif
#if ADC2
    static const adc2_channel_t channel = ADC2_CHANNEL_2; // GPIO 02
#endif
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;     // presnost mereni - na 12 bitu
static const adc_atten_t atten = ADC_ATTEN_DB_11;           // rozsah 150 - 2450 mV
static const adc_unit_t unit = ADC_UNIT_1;                  // vzdy ADC_UNIT_1 ESP32 nepodporuje ADC_UNIT_2
static esp_adc_cal_characteristics_t adc_chars;             // charakteristika ADC

void init_temperature()
{
    // volba dle vybraneho ADC
    #if ADC1
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten); // vyber GPIO, ktere ma byt mereno
    #endif

    #if ADC2
        adc2_config_channel_atten(channel, atten);
    #endif

    esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, &adc_chars);  // zadani charakteristiky zarizeni
}

void measure_temperature()
{
    while (1) // nekonecna smycka programu
    {
        uint32_t adc_reading = 0;
        for (int i = 0; i < NUMBER_OF_SAMPLES; i++) // opakovane mereni pro vyhlazeni sumu
        {
            // volba dle aktivniho ADC
            #if ADC2
                int raw = 0;
                adc2_get_raw(channel, width, &raw);
                adc_reading += raw;
            #endif
            #if ADC1
                adc_reading += adc1_get_raw(channel);
            #endif
        }

        adc_reading >>= SAMPLE_SHIFT; // prumer 
        
        // prevod prectene hodnoty na milivolty
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);

        float temp = (8.194 - (float)sqrt(67.141636 + 0.01048 * (1324.0 - (float)voltage))) / -0.00524 + 30;
        store_measurment(temp); // ulozeni teploty

        vTaskDelay(1000 / portTICK_RATE_MS); // cekani na dalsi sekundu
    }
}
