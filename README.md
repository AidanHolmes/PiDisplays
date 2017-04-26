# PiDisplays

This is a base hardware set of libraries and functions to support various displays.
The idea is to have a set of image manipulation functions to support the interfaces without the requirement for too many support libraries.
 
The hardware libraries use the custom graphicslib library libdisp.
This can be found and cloned from github.com
> git clone https://github.com/AidanHolmes/graphicslib

The default location for this is in a sibling directory to the hardware lib and searches in:
../graphicslib

Main dependencies
* libjpeg
* libwiringPi
* libfreetype2
* libdisp (custom lib)

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
