# Projekt:     VUT, FIT, IMP, Mereni teploty
# Datum:       2. 12. 2021
# Autor:       David Mihola
# Kontakt:     xmihol00@stud.fit.vutbr.cz

# Soubory projektu
--|
  |-- data/      Adresar obsahujici minifikovany soubor index.html pro nahrani do ESP.
  |          
  |-- doc/       Adresar obsahujici soubory s dokumentaci v binarni i zdrojove podobe.
  |
  |-- src/       Adresar obsahujici hlavickove a zdrojove soubory, formatovany soubor index.html a Makefile.
  |
  |-- README.md  Tento soubor.
  |

Preklad a nahrani do souboru ESP 32 probihalo pomoci programu *Visual Studio Code* za pouziti rozsireni *PlatformIO*.
V konfiguracnim souboru *platformio.ini* byly pouzity nasledujici hodnoty:
    [env:esp32dev]
    platform = espressif32
    board = esp32dev
    framework = espidf
    monitor_speed = 115200
    monitor_flags = --raw 
