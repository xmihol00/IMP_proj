# Projekt:     VUT, FIT, IMP, Mereni teploty
### Datum:       5. 12. 2021
### Autor:       David Mihola
### Kontakt:     xmihol00@stud.fit.vutbr.cz

## Soubory projektu
```
--|
  |-- data/                       Adresar obsahujici minifikovany soubor index.html pro nahrani do flash pameti modulu ESP.
  |          
  |-- doc/                        Adresar obsahujici soubory s dokumentaci v binarni i zdrojove podobe.
  |
  |-- src/                        Adresar obsahujici hlavickove a zdrojove soubory a formatovany soubor index.html.
  |
  |-- README.md                   Tento soubor.
  |
  |-- partitions_singleapp.csv    Soubor obsahujici konfiguraci flash pameti modulu ESP.
```

## Preklad
Preklad a nahrani do souboru ESP 32 probihalo pomoci programu *Visual Studio Code* za pouziti rozsireni *PlatformIO*.
V konfiguracnim souboru *platformio.ini* byly pouzity nasledujici hodnoty:
```
    [env:esp32dev]
    platform = espressif32
    board = esp32dev
    framework = espidf
    monitor_speed = 115200
    monitor_flags = --raw 
```

## Dokumentace
Zdrojová forma dokumentace obsahuje pouze soubor *main.tex* z důvodu příliš velké velikosti obrázku. Ke všem zdrojovým souborům lze přistoupit pod tímto odkazem https://www.overleaf.com/read/pqgxkbybcmzs.
