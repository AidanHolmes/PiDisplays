#ifndef __ARDUINO_HARDWARE_HPP
#define __ARDUINO_HARDWARE_HPP
#include <Arduino.h>
#include <SPI.h>
#include "hardware.hpp"

class ArduinoSpiHw: public IHardwareSPI{
public:
  ArduinoSpiHw();
  ~ArduinoSpiHw() ;
  
  bool spiopen(uint32_t bus, uint32_t device) ;
  void spiclose() ;
  bool write(uint8_t byte);
  bool write(uint8_t *bytes, uint32_t len) ;
  bool read(uint8_t *bytes, uint32_t len) ;
  
  bool setSpeed(uint32_t speed);
  bool setMode(uint8_t mode);
  bool set3Wire(bool b3Wire);
  bool setLoop(bool bLoop);
  bool setBPW(uint8_t bits);
  bool setBitOrder(bool bLSB);
  bool setCSHigh(bool bHigh);

protected:
  uint8_t *m_rxbuffer ;
  uint32_t m_size_buffer ;
  uint8_t m_mode ;
  uint8_t m_bitsperword;
  uint32_t m_speed_hz ;
  bool m_msb ;
  bool m_cshigh;
  uint32_t m_cs ;
  SPISettings *m_psettings;

private:
  bool updateSettings();
};

class ArduinoGPIO: public IHardwareGPIO{
public:

  bool setup(uint32_t pin, enDirection eDir) ;

  bool output(uint32_t pin, enValue eVal) ;

  IHardwareGPIO::enValue input(uint32_t pin) ;

  bool register_interrupt(uint32_t pin, enEdge edge, void(*function)(void)) ;
};

class ArduinoTimer: public IHardwareTimer{
public:
  // Simple interface exposing micro and millisecond sleep timers
  void microSleep(unsigned int nMicroSec) ;
  void milliSleep(unsigned int nMilliSec) ;
};

#endif // __ARDUINO_HARDWARE_HPP
