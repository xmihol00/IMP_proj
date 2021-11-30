
#include "data.h"

static uint32_t log_interval = 0;   // aktualni perioda vypisu mereni v s
static collected_data_t data;       // vsechna namerena data

/**
 * @brief Zprumeruje namerene vzorky za posledni minutu.
 **/
static void average_last_minute();

/**
 * @brief Zprumeruje namerene vzorky za posledni hodinu.
 **/
static void average_last_hour();

/**
 * @brief Zprumeruje namerene vzorky za posledni den.
 **/
static void average_last_day();

// funkce z UART.H - reseni cyklicke zavislosti
extern void uart_log_measurment(measurment_t *);
extern void uart_print_measurment(measurment_t *);
extern void uart_print_string(const char *);

esp_err_t init_data()
{
    data.days_pos = 0;
    data.hours_pos = 0;
    data.minutes_pos = 0;
    data.seconds_pos = 0;

    // alokace a inicializace pameti pro cyklicke VP
    data.days = calloc(DAYS, sizeof(measurment_t));
    data.hours = calloc(HOURS, sizeof(measurment_t));
    data.minutes = calloc(MINUTES, sizeof(measurment_t));
    data.seconds = calloc(SECONDS, sizeof(measurment_t));

    return data.days && data.hours && data.minutes && data.seconds ? ESP_OK : ESP_FAIL;
}

void store_measurment(float temperature)
{
    data.seconds[data.seconds_pos].temperature = temperature;
    time(&data.seconds[data.seconds_pos].time);     // sejmuti casoveho vzorku

    if (!(data.seconds[data.seconds_pos].time % SEC_IN_MIN))    // dovrseni minuty
    {
        average_last_minute();

        if (!(data.seconds[data.seconds_pos].time % SEC_IN_HOUR))  // dovrseni hodiny
        {
            average_last_hour();

            if (!((data.seconds[data.seconds_pos].time + SEC_IN_HOUR) % SEC_IN_DAY))  // dovrseni dne
            {
                average_last_day();
            }
        }
    }

    if (log_interval && !(data.seconds[data.seconds_pos].time % log_interval)) // je povoleno logovani a byl dovrsen casovy interval 
    {
        uart_log_measurment(&data.seconds[data.seconds_pos]); 
    }

    if (++data.seconds_pos == SECONDS)
    {
        data.seconds_pos = 0; // rotace bufferu
    }
}

static void average_last_minute()
{
    uint16_t pos = data.seconds_pos - SEC_IN_MIN + SECONDS; // posun o minutu zpet v sekundovem bufferu

    float collected = 0.0;
    uint8_t measurments = 0;
    for (uint8_t i = 0; i < SEC_IN_MIN; i++)
    {
        pos++;
        if (data.seconds[pos % SECONDS].time != 0) // hodnota je jiz zapsana
        {
            collected += data.seconds[pos % SECONDS].temperature;
            measurments++;
        }
    }

    data.minutes[data.minutes_pos].temperature = collected / measurments;       // prumer
    data.minutes[data.minutes_pos].time = data.seconds[data.seconds_pos].time;
    
    if (++data.minutes_pos == MINUTES)
    {
        data.minutes_pos = 0; // rotace minutoveho bufferu
    }
}

static void average_last_hour()
{
    uint16_t pos = data.minutes_pos - MINS_IN_HOUR + MINUTES; // posun o hodinu zpet v minutovem bufferu

    float collected = 0.0;
    uint8_t measurments = 0;
    for (uint8_t i = 0; i < MINS_IN_HOUR; i++)
    {
        if (data.minutes[pos % MINUTES].time != 0) // hodnota jiz byla nemrena
        {
            collected += data.minutes[pos % MINUTES].temperature;
            measurments++;
        }
        pos++;
    }

    data.hours[data.hours_pos].temperature = collected / measurments;  // prumer
    data.hours[data.hours_pos].time = data.minutes[data.minutes_pos - 1].time;
    
    if (++data.hours_pos == HOURS)
    {
        data.hours_pos = 0; // rotace hodinoveho bufferu
    }
}

static void average_last_day()
{
    uint16_t pos = data.hours_pos - HOURS_IN_DAY + HOURS; // posun o den zpet v hodinovem bufferu

    float collected = 0.0;
    uint8_t measurments = 0;
    for (uint8_t i = 0; i < HOURS_IN_DAY; i++)
    {
        if (data.hours[pos % HOURS].time != 0)  // hodnota jiz byla namerena
        {
            collected += data.hours[pos % HOURS].temperature;
            measurments++;
        }
        pos++;
    }

    data.days[data.days_pos].temperature = collected / measurments;     // prumer
    data.days[data.days_pos].time = data.hours[data.hours_pos - 1].time;
    
    if (++data.days_pos == DAYS)
    {
        data.days_pos = 0; // rotace v dennim bufferu
    }
}

uint8_t set_log_interval(uint32_t interval, char unit)
{
    if (interval == 0) // vypnuti logovani
    {
        log_interval = 0;
        return 0;
    }

    switch (unit)
    {
        case 'd':
            log_interval = interval * SEC_IN_DAY; // logovani jednou za 'inteval' dnu prevedeno na sekundy
            break;

        case 'h':
            log_interval = interval * SEC_IN_HOUR; // logovani jednou za 'inteval' hodin prevedeno na sekundy
            break;

        case 'm':
            log_interval = interval * SEC_IN_MIN; // logovani jednou za 'inteval' minut prevedeno na sekundy
            break;
        
        case 's':
            log_interval = interval; // logovani jednou za 'inteval' sekund
            break;

        default:
            return 4;   // nespravne specifikovana jednotka logovani
    }

    return 0;
}

uint8_t print_samples(uint16_t count, char unit)
{
    char buffer[64] = {0, };
    uint16_t modulator = 0;
    int16_t pos = 0;
    measurment_t *selected = NULL;

    switch (unit)
    {
        case 'd':
            if (count <= DAYS) // specifikovany pocet dnu lze vytiskout, je mensi nez velikost denniho bufferu
            {
                selected = data.days;
                pos = data.days_pos - count + DAYS;  // posun o 'count' dni zpet v dennim bufferu
                modulator = DAYS;
                // hlavicka pro denni vypis
                sprintf(buffer, "|    AVERAGE TEMPERATURE OVER LAST %d DAY%c\t|\r\n", count, count > 1 ? 'S' : ' ');
                break;
            }
            return 4;

        case 'h':
            if (count <= HOURS) // specifikovany pocet dnu lze vytiskout, je mensi nez velikost denniho bufferu
            {
                selected = data.hours;
                pos = data.hours_pos - count + HOURS; // posun o 'count' hodin zpet v hodinovem bufferu
                modulator = HOURS;
                // hlavicka pro hodinovy vypis
                sprintf(buffer, "|    AVERAGE TEMPERATURE OVER LAST %d HOUR%c\t|\r\n", count, count > 1 ? 'S' : ' '); 
                break;
            }
            return 4;

        case 'm':
            if (count <= MINUTES) // specifikovany pocet dnu lze vytiskout, je mensi nez velikost denniho bufferu
            {
                selected = data.minutes;
                pos = data.minutes_pos - count + MINUTES; // posun o 'count' minut zpet v minutovem bufferu
                modulator = MINUTES;
                // hlavicka pro minutovy vypis
                sprintf(buffer, "|   AVERAGE TEMPERATURE OVER LAST %d MINUTE%c\t|\r\n", count, count > 1 ? 'S' : ' ');
                break;
            }
            return 4;
        
        case 's':
            if (count <= SECONDS) // specifikovany pocet dnu lze vytiskout, je mensi nez velikost denniho bufferu
            {
                selected = data.seconds;
                pos = data.seconds_pos - count + SECONDS; // posun o 'count' sekund zpet v sekundovem bufferu
                modulator = SECONDS;
                // hlavicka pro sekundovy vypis
                sprintf(buffer, "|   TEMPERATURE SAMPLES OVER LAST %d SECOND%c\t|\r\n", count, count > 1 ? 'S' : ' ');
                break;
            }
            return 4;

        default:
            return 4;
    }
    
    // tisk hlavicky tabulky
    uart_print_string("+-----------------------------------------------+\r\n");
    uart_print_string(buffer);
    uart_print_string("+-----------------------------------------------+\r\n");
    for (uint16_t i = 0; i < count; i++) // tisk namerenych hodnot
    {
        if (selected[pos % modulator].time != 0)
        {
            uart_print_measurment(&selected[pos % modulator]);
        }
        pos++;
    }
    uart_print_string("+-----------------------------------------------+\r\n"); // paticka tabulky

    return 0;
}

void print_log_interval(char *buff)
{
    const char* template = "| - Logging with interval of %d %s\r\n";

    if (log_interval == 0) // neprobiha logovani
    {
        uart_print_string("| - Logging is turned off.\t\t\t\t\t\t\t|\r\n");
    }
    else if (log_interval < SEC_IN_MIN || (log_interval < SECONDS && log_interval % SEC_IN_MIN)) // logovani v radu sekundach
    {
        sprintf(buff, template, log_interval, "seconds. \t\t\t\t\t|");
        uart_print_string(buff);
    }
    else if (log_interval < SEC_IN_HOUR) // logovani v radu minut
    {
        sprintf(buff, template, log_interval / SEC_IN_MIN, "minutes. \t\t\t\t\t|");
        uart_print_string(buff);
    }
    else if (log_interval < SEC_IN_DAY) // logovani v radu hodin
    {
        sprintf(buff, template, log_interval / SEC_IN_HOUR, "hours.\t\t\t\t\t\t|");
        uart_print_string(buff);
    }
    else // logovani v radu dni
    {
        sprintf(buff, template, log_interval / SEC_IN_DAY, "days.\t\t\t\t\t\t|");
        uart_print_string(buff);
    }
}

collected_data_t *get_data()
{
    return &data;
}
