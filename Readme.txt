This repo contains code to create a network analyser using 1 Arduino, 1 1024K SRAM chips and 1 ADC chips (two versions). Schematics and user story also included as Word format.

I originally did this pet project to replace a broken Networtk Analyser at my lab UC Berkeley lab (cost ~10k usd...). My alternative offered okay performances for our needs at the time (~16bits @ 100kHz) for about 50 usd. Also taught me SPI communication protocol and some basic C++ :).

Both chips offer similar performances at identical cost: Precision and sampling rate are similar.

Each chips-related folder comes with the Arduino compilable code + a matlab script to analyse text data (not required).

Datasheets of interesting alternative ADC chips are included.
Far better performance can be obtained by using a higher-end microcontroller. The code should work.