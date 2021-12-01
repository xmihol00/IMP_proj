#ifndef __HTTP_H__
#define __HTTP_H__

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include <stdio.h>

#include "data.h"
#include "file.h"

#define JSON_OBJ_LEN 38         // pocet znaku jednoho JSON objektu nesouci udaje o namerene hodnote teploty

/**
 * @brief Spusti web server.
 **/
void start_webserver();

#endif
