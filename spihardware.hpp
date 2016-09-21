#ifndef __SPI_HARDWARE_HPP
#define __SPI_HARDWARE_HPP

#include "hardware.hpp"

class spiHw: public IHardwareSPI{
public:
  spiHw();
  ~spiHw() ;
  
  bool spiopen(uint32_t bus, uint32_t device) ;
  bool write(uint8_t byte);
  bool write(uint8_t *bytes, uint32_t len) ;

  // bidirectional xfer call, used by the write calls above
  // only they discard the rxbuf data
  bool xfer(uint8_t *bytes, uint8_t *rxbuf, uint32_t len) ;
  
  bool setSpeed(uint32_t speed);
  bool setMode(uint8_t mode);
  bool set3Wire(bool b3Wire);
  bool setLoop(bool bLoop);
  bool setBPW(uint8_t bits);
  bool setBitOrder(bool bLSB);
  bool setCSHigh(bool bHigh);

  void printState() ;

protected:
  int m_fd ;
  uint8_t m_mode ;
  uint8_t m_bitsperword;
  uint32_t m_maxspeed_hz ;
  uint32_t m_speed_hz ;

private:
  int spidev_set_mode( int fd, uint8_t mode) ;
};




#endif // __SPI_HARDWARE_HPP
