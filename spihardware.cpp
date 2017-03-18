#include "spihardware.hpp"
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define SPIDEV_MAXPATH 1024

spiHw::spiHw()
{
  m_fd = -1;
  m_mode = 0;
  m_bitsperword = 0;
  m_maxspeed_hz = 0;
  m_speed_hz = 0;
  m_rxbuffer = NULL ;
  m_size_buffer = 0;
}
spiHw::~spiHw()
{
  if (m_rxbuffer) delete[] m_rxbuffer ;
  if (m_fd > 0){
    close(m_fd) ;
  }
}

bool spiHw::setSpeed(uint32_t speed)
{
  if (m_fd < 0){
    fprintf(stderr, "setSpeed: SPI is not open\n") ;
    return false ;
  }

  if (ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1){
    fprintf(stderr, "setSpeed: Failed to set speed to %d\n", speed) ;
    return false ;
  }

  m_speed_hz = speed ;
  return true ;
}

bool spiHw::spiopen(uint32_t bus, uint32_t device)
{
  char path[SPIDEV_MAXPATH];
  uint8_t tmp8 ;
  uint32_t tmp32 ;

  if (snprintf(path, SPIDEV_MAXPATH, "/dev/spidev%d.%d", bus, device) >= SPIDEV_MAXPATH) {
    fprintf(stderr, "open: path invalid\n") ;
    return false ;
  }

  // if a file descriptor is already open then close 
  // and reopen here
  if (m_fd > 0) close(m_fd) ;
  m_fd = -1 ;

  if ((m_fd = open(path, O_RDWR, 0)) == -1) {
    fprintf(stderr, "open: Failed to open %s\n", path) ;
    return false ;
  }
  
  if (ioctl(m_fd, SPI_IOC_RD_MODE, &tmp8) == -1) {
    fprintf(stderr, "open: Failed to read mode\n") ;
    return false ;
  }
  m_mode = tmp8 ;
  
  if (ioctl(m_fd, SPI_IOC_RD_BITS_PER_WORD, &tmp8) == -1) {
    fprintf(stderr, "open: Failed to read BPW\n") ;
    return false ;
  }
  m_bitsperword = tmp8 ;
  
  if (ioctl(m_fd, SPI_IOC_RD_MAX_SPEED_HZ, &tmp32) == -1) {
    fprintf(stderr, "open: Failed to read max speed\n") ;
    return false ;
  }
  m_maxspeed_hz = tmp32 ;

  return true ;
}

void spiHw::printState()
{
  fprintf(stdout, "BPW: %d\n", m_bitsperword) ;
  fprintf(stdout, "cshigh: %s\n", m_mode & SPI_CS_HIGH?"true":"false") ;
  fprintf(stdout, "lsbfirst: %s\n", m_mode & SPI_LSB_FIRST?"true":"false") ;
  fprintf(stdout, "Speed: %d\n", m_maxspeed_hz) ;
  fprintf(stdout, "Mode: %d\n", m_mode & (SPI_CPHA | SPI_CPOL));
}

bool spiHw::write(uint8_t byte)
{
  return write(&byte, 1) ;
}

bool spiHw::write(uint8_t *bytes, uint32_t len)
{
  if (m_size_buffer < len){
    if (m_rxbuffer) delete[] m_rxbuffer ;
    m_rxbuffer = new uint8_t[len];
    if (!m_rxbuffer){
      m_size_buffer = 0 ;
      return false ;
    }
  }
  return xfer(bytes, m_rxbuffer, len) ;
}

bool spiHw::read(uint8_t *bytes, uint32_t len)
{
  if (!m_rxbuffer || len > m_size_buffer) return false ;

  memcpy(bytes, m_rxbuffer, len) ;
  return true;
}

bool spiHw::xfer(uint8_t *bytes, uint8_t *rxbuf, uint32_t len)
{
  // Do a xfer
  struct spi_ioc_transfer xfer;
  int status = 0;

  memset(&xfer, 0, sizeof(xfer));

  xfer.tx_buf = (unsigned long)bytes ;
  xfer.rx_buf = (unsigned long)rxbuf ;
  xfer.len = len ;
  xfer.delay_usecs = 0; // set to zero
  xfer.speed_hz = m_speed_hz > 0?m_speed_hz:m_maxspeed_hz ;
  xfer.bits_per_word = m_bitsperword ;
  
  status = ioctl(m_fd, SPI_IOC_MESSAGE(1), &xfer);
  if (status < 0){
    //fprintf(stderr, "SPI_IOC_MESSAGE(1) failed: %d\n", status) ;
    return false ;
  }

  // WA:
  // in CS_HIGH mode CS isn't pulled to low after transfer, but after read
  // reading 0 bytes doesnt matter but brings cs down
  // tomdean:
  // Stop generating an extra CS except in mode CS_HIGH
  if (m_mode & SPI_CS_HIGH) status = ::read(m_fd, &rxbuf[0], 0);

  return true ;
}

int spiHw::spidev_set_mode( int fd, uint8_t mode) {
  __u8 test;
  if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1) {
    return -1;
  }
  if (ioctl(fd, SPI_IOC_RD_MODE, &test) == -1) {
    return -1;
  }
  if (test != mode) {
    return -1;
  }
  return 0;
}

bool spiHw::setMode(uint8_t mode)
{
  if (mode > 3){
    fprintf(stderr, "set_mode: parameter is over 3\n") ;
    return false ;
  }

  if (m_fd < 0){
    fprintf(stderr, "set_mode: SPI hasn't been opened\n") ;
    return false ;
  }

  // Clear the SPI_CPHA and SPI_CPOL to set again with 'mode'
  uint8_t tmp = ( m_mode & ~(SPI_CPHA | SPI_CPOL) ) | mode ;
  if (spidev_set_mode(m_fd, tmp) == -1){
    fprintf(stderr, "set_mode: failed to set mode\n") ;
    return false ;
  }

  m_mode = tmp ;
  return true ;
}

bool spiHw::set3Wire(bool b3Wire)
{
  uint8_t tmp ;

  if (b3Wire)
    tmp = m_mode | SPI_3WIRE ;
  else
    tmp = m_mode & ~SPI_3WIRE ;

  if (spidev_set_mode(m_fd, tmp) == -1){
    fprintf(stderr, "set3Wire: Failed to set parameter\n") ;
    return false ;
  }

  return true ;  
}

bool spiHw::setLoop(bool bLoop)
{
  uint8_t tmp ;

  if (bLoop)
    tmp = m_mode | SPI_LOOP ;
  else
    tmp = m_mode & ~SPI_LOOP ;

  if (spidev_set_mode(m_fd, tmp) == -1){
    fprintf(stderr, "setLoop: Failed to set parameter\n") ;
    return false ;
  }

  return true ;  
}

bool spiHw::setBPW(uint8_t bits)
{
  if (bits < 8 || bits > 16){
    fprintf(stderr, "setBPW: Incorrect parameter\n") ;
    return false ;
  }

  if (m_fd < 0){
    fprintf(stderr, "SPI is not open\n") ;
    return false ;
  }

  if (ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1){
    fprintf(stderr, "setBPW: Failed to set bits per word to %d\n", bits) ;
    return false ;
  }

  m_bitsperword = bits ;
  return true ;
}

bool spiHw::setBitOrder(bool bLSB)
{
  uint8_t tmp ;

  if (bLSB)
    tmp = m_mode | SPI_LSB_FIRST ;
  else
    tmp = m_mode & ~SPI_LSB_FIRST ;

  if (spidev_set_mode(m_fd, tmp) == -1){
    fprintf(stderr, "setBitOrder: Failed to set parameter\n") ;
    return false ;
  }

  return true ;  
}

bool spiHw::setCSHigh(bool bHigh)
{
  uint8_t tmp ;

  if (bHigh)
    tmp = m_mode | SPI_CS_HIGH ;
  else
    tmp = m_mode & ~SPI_CS_HIGH ;

  if (spidev_set_mode(m_fd, tmp) == -1){
    fprintf(stderr, "cshigh: Failed to set parameter\n") ;
    return false ;
  }

  return true ;
}
