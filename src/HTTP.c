#include "HTTP.h"

/**
 * @brief HTTP GET koncovy bod pro hlavni HTML stranku. 
 **/
static esp_err_t main_page(httpd_req_t *req)
{
    main_page_t page = get_main_page();
    httpd_resp_send(req, page.data, page.size); // response
    return ESP_OK;
}

/**
 * @brief HTTP GET koncovy bod pro ziskani vsech namerenych dat.
 **/
static esp_err_t all_data(httpd_req_t *req)
{
    collected_data_t *data = get_data();  // ziskani vsech namerenych dat
    uint16_t data_pos = data->seconds_pos;
    uint16_t modul = 0;
    
    uint32_t json_pos = 6;
    char *json = calloc(1, JSON_OBJ_LEN * (SECONDS + MINUTES + HOURS + DAYS) + 100);
    if (json == NULL)
    {
        httpd_resp_send_500(req);
        return ESP_OK;
    }
    strcpy(json, "{\"s\":["); // zapocati hlavniho JSON objektu a JSON array
    
    uint16_t i = 0;
    while(i++ < SECONDS) // serializace sekundoveho bufferu
    {
        modul = data_pos % SECONDS;
        if (data->seconds[modul].time != 0)
        {
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.2f}   ", data->seconds[modul].time, data->seconds[modul].temperature);
            json_pos += JSON_OBJ_LEN;
            json[json_pos - 1] = ',';
        }
        data_pos++;
    }

    json[json_pos - 1] = ']'; // ukonceni a zapocani JSON array
    json[json_pos++] = ',';
    strcpy(&json[json_pos], "\"m\":[ ");
    json_pos += 6;

    data_pos = data->minutes_pos;
    i = 0;
    while(i++ < MINUTES) // seriaizace minutoveho bufferu
    {
        modul = data_pos % MINUTES;
        if (data->minutes[modul].time != 0)
        {
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.2f}   ", data->minutes[modul].time, data->minutes[modul].temperature);
            json_pos += JSON_OBJ_LEN;
            json[json_pos - 1] = ',';
        }
        data_pos++;
    }

    json[json_pos - 1] = ']'; // ukonceni a zapocani JSON array
    json[json_pos++] = ',';
    strcpy(&json[json_pos], "\"h\":[ ");
    json_pos += 6;

    data_pos = data->hours_pos;
    i = 0;
    while(i++ < HOURS) // serilizace hodinoveho bufferu
    {
        modul = data_pos % HOURS;
        if (data->hours[modul].time != 0)
        {
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.2f}   ", data->hours[modul].time, data->hours[modul].temperature); // JSON objekt
            json_pos += JSON_OBJ_LEN;
            json[json_pos - 1] = ',';
        }
        data_pos++;
    }

    json[json_pos - 1] = ']';   // ukonceni a zapocani JSON array
    json[json_pos++] = ',';
    strcpy(&json[json_pos], "\"d\":[ ");
    json_pos += 6;

    data_pos = data->days_pos;
    i = 0;
    while(i++ < DAYS) // serializace denniho bufferu
    {
        modul = data_pos % DAYS;
        if (data->days[modul].time != 0)
        {
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.2f}   ", data->days[modul].time, data->days[modul].temperature);
            json_pos += JSON_OBJ_LEN;
            json[json_pos - 1] = ',';
        }
        data_pos++;
    }

    json[json_pos - 1] = ']'; // ukonceni JSON array
    json[json_pos++] = '}'; // ukonceni hlavniho JSON objektu

    httpd_resp_set_type(req, "application/json"); // nastaveni spravne Content-Type
    httpd_resp_send(req, json, json_pos); // response
    free(json);
    return ESP_OK;
}

/**
 * @brief HTTP POST koncovy bod pro ziskani N poslednich nsmerenych dat.
 **/
static esp_err_t update_n(httpd_req_t *req)
{
    char content[16];

    uint8_t recv_size = MIN(req->content_len, 15); // ziskani max velikosti obsahu, ktera bude prectena

    int ret = httpd_req_recv(req, content, recv_size); // precteni obsahu dotazu
    if (ret <= 0) // cteni se nepodarilo
    {  
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) 
        {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    content[recv_size] = '\0';

    int update_count = atoi(content); // ziskani poctu vzorku, ktere maji byt odeslany
    if (update_count <= 0 || update_count >= SEC_IN_MIN) // pocet vzorku je mimo meze
    {
        httpd_resp_send_404(req);
        return ESP_OK;
    }

    char *json = calloc(1, JSON_OBJ_LEN * (update_count + 3) + 50);
    if (json == NULL)
    {
        httpd_resp_send_500(req);
        return ESP_OK;
    }

    collected_data_t *data = get_data();
    uint16_t data_pos = data->seconds_pos - update_count + SECONDS - 1; // posunuti zpet v sekundovem bufferu o pozadovany pocet vzorku
    bool min = false, hour = false, day = false;
    uint16_t json_pos = 6;
    uint16_t modul = 0;

    strcpy(json, "{\"s\":["); // zacatek JSON obsahu
    for (int i = 0; i < update_count; i++)
    {
        modul = data_pos % SECONDS;
        if (data->seconds[modul].time != 0) // serilaizace poctu vzorku ze sekundoveho bufferu
        {
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.2f}   ", data->seconds[modul].time, data->seconds[modul].temperature);
            json_pos += JSON_OBJ_LEN;
            json[json_pos - 1] = ',';

            if (!(data->seconds[modul].time % SEC_IN_MIN)) // v danem poctu vzorku doslo k dovrseni minuty
            {
                min = true;
                if (!(data->seconds[modul].time % SEC_IN_HOUR)) // v danem poctu vzorku doslo k dovrseni hodiny
                {
                    hour = true;
                    if (!((data->seconds[modul].time + SEC_IN_HOUR) % SEC_IN_DAY)) // v danem poctu vzorku doslo k dovrseni dne
                    {
                        day = true;
                    }
                }
            }
        }
        data_pos++;
    }

    if (min) // doslo k dovrseni minuty
    {
        json[json_pos - 1] = ']';
        json[json_pos++] = ',';
        strcpy(&json[json_pos], "\"m\":[ ");
        json_pos += 6;
        sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.2f}   ", data->minutes[data->minutes_pos - 1].time, 
                                                               data->minutes[data->minutes_pos - 1].temperature); // serializace dovrsene min
        json_pos += JSON_OBJ_LEN;
        json[json_pos - 1] = ',';

        if (hour) // doslo k dovrseni hodiny
        {
            if (json[json_pos - 2] == ',')
            {
                json[json_pos - 2] = ' ';
            }
            json[json_pos - 1] = ']';
            json[json_pos++] = ',';
            strcpy(&json[json_pos], "\"h\":[ ");
            json_pos += 6;
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.2f}   ", data->hours[data->hours_pos - 1].time, 
                                                                   data->hours[data->hours_pos - 1].temperature); // serializace dovrsene hod
            json_pos += JSON_OBJ_LEN;
            json[json_pos - 1] = ',';

            if (day) // doslo k dovrseni dne
            {
                if (json[json_pos - 2] == ',')
                {
                    json[json_pos - 2] = ' ';
                }
                json[json_pos - 1] = ']';
                json[json_pos++] = ',';
                strcpy(&json[json_pos], "\"d\":[ ");
                json_pos += 6;
                sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.2f}   ", data->days[data->days_pos - 1].time, 
                                                                       data->days[data->days_pos - 1].temperature); // serilizace dne
                json_pos += JSON_OBJ_LEN;
                json[json_pos - 1] = ',';
            }
        }
    }
    
    json[json_pos - 1] = ']';
    json[json_pos++] = '}';
    
    httpd_resp_send(req, json, json_pos); // odpoved
    return ESP_OK;
}

/**
 * @brief Nastaveni koncoveho bodu pro zisk dat.
 **/
httpd_uri_t all_data_uri = 
{
    .uri      = "/AllData",
    .method   = HTTP_GET,
    .handler  = all_data,
    .user_ctx = NULL
};

/**
 * @brief Nastaveni koncoveho bodu pro zisk hlavni stranky.
 **/
httpd_uri_t main_page_uri = 
{
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = main_page,
    .user_ctx = NULL
};

/**
 * @brief Nastaveni koncoveho bodu pro N poslednich namerenych vzorku.
 **/
httpd_uri_t uri_update_n = 
{
    .uri      = "/Update",
    .method   = HTTP_POST,
    .handler  = update_n,
    .user_ctx = NULL
};

esp_err_t start_webserver()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) // spusteni serveru
    {
        // registrace koncovych bodu
        httpd_register_uri_handler(server, &main_page_uri);
        httpd_register_uri_handler(server, &all_data_uri);
        httpd_register_uri_handler(server, &uri_update_n);
    }

    return server ? ESP_OK : ESP_FAIL;
}
