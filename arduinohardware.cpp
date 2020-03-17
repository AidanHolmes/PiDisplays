#include "arduinohardware.hpp"

ArduinoSpiHw::ArduinoSpiHw()
{
  m_mode = SPI_MODE0; // Arduino modes required
  m_msb = true ;
  m_cshigh = false ; // Default behaviour is to select slave on low CS
  m_bitsperword = 8; // Doesn't actually work in interface library as byte is assumed
  m_speed_hz = 0;
  m_rxbuffer = NULL ;
  m_size_buffer = 0;
  m_cs = 0 ;
  m_psettings = NULL; 
  
}

ArduinoSpiHw::~ArduinoSpiHw()
{
  spiclose() ;
  if (m_psettings) delete m_psettings;
  m_psettings = NULL ;
}

void ArduinoSpiHw::spiclose()
{
  if (m_rxbuffer) delete[] m_rxbuffer ;
  m_size_buffer = 0;
  SPI.end() ;
}

bool ArduinoSpiHw::spiopen(uint32_t bus, uint32_t device)
{
  uint8_t tmp8 ;
  uint32_t tmp32 ;
  
  // bus is ignored as Arduino assumed to run using just one bus (simplification)
  // device is the CS pin to use
  SPI.begin() ;
   
  m_cs = device ;
  if (m_psettings) delete m_psettings ;
  m_psettings = new SPISettings(m_speed_hz, m_msb?MSBFIRST:LSBFIRST, m_mode) ;
  if (!m_psettings) return false ;    
  
  pinMode(m_cs, OUTPUT) ;
  digitalWrite(m_cs, m_cshigh?LOW:HIGH) ; // Set CS pin to inactive
  
  return true ;
}

bool ArduinoSpiHw::write(uint8_t byte)
{
  return write(&byte, 1) ;
}

bool ArduinoSpiHw::write(uint8_t *bytes, uint32_t len)
{
  // Check if SPI has been opened
  if (m_cs == 0 || !m_psettings) return false ;
  
  if (m_size_buffer < len){
    if (m_rxbuffer) delete[] m_rxbuffer ;
    m_rxbuffer = new uint8_t[len];
    if (!m_rxbuffer){
      m_size_buffer = 0 ;
      return false ;
    }
    m_size_buffer = len ;
  }
  memcpy(m_rxbuffer, bytes, len) ;
  
  SPI.beginTransaction(*m_psettings);
  digitalWrite(m_cs, m_cshigh?HIGH:LOW) ; // Set CS pin to active
  SPI.transfer(m_rxbuffer, len) ;
  digitalWrite(m_cs, m_cshigh?LOW:HIGH) ; // Set CS pin to inactive
  SPI.endTransaction() ;
  
  return true ;
}

bool ArduinoSpiHw::read(uint8_t *bytes, uint32_t len)
{
  if (!m_rxbuffer || len > m_size_buffer) return false ;

  memcpy(bytes, m_rxbuffer, len) ;
  return true;
}

bool ArduinoSpiHw::updateSettings()
{
  if (m_psettings) delete m_psettings ;
  m_psettings = NULL ;
  m_psettings = new SPISettings(m_speed_hz, m_msb?MSBFIRST:LSBFIRST, m_mode) ;
  if (!m_psettings) return false ;
  return true ;
}

bool ArduinoSpiHw::setSpeed(uint32_t speed)
{
  m_speed_hz = speed ;
  return updateSettings() ;
}

bool ArduinoSpiHw::setMode(uint8_t mode)
{
  m_mode = mode ;
  return updateSettings() ;
}

bool ArduinoSpiHw::set3Wire(bool b3Wire)
{
  // No support for 3 wire Arduino at the moment
  return false ;
}

bool ArduinoSpiHw::setLoop(bool bLoop)
{
  // No implementation of a loop back for Arduino
  return false ;
}

bool ArduinoSpiHw::setBPW(uint8_t bits)
{
  // Bits per word isn't properly implmented yet at the interface class
  // Requires a 2 byte word implementation of write(...)
  if (bits != 8) return false ;
  m_bitsperword = bits ;
  return true ;
}

bool ArduinoSpiHw::setBitOrder(bool bLSB)
{
  m_msb = !bLSB ;

  return updateSettings() ;  
}

bool ArduinoSpiHw::setCSHigh(bool bHigh)
{
  m_cshigh = bHigh ;
  return true ;
}

bool ArduinoGPIO::setup(uint32_t pin, enDirection eDir)
{
  pinMode(pin, eDir == enDirection::gpio_output?OUTPUT:INPUT) ;
  return true ;
}

bool ArduinoGPIO::output(uint32_t pin, enValue eVal)
{
  digitalWrite(pin, eVal==enValue::high?HIGH:LOW);
  return true ;
}

IHardwareGPIO::enValue ArduinoGPIO::input(uint32_t pin)
{
  return digitalRead(pin)==HIGH?enValue::high:enValue::low ;
}

bool ArduinoGPIO::register_interrupt(uint32_t pin, enEdge edge, void(*function)(void))
{
  int edge_type ;

  switch (edge){
  case rising:
    edge_type = RISING;
    break;
  case falling:
    edge_type = FALLING;
    break ;
  case both:
    edge_type = CHANGE;
    break;
  default:
    return false ;
  }

  //SPI.usingInterrupt(digitalPinToInterrupt(pin));
  attachInterrupt (digitalPinToInterrupt(pin), function, edge_type) ;

  return true;
}

void ArduinoTimer::microSleep(unsigned int nMicroSec)
{
  delayMicroseconds(nMicroSec) ;
}

void ArduinoTimer::milliSleep(unsigned int nMilliSec)
{
  delay(nMilliSec) ;
}
