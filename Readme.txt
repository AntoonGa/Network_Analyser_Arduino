This repo contains code to create a network analyser using 1 Arduino, 1 1024K SRAM chips and 1 ADC chips (two versions). Schematics and user story also included as Word format.

I originally did this pet project to replace a broken Networtk Analyser in my lab at UC Berkeley (cost ~10k usd...). My alternative offered okay performances for our needs at the time (~16bits @ 100kHz) for about 50 usd. Also taught me SPI communication protocol and some basic C++ :).

Both chips offer similar performances at identical cost: Precision and sampling rate are similar.

Each chips-related folder comes with the Arduino compilable code + a matlab script to analyse text data (not required).
Soldering schematics are included in the user story.
Pin configuraton is chip dependent and is indicated in the Arduino codes (bottom comments).

Datasheets of interesting alternative ADC chips are included.
Far better performance can be obtained by using a higher-end microcontroller. 
