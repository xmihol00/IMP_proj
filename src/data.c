
#include "data.h"

static uint32_t log_interval = 0;
static collected_data_t data;

static void average_last_minute();
static void average_last_hour();
static void average_last_day();

extern void uart_log_measurment(measurment_t *);
extern void uart_print_measurment(measurment_t *);
extern void uart_print_string(const char *);

esp_err_t init_data()
{
    data.days_pos = 0;
    data.hours_pos = 0;
    data.minutes_pos = 0;
    data.seconds_pos = 0;

    data.days = calloc(DAYS, sizeof(measurment_t));
    data.hours = calloc(HOURS, sizeof(measurment_t));
    data.minutes = calloc(MINUTES, sizeof(measurment_t));
    data.seconds = calloc(SECONDS, sizeof(measurment_t));

    
    return data.days && data.hours && data.minutes && data.seconds ? ESP_OK : ESP_FAIL;
}

void store_measurment(float temperature)
{
    data.seconds[data.seconds_pos].temperature = temperature;
    time(&data.seconds[data.seconds_pos].time);

    if (!(data.seconds[data.seconds_pos].time % SEC_IN_MIN))
    {
        average_last_minute();

        if (!(data.seconds[data.seconds_pos].time % SEC_IN_HOUR))
        {
            average_last_hour();

            if (!(data.seconds[data.seconds_pos].time % SEC_IN_DAY))
            {
                average_last_day();
            }
        }
    }

    if (log_interval && !(data.seconds[data.seconds_pos].time % log_interval))
    {
        uart_log_measurment(&data.seconds[data.seconds_pos]);
    }

    if (++data.seconds_pos == SECONDS)
    {
        data.seconds_pos = 0;
    }
}

static void average_last_minute()
{
    uint16_t pos = data.seconds_pos - SEC_IN_MIN + SECONDS;

    float collected = 0.0;
    uint8_t measurments = 0;
    for (uint8_t i = 0; i < SEC_IN_MIN; i++)
    {
        if (data.seconds[pos % SECONDS].time != 0)
        {
            collected += data.seconds[pos % SECONDS].temperature;
            measurments++;
        }
        pos++;
    }

    data.minutes[data.minutes_pos].temperature = collected / measurments;
    data.minutes[data.minutes_pos].time = data.seconds[data.seconds_pos].time;
    
    if (++data.minutes_pos == MINUTES)
    {
        data.minutes_pos = 0;
    }
}

static void average_last_hour()
{
    uint16_t pos = data.minutes_pos - MINS_IN_HOUR - 1 + MINUTES;

    float collected = 0.0;
    uint8_t measurments = 0;
    for (uint8_t i = 0; i < MINS_IN_HOUR; i++)
    {
        if (data.minutes[pos % MINUTES].time != 0)
        {
            collected += data.minutes[pos % MINUTES].temperature;
            measurments++;
        }
        pos++;
    }

    data.hours[data.hours_pos].temperature = collected / measurments;
    data.hours[data.hours_pos].time = data.minutes[data.minutes_pos - 1].time;
    
    if (++data.hours_pos == HOURS)
    {
        data.hours_pos = 0;
    }
}

static void average_last_day()
{
    uint16_t pos = data.minutes_pos - HOURS_IN_DAY - 1 + HOURS;

    float collected = 0.0;
    uint8_t measurments = 0;
    for (uint8_t i = 0; i < HOURS_IN_DAY; i++)
    {
        if (data.hours[pos % HOURS].time != 0)
        {
            collected += data.hours[pos % HOURS].temperature;
            measurments++;
        }
        pos++;
    }

    data.days[data.days_pos].temperature = collected / measurments;
    data.days[data.days_pos].time = data.hours[data.hours_pos - 1].time;
    
    if (++data.days_pos == DAYS)
    {
        data.days_pos = 0;
    }
}

uint8_t set_log_interval(uint32_t interval, char unit)
{
    if (interval == 0)
    {
        log_interval = 0;
        return 0;
    }

    switch (unit)
    {
        case 'd':
            log_interval = interval * SEC_IN_DAY;
            break;

        case 'h':
            log_interval = interval * SEC_IN_HOUR;
            break;

        case 'm':
            log_interval = interval * SEC_IN_MIN;
            break;
        
        case 's':
            log_interval = interval;
            break;

        default:
            return 4;
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
            if (count <= DAYS)
            {
                selected = data.days;
                pos = data.days_pos - count;
                pos = pos < 0 ? pos + DAYS : pos;
                modulator = DAYS;
                sprintf(buffer, "|    AVERAGE TEMPERATURE OVER LAST %d DAY%c\t|\r\n", count, count > 1 ? 'S' : ' ');
                break;
            }
            return 4;

        case 'h':
            if (count <= HOURS)
            {
                selected = data.hours;
                pos = data.hours_pos - count;
                pos = pos < 0 ? pos + HOURS : pos;
                modulator = HOURS;
                sprintf(buffer, "|    AVERAGE TEMPERATURE OVER LAST %d HOUR%c\t|\r\n", count, count > 1 ? 'S' : ' ');
                break;
            }
            return 4;

        case 'm':
            if (count <= MINUTES)
            {
                selected = data.minutes;
                pos = data.minutes_pos - count;
                pos = pos < 0 ? pos + MINUTES : pos;
                modulator = MINUTES;
                sprintf(buffer, "|   AVERAGE TEMPERATURE OVER LAST %d MINUTE%c\t|\r\n", count, count > 1 ? 'S' : ' ');
                break;
            }
            return 4;
        
        case 's':
            if (count <= SECONDS)
            {
                selected = data.seconds;
                pos = data.seconds_pos - count;
                pos = pos < 0 ? pos + SECONDS : pos;
                modulator = SECONDS;
                sprintf(buffer, "|   TEMPERATURE SAMPLES OVER LAST %d SECOND%c\t|\r\n", count, count > 1 ? 'S' : ' ');
                break;
            }
            return 4;

        default:
            return 4;
    }
    
    uart_print_string("+-----------------------------------------------+\r\n");
    uart_print_string(buffer);
    uart_print_string("+-----------------------------------------------+\r\n");
    for (uint16_t i = 0; i < count; i++)
    {
        if (selected[pos % modulator].time != 0)
        {
            uart_print_measurment(&selected[pos % modulator]);
        }
        pos++;
    }
    uart_print_string("+-----------------------------------------------+\r\n");

    return 0;
}

void print_log_interval(char *buff)
{
    const char* template = "| - Logging with interval of %d %s\r\n";

    if (log_interval == 0)
    {
        uart_print_string("| - Logging is turned off.\t\t\t\t\t\t\t|\r\n");
    }
    else if (log_interval < SEC_IN_MIN || (log_interval < SECONDS && log_interval % SEC_IN_MIN))
    {
        sprintf(buff, template, log_interval, "seconds. \t\t\t\t\t|");
        uart_print_string(buff);
    }
    else if (log_interval < SEC_IN_HOUR)
    {
        sprintf(buff, template, log_interval / SEC_IN_MIN, "minutes. \t\t\t\t\t|");
        uart_print_string(buff);
    }
    else if (log_interval < SEC_IN_DAY)
    {
        sprintf(buff, template, log_interval / SEC_IN_HOUR, "hours.\t\t\t\t\t\t|");
        uart_print_string(buff);
    }
    else
    {
        sprintf(buff, template, log_interval / SEC_IN_DAY, "days.\t\t\t\t\t\t|");
        uart_print_string(buff);
    }
}

collected_data_t *get_data()
{
    return &data;
}
