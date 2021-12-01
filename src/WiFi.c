
#include "WiFi.h"

/**
 * @brief Funkce pro spusteni wifi.
 **/
static void wifi_start();

/**
 * @brief Funkce pro zastaveni wifi.
 **/
static void wifi_stop();

/**
 * @brief Callback volany, jakmile je obdrzena IP adresa a zarizeni je tak pripojeno.
 **/
static void on_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

/**
 * @brief Callback volany pri nahlem odpojeni od wifi
 **/
static void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

static bool connected = false;                         // true, pokud je wifi pripojena, jinak false 
static xSemaphoreHandle s_semph_get_ip_addrs;          // semafor pro pockani na pripojeni zajisteni pripojeni
esp_ip4_addr_t s_ip_addr;                              // zde je ulozena IP adresa po pripojeni  

credentials_t credentials;

static void signalize_wifi_disconnected()
{
    // rozsviceni LED diody
    gpio_pad_select_gpio(GPIO_LED_RED);
    gpio_set_direction(GPIO_LED_RED, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_LED_RED, 1);
}

static void signalize_wifi_connected()
{
    // zhasnuti LED diody
    gpio_set_level(GPIO_LED_RED, 0);
    gpio_set_direction(GPIO_LED_RED, GPIO_MODE_DISABLE);
}

void wifi_connect()
{
    signalize_wifi_disconnected(); // zignalizace LED diodou, ze wifi neni pripojena
    s_ip_addr.addr = 0;
    connected = false;

    if (s_semph_get_ip_addrs != NULL) 
    {
        return;
    }

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    esp_event_loop_create_default(); // vytvoreni obsluzne rutiny pro zpracovani WiFi udalosti

    wifi_start();
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&wifi_stop)); // registrace handleru pro vypnuti wifi

    xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY); // cekani na dokonceni pripojeni

    if (s_ip_addr.addr == 0)
    {
        wifi_disconnect(); // pripojeni se nezdarilo, wifi je vhodne odpojit
    }
}

void wifi_disconnect()
{
    if (s_semph_get_ip_addrs == NULL) 
    {
        return;
    }

    vSemaphoreDelete(s_semph_get_ip_addrs);
    s_semph_get_ip_addrs = NULL;
    wifi_stop(); // zastaveni wifi
    ESP_ERROR_CHECK(esp_unregister_shutdown_handler(&wifi_stop)); // zruseni handleru pro vypnuti

    signalize_wifi_disconnected(); // rozsviceni diody
}

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (s_ip_addr.addr == 0) // wifi nebyla pripojena
    {
        xSemaphoreGive(s_semph_get_ip_addrs); // uvolneni semaforu a pokracovani bez pripojeni
    }
    else
    {
        signalize_wifi_disconnected(); // zignalizace LED diodou, ze doslo k odpojeni
        connected = false; // wifi neni pripojena
        esp_wifi_connect(); // pokud o opetovne pripojeni
    }    
}

static void wifi_start()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // ziskani zakladni konfigurace
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // inicializace wifi

    esp_wifi_set_default_wifi_sta_handlers();

    // registrace hanfdleru pro nahle odpojeni a pro pripojeni (ziskani IP adresy)
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    // vyplneni prihlasovacich udaju
    memcpy(wifi_config.sta.ssid, credentials.name, CREDENTIAL_NAME); 
    memcpy(wifi_config.sta.password, credentials.password, CREDENTIAL_PASS);

    // nastaveni a spusteni wifi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_connect();
    
    s_semph_get_ip_addrs = xSemaphoreCreateCounting(1, 0);
}

static void wifi_stop()
{
    esp_netif_t *wifi_netif = esp_netif_next(NULL);
    // odebrani handleru pro odpojeni a ziskani IP
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip));

    esp_err_t err = esp_wifi_stop(); // zastaveni wifi
    if (err == ESP_ERR_WIFI_NOT_INIT) 
    {
        return;
    }
    else if (err == ESP_OK)
    {
        connected = false;
    }

    ESP_ERROR_CHECK(err);
    // deinicializace wifi
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(wifi_netif));
    esp_netif_destroy(wifi_netif);
}

static void on_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    memcpy(&s_ip_addr, &event->ip_info.ip, sizeof(s_ip_addr)); // ulozeni IP adresy

    connected = true;               // zaznamenani uspesneho pripojeni
    signalize_wifi_connected();     // signalizace pripojeni k wifi

    xSemaphoreGive(s_semph_get_ip_addrs); // uvolneni behu programu, pripojeni vytvoreno
}

bool wifi_is_connected()
{
    return connected;
}
