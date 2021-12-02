//=================================================================================================================
// Soubor:      file.c
// Projekt:     VUT, FIT, IMP, Mereni teploty
// Datum:       2. 12. 2021
// Autor:       David Mihola
// Kontakt:     xmihol00@stud.fit.vutbr.cz
// Popis:       Funkce pro spravu souboroveho systemu
//=================================================================================================================

#include "file.h"

extern credentials_t credentials; // prihlasovaci udaje k WiFi

static main_page_t main_page =
{
    .data = NULL,
    .size = 0
}; // struktura, do ktere bude nahrana hlavni stranka http serveru

void init_file_system()
{
    esp_vfs_spiffs_conf_t conf = 
    {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    }; 

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf)); // konfigurace souboroveho systemu

    main_page.data = malloc(INDEX_FILE_SIZE);
    if (main_page.data == NULL)
    {
        return; // nedostatek pameti pro HTML stranku
    }

    FILE *file = fopen("/spiffs/index.html", "r");
    if (file == NULL)
    {
        return; // hlavni HTML stranku se nezdarilo otevrit
    }

    main_page.size = fread(main_page.data, 1, INDEX_FILE_SIZE, file); // nacteni HTML stranky
    fclose(file);

    // predvypleni udaju pro WiFi
    credentials.name[0] = '*';
    credentials.name[1] = 0;
    credentials.password[0] = '*';
    credentials.password[1] = 0;

    file = fopen("/spiffs/credentials", "r");
    if (file == NULL)
    {
        return; // soubor s udaji se nezdarilo otevrit
    }

    fread(&credentials, 1, CREDENTIAL_SIZE, file); // nacteni souboru s udaji
    // v pripade, ze udaje nejsou vyplneny je nutne aby tyto retezce nebyly prazdne, jinak dochazi k chybe pri pripojeni k wifi
    if (credentials.name[0] == 0)
    {
         credentials.name[0] = '*';
         credentials.name[1] = 0;
    }
    if (credentials.password[0] == 0)
    {
         credentials.password[0] = '*';
         credentials.password[1] = 0;
    }

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
        return; // soubor s udaji se nezdarilo otevrit
    }

    // v pripade, ze udaje nejsou vyplneny je nutne aby tyto retezce nebyly prazdne, jinak dochazi k chybe pri pripojeni k wifi
    if (credentials.name[0] == 0)
    {
         credentials.name[0] = '*';
         credentials.name[1] = 0;
    }
    if (credentials.password[0] == 0)
    {
         credentials.password[0] = '*';
         credentials.password[1] = 0;
    }

    fwrite(&credentials, 1, CREDENTIAL_SIZE, file); // zapsani udaju
    
    fclose(file);
}
