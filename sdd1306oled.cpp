#include "sdd1306oled.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

SDD1306OLED::SDD1306OLED()
{
  m_pGPIO = NULL ;
  m_pSPI = NULL ;
  m_pTime = NULL ;
  m_dcpin = 0 ;
  m_resetpin = 0 ;
  m_width = 64 ;
  m_height = 48 ;
  m_pDisplay = 0 ;
}

SDD1306OLED::~SDD1306OLED()
{
  if (m_pDisplay) delete[] m_pDisplay ;
}

void SDD1306OLED::setGPIO(IHardwareGPIO &gpio)
{
  m_pGPIO = &gpio ;
}

void SDD1306OLED::setSPI(IHardwareSPI &spi)
{
  m_pSPI = &spi ;
}

bool SDD1306OLED::setup(unsigned int width, unsigned int height, unsigned int dc_pin, unsigned int reset_pin)
{
  m_dcpin = dc_pin ;
  m_resetpin = reset_pin ;
  m_width = width ;
  m_height = height;

  if (!verify()) return false ;

  if (height % 8 > 0){
    fprintf(stderr, "Height needs to be a multiple of 8 to fit into a memory page\n") ;
    return false ;
  }

  // Note that there's no stride calculation as this code
  // assumes that the display height is exact multiple of 8 to fit a byte fully.
  m_pDisplay = new uint8_t[(width * height)/8] ;
  if (!m_pDisplay){
    fprintf(stderr, "setup: Memory error\n") ; 
    return false ;
  }

  m_pGPIO->setup(m_dcpin, IHardwareGPIO::gpio_output) ;
  m_pGPIO->setup(m_resetpin, IHardwareGPIO::gpio_output) ;
  
  return true ;
}

bool SDD1306OLED::verify()
{
  // Internal check that this object is configured

  if (m_dcpin == 0) return false ;
  if (m_resetpin == 0) return false;
  if (!m_pGPIO) return false ;
  if (!m_pSPI) return false ;
  if (!m_pTime) return false ;

  return true ; 
}

bool SDD1306OLED::writeCmd(uint8_t byte)
{
  m_pGPIO->output(m_dcpin, IHardwareGPIO::low) ;
  return m_pSPI->write(byte) ;
}

bool SDD1306OLED::writeData(uint8_t byte)
{
  m_pGPIO->output(m_dcpin, IHardwareGPIO::high) ;
  return m_pSPI->write(byte) ;
}

bool SDD1306OLED::turnOff()
{
  if (!writeCmd(0xAE)) return false ;
  m_pTime->milliSleep(100) ;
  m_pGPIO->output(m_resetpin, IHardwareGPIO::low) ;

  return true ;
}

bool SDD1306OLED::turnOn()
{
  // Setting low first may be unnecessary when already turned off using the 
  // other method in this object. This is a power on enable. 
  m_pGPIO->output(m_resetpin, IHardwareGPIO::low) ;
  m_pTime->microSleep(100) ; // recommended 3us
  m_pGPIO->output(m_resetpin, IHardwareGPIO::high) ;

  if (!writeCmd(0xAF)) return false ;

  return true ;
}

bool SDD1306OLED::setColumnAddress(uint16_t address)
{
  if (!verify()) return false ;

  writeCmd((0x10 | address >> 4) + 0x02) ;
  writeCmd(0x0F & address) ;

  return true ;
}

bool SDD1306OLED::initialise()
{
  if (!verify()) return false ;

  // Set initial state for pins
  m_pGPIO->output(m_dcpin, IHardwareGPIO::high) ;
  m_pGPIO->output(m_resetpin, IHardwareGPIO::low) ;
  m_pTime->milliSleep(100) ;


  // Initialisation sequence and defaults copied from SparkFuns 
  // Arduino code. Only part of the chip graphic memory is in use with this
  // smaller display. Implementations can vary and you cannot assume a 64x48 is mapped
  // the same as another manufacturers 64x48. 

  // Enable the reset pin (disable reset essentially)
  m_pGPIO->output(m_resetpin, IHardwareGPIO::high) ;
  m_pTime->milliSleep(5) ; // Sleep for 5ms
  m_pGPIO->output(m_resetpin, IHardwareGPIO::low) ;
  m_pTime->milliSleep(10) ; // Sleep for 10ms 
  m_pGPIO->output(m_resetpin, IHardwareGPIO::high) ;

  // Send display off command
  writeCmd(0xAE) ;

  // Set display clock div
  writeCmd(0xD5) ;
  writeCmd(0x80) ; // 1000 0000 seems to suggest no divide ratio and default frequency

  // Set multiplex
  writeCmd(0xA8) ;
  writeCmd(0x2F) ; // Works for the SparkFun 64x48 board

  // Set display offset
  writeCmd(0xD3) ; 
  writeCmd(0x00) ; // Zero offset

  // Set startline offset
  writeCmd(0x40 | 0x00) ; // Zero offset

  // Charge pump
  writeCmd(0x8D) ;
  writeCmd(0x14) ; // Enable pump

  // Normal display
  writeCmd(0xA6) ;
  
  // Display all on resume
  writeCmd(0xA4) ;

  // Define memory mode
  writeCmd(0x20) ;
  writeCmd(0x10) ; // Page address mode (0x00 for horizontal, 0x01 for vertical)

  // SEG remapping
  writeCmd(0xA0 | 0x01) ; // Map SEG so rows are left to right from display ribbon

  // COM scan mapping (ascending or descending)
  writeCmd(0xC0) ; // Ascending from display ribbon connector

  // Set com pin mapping
  writeCmd(0xDA) ;
  writeCmd(0x12) ; // Sequential mapping with left to right remap

  // Set contrast
  writeCmd(0x81) ;
  writeCmd(0xCF) ; // Need to be aware of current draw when setting. 0x00 to 0xFF are valid.

  // Set precharge
  writeCmd(0xD9) ;
  writeCmd(0xF1) ; // Phase 1: 1 dclk (max 15), Phase 2: 15 dclks (max 15)

  // Set com deselect
  writeCmd(0xDB) ;
  writeCmd(0x40) ; // above 0.83 x Vcc for regulator output.

  // Display On
  writeCmd(0xAF) ;

  m_pTime->milliSleep(100) ; // Sleep

  return true ;
}

bool SDD1306OLED::setYOrigin(bool bTop)
{
  if (!verify()) return false ;
  
  // COM scan mapping (ascending or descending)
  if (bTop) writeCmd(0xC8) ; // Descending COM ordering
  else writeCmd(0xC0) ; // Ascending from display ribbon connector

  return true ;
}

bool SDD1306OLED::showAllPixels()
{
  if (!verify()) return false ;

  uint8_t pages = m_height/8;

  for (uint8_t p=0; p < pages; p++){
    setColumnAddress(0) ; // Reset column
    writeCmd(0xB0 | p) ; // Set page
    for (unsigned int col=0; col < m_width; col++){
      writeData(0xFF) ; // Set all pixels ON
    }
  }

  return true ;
}


bool SDD1306OLED::writeImage(DisplayImage &img, enum enMode eMode, int xoffset, int yoffset)
{
  uint8_t val = 0;
  int imgx=0, imgy =0;
  if (!verify()) return false ;

  if (!m_pDisplay){
    fprintf(stderr, "writeImage: display not setup\n") ;
    return false ;
  }

  // iterate throught the display buffer not the image buffer being written.
  // This should auto clip any parts of the image outside of the buffer region
  for (int cy=0; cy < (int)m_height; cy++){
    for (int cx=0; cx < (int)m_width; cx++){
      imgx = cx - xoffset ;
      imgy = cy - yoffset ;
      if (imgx < 0 || imgx >= (int)img.m_width || imgy < 0 || imgy >= (int)img.m_height){
	val = 0; // outside of image
      }
      else{
	// Image buffer is a bit array with bytes assigned along rows
	val = img.m_img[imgx/8+(imgy*img.m_stride)] & (1 << (imgx % 8)) ;
      }
      
      // val is greater than zero is set or zero if unset pixel
      if (val > 0){
	if(eMode == exclusive){
	  m_pDisplay[cx+ (cy/8)*m_width] ^= 1 << (cy%8);
	}
	else{
	  // Display is a bit array. This combines pixels with ones already set on the display
	  m_pDisplay[cx+ (cy/8)*m_width] |= 1 << (cy%8);
	}
      }else if (eMode == overwrite){
	// If this is an overwrite of the display and not additive (overlayed) 
	// then remove the pixel setting
	m_pDisplay[cx+ (cy/8)*m_width] &= ~(1 << (cy%8));
      }
    }
  }

  return true ;
}

bool SDD1306OLED::display()
{
  uint8_t pages = m_height/8;

  for (uint8_t p=0; p < pages; p++){
    setColumnAddress(0) ; // Reset column
    writeCmd(0xB0 | p) ; // Set page
    for (unsigned int col=0; col < m_width; col++){
      writeData(m_pDisplay[(p*m_width)+col]) ;
    }
  }

  return true ;

}

