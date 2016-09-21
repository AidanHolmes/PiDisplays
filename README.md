# PiDisplays

This is a base hardware set of libraries and functions to support various displays.
The idea is to have a set of image manipulation functions to support the interfaces without the requirement for too many support libraries.
For instance the Freetype libraries are only required for font conversion.
 
Display.hpp contains classes used by the display hardware. This is very basic and at the moment supports loading JPG files. 

Main dependencies
* libjpeg
* libwiringPi

pcf2oled:-
This supports conversion of basic pcf.gz to a binary format used for the library. Direct reading of fonts is not included

xbm2bin:-
Converts XBM files to a binary format used in the display libraries.


Current displays supported are
* Nokia 6100 using PCF8833 - Arduino shield display. Credit to James P. Lynch on his work.
* OLED display using SDD1306 - Sparkfun 64x48 display

The library has interfaces for SPI, GPIO and timers. All are defined in hardware.hpp
These form a fundamental set of interfaces used for the displays. 
Interface implementations come from 
* spihardware.hpp - basically a copy of code from the very good SPIDEV Python library by Stephen Caudle (https://github.com/doceme/py-spidev)
* wpihardware.hpp - WiringPi library wrapper from Gordon Henderson (http://wiringpi.com/)

2 examples are built
* oledrun - test the OLED display, check config in the test file main.cpp
* nokia6100 - test the PCF8833 output with some examples. Check nokia6100.cpp for config
