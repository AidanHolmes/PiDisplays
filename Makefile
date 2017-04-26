LIBDISPDIR=../graphicslib
CXX = g++
CXXFLAGS= -Wall -I$(LIBDISPDIR)
LIBS = -lwiringPi -ldisp -ljpeg
LDFLAGS = -L$(LIBDISPDIR)

SRCS_LIB = hardware.cpp wpihardware.cpp spihardware.cpp sdd1306oled.cpp pcf8833lcd.cpp
H_LIB = $(SRCS_LIB:.cpp=.hpp)
OBJS_LIB = $(SRCS_LIB:.cpp=.o)

SRCS_NOK = nokia6100.cpp
OBJS_NOK = $(SRCS_NOK:.cpp=.o)

SRCS_OLED = main.cpp
OBJS_OLED = $(SRCS_OLED:.cpp=.o)

EXECUTABLE = oledrun
NOKTST = nokia6100
ARCHIVE = libpihw.a

.PHONY: all
all: $(EXECUTABLE) $(ARCHIVE) $(NOKTST)

$(EXECUTABLE): $(OBJS_LIB) $(OBJS_OLED) libdisp
	$(CXX) $(LDFLAGS) $(OBJS_LIB) $(OBJS_OLED) $(LIBS) -o $@

$(NOKTST): $(OBJS_LIB) $(OBJS_NOK) libdisp
	$(CXX) $(LDFLAGS) $(OBJS_LIB) $(OBJS_NOK) $(LIBS) -o $@

$(ARCHIVE): $(OBJS_LIB)
	ar r $@ $?

$(OBJS_LIB): $(H_LIB)

.PHONY: libdisp
libdisp:
	$(MAKE) -C $(LIBDISPDIR)

.PHONY: clean
clean:
	rm -f *.o $(EXECUTABLE) $(ARCHIVE) $(NOKTST)
