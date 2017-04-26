#include "wpihardware.hpp"
#include "wiringPiSPI.h"
#include "wiringPi.h"

wPi::wPi()
{
  m_nSpeed = 500000 ;
  m_nDevice = 0 ;
  m_nFD = -1 ;

  // use GPIO numbering (as written on the board and cobbler)
  wiringPiSetupGpio() ;
}

bool wPi::setSpeed(uint32_t speed)
{
  m_nSpeed = speed ;
  return true ;
}

bool wPi::spiopen(uint32_t bus, uint32_t device)
{
  if ((m_nFD=wiringPiSPISetup (device, m_nSpeed)) < 0) return false ;
  m_nDevice = device ;

  return true ;
}

bool wPi::write(uint8_t byte)
{
  uint8_t b = byte ;
  if (wiringPiSPIDataRW (m_nDevice, &b, 1) < 0) return false ;

  return true ;
}

bool wPi::write(uint8_t *bytes, uint32_t len)
{
  if (wiringPiSPIDataRW (m_nDevice, bytes, len) < 0) return false ;

  return false ;
}


////////////////////////////////////////
//
//  Timer interface
void wPi::microSleep(unsigned int nMicroSec)
{
  delayMicroseconds(nMicroSec) ;
}
void wPi::milliSleep(unsigned int nMilliSec)
{
  delay(nMilliSec) ;
}


////////////////////////////////////////
//
//  GPIO Interface


bool wPi::setup(uint32_t pin, enDirection eDir)
{
  pinMode(pin, eDir==gpio_input?INPUT:OUTPUT) ;
  return true ;
}

bool wPi::output(uint32_t pin, enValue eVal)
{
  digitalWrite (pin, eVal==low?0:1) ;
  return true ;
}

IHardwareGPIO::enValue wPi::input(uint32_t pin)
{
  return digitalRead(pin) == 0?low:high;
}

bool wPi::register_interrupt(uint32_t pin, enEdge edge, void(*function)(void))
{
  int edge_type ;

  switch (edge){
  case rising:
    edge_type = INT_EDGE_RISING;
    break;
  case falling:
    edge_type = INT_EDGE_FALLING;
    break ;
  case both:
    edge_type = INT_EDGE_BOTH;
    break;
  default:
    return false ;
  }

  if (wiringPiISR (pin, edge_type, function) < 0)
    return false ;

  return true;
}
