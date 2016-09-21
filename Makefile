CXX = g++
CXXFLAGS=-Wall $(shell freetype-config --cflags)
LIBS = -lwiringPi -ljpeg
LDFLAGS = 

SRCS_LIB = hardware.cpp wpihardware.cpp spihardware.cpp sdd1306oled.cpp pcf8833lcd.cpp displayimage.cpp
H_LIB = $(SRCS_LIB:.cpp=.hpp)
OBJS_LIB = $(SRCS_LIB:.cpp=.o)

SRCS_NOK = nokia6100.cpp
OBJS_NOK = $(SRCS_NOK:.cpp=.o)

SRCS_OLED = main.cpp
OBJS_OLED = $(SRCS_OLED:.cpp=.o)

SRCS_XBMUTIL = xbm2bin.cpp
OBJS_XBMUTIL = $(SRCS_XBMUTIL:.cpp=.o)

SRCS_PSFUTIL = psf2oled.cpp
OBJS_PSFUTIL = $(SRCS_PSFUTIL:.cpp=.o)

EXECUTABLE = oledrun
NOKTST = nokia6100
XBMUTIL = xbm2bin
PSFUTIL = pcf2oled
ARCHIVE = libpihw.a

.PHONY: all
all: $(EXECUTABLE) $(ARCHIVE) $(XBMUTIL) $(PSFUTIL) $(NOKTST)

$(EXECUTABLE): $(OBJS_LIB) $(OBJS_OLED)
	$(CXX) $(LDFLAGS) $(OBJS_LIB) $(OBJS_OLED) $(LIBS) -o $@

$(NOKTST): $(OBJS_LIB) $(OBJS_NOK)
	$(CXX) $(LDFLAGS) $(OBJS_LIB) $(OBJS_NOK) $(LIBS) -o $@

$(XBMUTIL): $(OBJS_XBMUTIL)
	$(CXX) $(OBJS_XBMUTIL) -o $@

$(PSFUTIL): $(OBJS_PSFUTIL)
	$(CXX) $(OBJS_PSFUTIL) $(shell freetype-config --libs) -o $@

$(ARCHIVE): $(OBJS_LIB)
	ar r $@ $?

$(OBJS_LIB): $(H_LIB)

.PHONY: clean
clean:
	rm -f *.o $(EXECUTABLE) $(ARCHIVE) $(XBMUTIL) $(PSFUTIL) $(NOKTST)
