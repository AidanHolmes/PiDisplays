#ifndef __WPI_HPP
#define __WPI_HPP

#include "hardware.hpp"

class wPi: public IHardwareSPI, public IHardwareTimer, public IHardwareGPIO{
public:
  wPi() ;

  bool spiopen(uint32_t port, uint32_t device) ;

  bool setSpeed(uint32_t speed) ;

  bool write(uint8_t byte) ;

  bool write(uint8_t *bytes, uint32_t len) ;

  bool read(uint8_t *bytes, uint32_t len){return false;} ; // Not implemented

  // Unsupported interface methods
  bool setBitOrder(bool bLSB){return false;}
  bool setCSHigh(bool bHigh){return false;}
  bool setMode(uint8_t mode){return false;}
  bool set3Wire(bool b3Wire){return false;}
  bool setLoop(bool bLoop){return false;}
  bool setBPW(uint8_t bits){return false;}


  // IHardwareTimer implementation
  void microSleep(unsigned int nMicroSec);
  void milliSleep(unsigned int nMilliSec);

  // IHardwareGPIO implementation

  // Don't care what numbering used. Can be chip or abstraction.
  // This is where the user will need to code directly for the hardware
  bool setup(uint32_t pin, enDirection eDir);

  bool output(uint32_t pin, enValue eVal);

  enValue input(uint32_t pin);

  bool register_interrupt(uint32_t pin, enEdge edge, void(*function)(void));
  
private:
  uint32_t m_nSpeed ;
  uint32_t m_nDevice ;
  int m_nFD ;
};


#endif // __WPI_HPP

