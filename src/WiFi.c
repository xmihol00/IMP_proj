
#include "WiFi.h"

static esp_netif_t *get_netif_from_desc(const char *desc);
static void wifi_start();
static void wifi_stop();
static bool is_our_netif(const char *prefix, esp_netif_t *netif);
static void on_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

static bool connected = false;
static char *TAG = "WiFi";
static int s_active_interfaces = 0;
static xSemaphoreHandle s_semph_get_ip_addrs;
static esp_netif_t *s_example_esp_netif = NULL;
esp_ip4_addr_t s_ip_addr;

credentials_t credentials;

static void signalize_wifi_disconnected()
{
    gpio_pad_select_gpio(GPIO_LED_RED);
    gpio_set_direction(GPIO_LED_RED, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LED_RED, 1);
}

static void signalize_wifi_connected()
{
    gpio_set_level(GPIO_LED_RED, 0);
    gpio_set_direction(GPIO_LED_RED, GPIO_MODE_DISABLE);
}

esp_err_t wifi_connect()
{
    signalize_wifi_disconnected();
    s_ip_addr.addr = 0;
    connected = false;

    if (s_semph_get_ip_addrs != NULL) 
    {
        return ESP_ERR_INVALID_STATE;
    }
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    esp_event_loop_create_default();

    wifi_start();
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&wifi_stop));

    for (int i = 0; i < s_active_interfaces; i++) 
    {
        xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY);
    }

    if (s_ip_addr.addr == 0)
    {
        wifi_disconnect();
    }
    
    return ESP_OK;
}

esp_err_t wifi_disconnect()
{
    if (s_semph_get_ip_addrs == NULL) 
    {
        return ESP_ERR_INVALID_STATE;
    }
    vSemaphoreDelete(s_semph_get_ip_addrs);
    s_semph_get_ip_addrs = NULL;
    wifi_stop();
    ESP_ERROR_CHECK(esp_unregister_shutdown_handler(&wifi_stop));

    signalize_wifi_disconnected();
    return ESP_OK;
}

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI("WIFI", "%d", s_ip_addr.addr);
    if (s_ip_addr.addr == 0)
    {
        ESP_LOGI("WIFI", "SEM givup");
        xSemaphoreGive(s_semph_get_ip_addrs);
    }
    else
    {
        signalize_wifi_disconnected();
        connected = false;
        esp_wifi_connect();
    }    
}

static void wifi_start()
{
    char *desc;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    
    asprintf(&desc, "%s: %s", TAG, esp_netif_config.if_desc);
    esp_netif_config.if_desc = desc;
    esp_netif_config.route_prio = 128;
    esp_netif_t *netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    free(desc);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    memcpy(wifi_config.sta.ssid, credentials.name, CREDENTIAL_NAME);
    memcpy(wifi_config.sta.password, credentials.password, CREDENTIAL_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_connect();
    
    s_example_esp_netif = netif;
    s_active_interfaces++;
    s_semph_get_ip_addrs = xSemaphoreCreateCounting(s_active_interfaces, 0);
}

static void wifi_stop()
{
    esp_netif_t *wifi_netif = get_netif_from_desc("sta");
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip));

    esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) 
    {
        return;
    }
    else if (err == ESP_OK)
    {
        connected = false;
    }

    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(wifi_netif));
    esp_netif_destroy(wifi_netif);
    s_example_esp_netif = NULL;
    s_active_interfaces--;
}

static esp_netif_t *get_netif_from_desc(const char *desc)
{
    esp_netif_t *netif = NULL;
    char *expected_desc;
    asprintf(&expected_desc, "%s: %s", TAG, desc);
    while ((netif = esp_netif_next(netif)) != NULL) 
    {
        if (strcmp(esp_netif_get_desc(netif), expected_desc) == 0) 
        {
            free(expected_desc);
            return netif;
        }
    }

    free(expected_desc);
    return netif;
}

static bool is_our_netif(const char *prefix, esp_netif_t *netif)
{
    return strncmp(prefix, esp_netif_get_desc(netif), strlen(prefix) - 1) == 0;
}

static void on_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    if (!is_our_netif(TAG, event->esp_netif)) 
    {
        return;
    }

    memcpy(&s_ip_addr, &event->ip_info.ip, sizeof(s_ip_addr));

    connected = true;
    signalize_wifi_connected();

    xSemaphoreGive(s_semph_get_ip_addrs);
}

bool wifi_is_connected()
{
    return connected;
}
