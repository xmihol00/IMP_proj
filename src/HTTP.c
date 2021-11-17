#include "HTTP.h"

static esp_err_t main_page(httpd_req_t *req)
{
    main_page_t page = get_main_page();
    httpd_resp_send(req, page.data, page.size);
    return ESP_OK;
}

static esp_err_t all_data(httpd_req_t *req)
{
    collected_data_t *data = get_data();
    uint16_t data_pos = data->seconds_pos + 1;
    uint16_t modul = 0;
    
    uint32_t json_pos = 6;
    char *json = calloc(1, JSON_OBJ_LEN * (SECONDS + MINUTES + HOURS + DAYS) + 100);
    if (json == NULL)
    {
        return ESP_FAIL;
    }
    strcpy(json, "{\"s\":[");
    
    uint16_t i = 0;
    while(i++ < SECONDS)
    {
        modul = data_pos % SECONDS;
        if (data->seconds[modul].time != 0)
        {
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.3f}, ", data->seconds[modul].time, data->seconds[modul].temperature);
            json_pos += JSON_OBJ_LEN;
        }
        data_pos++;
    }

    if (json[json_pos - 2] == ',')
    {
        json[json_pos - 2] = ' ';
    }
    json[json_pos - 1] = ']';
    json[json_pos++] = ',';
    strcpy(&json[json_pos], "\"m\":[ ");
    json_pos += 6;

    data_pos = data->minutes_pos + 1;
    i = 0;
    while(i++ < MINUTES)
    {
        modul = data_pos % MINUTES;
        if (data->minutes[modul].time != 0)
        {
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.3f}, ", data->minutes[modul].time, data->minutes[modul].temperature);
            json_pos += JSON_OBJ_LEN;
        }
        data_pos++;
    }

    if (json[json_pos - 2] == ',')
    {
        json[json_pos - 2] = ' ';
    }
    json[json_pos - 1] = ']';
    json[json_pos++] = ',';
    strcpy(&json[json_pos], "\"h\":[ ");
    json_pos += 6;

    data_pos = data->hours_pos + 1;
    i = 0;
    while(i++ < HOURS)
    {
        modul = data_pos % HOURS;
        if (data->hours[modul].time != 0)
        {
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.3f}, ", data->hours[modul].time, data->hours[modul].temperature);
            json_pos += JSON_OBJ_LEN;
        }
        data_pos++;
    }

    if (json[json_pos - 2] == ',')
    {
        json[json_pos - 2] = ' ';
    }
    json[json_pos - 1] = ']';
    json[json_pos++] = ',';
    strcpy(&json[json_pos], "\"d\":[ ");
    json_pos += 6;

    data_pos = data->days_pos + 1;
    i = 0;
    while(i++ < DAYS)
    {
        modul = data_pos % DAYS;
        if (data->days[modul].time != 0)
        {
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.3f}, ", data->days[modul].time, data->days[modul].temperature);
            json_pos += JSON_OBJ_LEN;
        }
        data_pos++;
    }

    if (json[json_pos - 2] == ',')
    {
        json[json_pos - 2] = ' ';
    }
    json[json_pos - 1] = ']';
    json[json_pos++] = '}';

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, json_pos);
    free(json);
    return ESP_OK;
}


static esp_err_t update_n(httpd_req_t *req)
{
    char content[16];

    uint8_t recv_size = MIN(req->content_len, 15);

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) 
    {  
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) 
        {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    content[recv_size] = '\0';

    int update_count = atoi(content);
    if (update_count <= 0 || update_count >= SEC_IN_MIN)
    {
        return ESP_FAIL;
    }

    char *json = calloc(1, JSON_OBJ_LEN * (update_count + 3) + 50);
    if (json == NULL)
    {
        return ESP_FAIL;
    }

    collected_data_t *data = get_data();
    uint16_t data_pos = data->seconds_pos - update_count + SECONDS - 1;
    bool min = false, hour = false, day = false;
    uint16_t json_pos = 6;
    uint16_t modul = 0;

    strcpy(json, "{\"s\":[");
    for (int i = 0; i < update_count; i++)
    {
        modul = data_pos % SECONDS;
        if (data->seconds[modul].time != 0)
        {
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.3f}, ", data->seconds[modul].time, data->seconds[modul].temperature);
            json_pos += JSON_OBJ_LEN;

            if (!(data->seconds[modul].time % SEC_IN_MIN))
            {
                min = true;
                if (!(data->seconds[modul].time % SEC_IN_HOUR))
                {
                    hour = true;
                    if (!(data->seconds[modul].time % SEC_IN_DAY))
                    {
                        day = true;
                    }
                }
            }
        }
        data_pos++;
    }

    if (min)
    {
        if (json[json_pos - 2] == ',')
    {
        json[json_pos - 2] = ' ';
    }
        json[json_pos - 1] = ']';
        json[json_pos++] = ',';
        strcpy(&json[json_pos], "\"m\":[ ");
        json_pos += 6;
        sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.3f}, ", data->minutes[data->minutes_pos - 1].time, 
                                                               data->minutes[data->minutes_pos - 1].temperature);
        json_pos += JSON_OBJ_LEN;

        if (hour)
        {
            if (json[json_pos - 2] == ',')
            {
                json[json_pos - 2] = ' ';
            }
            json[json_pos - 1] = ']';
            json[json_pos++] = ',';
            strcpy(&json[json_pos], "\"h\":[ ");
            json_pos += 6;
            sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.3f}, ", data->hours[data->hours_pos - 1].time, 
                                                                   data->hours[data->hours_pos - 1].temperature);
            json_pos += JSON_OBJ_LEN;

            if (day)
            {
                if (json[json_pos - 2] == ',')
                {
                    json[json_pos - 2] = ' ';
                }
                json[json_pos - 1] = ']';
                json[json_pos++] = ',';
                strcpy(&json[json_pos], "\"d\":[ ");
                json_pos += 6;
                sprintf(&json[json_pos], "{\"x\":%20ld,\"y\":%3.3f}, ", data->days[data->days_pos - 1].time, 
                                                                       data->days[data->days_pos - 1].temperature);
                json_pos += JSON_OBJ_LEN;
            }
        }
    }
    
    if (json[json_pos - 2] == ',')
    {
        json[json_pos - 2] = ' ';
    }
    json[json_pos - 1] = ']';
    json[json_pos++] = '}';
    
    httpd_resp_send(req, json, json_pos);
    return ESP_OK;
}

httpd_uri_t all_data_uri = 
{
    .uri      = "/AllData",
    .method   = HTTP_GET,
    .handler  = all_data,
    .user_ctx = NULL
};

httpd_uri_t main_page_uri = 
{
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = main_page,
    .user_ctx = NULL
};

httpd_uri_t uri_update_n = 
{
    .uri      = "/Update",
    .method   = HTTP_POST,
    .handler  = update_n,
    .user_ctx = NULL
};

/* Function for starting the webserver */
esp_err_t start_webserver()
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) 
    {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &main_page_uri);
        httpd_register_uri_handler(server, &all_data_uri);
        httpd_register_uri_handler(server, &uri_update_n);
    }
    /* If server failed to start, handle will be NULL */
    return server ? ESP_OK : ESP_FAIL;
}
