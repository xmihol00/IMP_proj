#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_spiffs.h"

#include "WiFi.h"

#define INDEX_FILE_SIZE 10500

typedef struct 
{
    char *data;
    uint16_t size;
} main_page_t;


void init_file_system();

main_page_t get_main_page();

#endif