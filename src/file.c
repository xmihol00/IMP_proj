#include "file.h"

extern credentials_t credentials;

static main_page_t main_page =
{
    .data = NULL,
    .size = 0
};

void init_file_system()
{
    esp_vfs_spiffs_conf_t conf = 
    {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    main_page.data = malloc(INDEX_FILE_SIZE);
    if (main_page.data == NULL)
    {
        return;
    }

    FILE *file = fopen("/spiffs/index.html", "r");
    if (file == NULL)
    {
        return;
    }

    main_page.size = fread(main_page.data, 1, INDEX_FILE_SIZE, file);
    fclose(file);

    file = fopen("/spiffs/credentials", "r");
    if (file == NULL)
    {
        return;
    }

    fread(&credentials, 1, CREDENTIAL_SIZE << 1, file);
    fclose(file);
}

main_page_t get_main_page()
{
    return main_page;
}

void store_credentials()
{
    FILE *file = fopen("/spiffs/credentials", "w");
    if (file == NULL)
    {
        return;
    }
    fwrite(&credentials, 1, CREDENTIAL_SIZE << 1, file);
    
    fclose(file);
}
