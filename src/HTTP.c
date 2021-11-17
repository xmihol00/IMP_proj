#include "HTTP.h"


/* Our URI handler function to be called during GET /uri request */
static esp_err_t get_handler(httpd_req_t *req)
{
    collected_data_t *data = get_data();
    uint16_t data_pos = data->seconds_pos + 1;
    uint16_t modul = 0;
    
    uint32_t json_pos = 4;
    char *json = calloc(1, 34 * (SECONDS + MINUTES) + 100);
    if (json == NULL)
    {
        return ESP_FAIL;
    }
    strcpy(json, "{s:[");
    
    uint16_t i = 0;
    while(i++ < SECONDS)
    {
        modul = data_pos % SECONDS;
        if (data->seconds[modul].time != 0)
        {
            sprintf(&json[json_pos], "{x:%20ld,y:%3.3f},", data->seconds[modul].time, data->seconds[modul].temperature);
            json_pos += 34;
        }

        data_pos++;
    }

    json[json_pos - 1] = ']';
    json[json_pos++] = ',';
    strcpy(&json[json_pos], "m:[ ");
    json_pos += 4;

    data_pos = data->minutes_pos + 1;
    i = 0;
    while(i++ < MINUTES)
    {
        modul = data_pos % MINUTES;
        if (data->minutes[modul].time != 0)
        {
            sprintf(&json[json_pos], "{x:%20ld,y:%3.3f},", data->minutes[modul].time, data->minutes[modul].temperature);
            json_pos += 34;
        }
        data_pos++;
    }

    json[json_pos - 1] = ']';
    json[json_pos++] = ',';
    strcpy(&json[json_pos], "h:[ ");
    json_pos += 4;

    data_pos = data->hours_pos + 1;
    i = 0;
    while(i++ < HOURS)
    {
        modul = data_pos % HOURS;
        if (data->hours[modul].time != 0)
        {
            sprintf(&json[json_pos], "{x:%20ld,y:%3.3f},", data->hours[modul].time, data->hours[modul].temperature);
            json_pos += 34;
        }
        data_pos++;
    }

    json[json_pos - 1] = ']';
    json[json_pos++] = ',';
    strcpy(&json[json_pos], "d:[ ");
    json_pos += 4;

    data_pos = data->days_pos + 1;
    i = 0;
    while(i++ < DAYS)
    {
        modul = data_pos % DAYS;
        if (data->days[modul].time != 0)
        {
            sprintf(&json[json_pos], "{x:%20ld,y:%3.3f},", data->days[modul].time, data->days[modul].temperature);
            json_pos += 34;
        }
        data_pos++;
    }

    json[json_pos - 1] = ']';
    json[json_pos] = '}';

    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* Our URI handler function to be called during POST /uri request */
static esp_err_t post_handler(httpd_req_t *req)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char content[100];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }

    /* Send a simple response */
    const char resp[] = "URI POST Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

/* URI handler structure for POST /uri */
httpd_uri_t uri_post = {
    .uri      = "/",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};

/* Function for starting the webserver */
httpd_handle_t start_webserver(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
static void stop_webserver(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}

