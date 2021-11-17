
#include "file.h"

esp_err_t init_file_system()
{
    esp_vfs_spiffs_conf_t conf = 
    {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    return esp_vfs_spiffs_register(&conf);
}
