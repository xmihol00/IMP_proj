#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_spiffs.h"

esp_err_t init_file_system();

#endif