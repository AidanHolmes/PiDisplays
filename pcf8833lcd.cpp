#include "pcf8833lcd.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

PCF8833LCD::PCF8833LCD()
{
  m_pGPIO = NULL ;
  m_pSPI = NULL ;
  m_pTime = NULL ;
  m_resetpin = 0 ;
  m_width = 132 ;
  m_height = 132 ;
  m_pDisplay = 0 ;
  m_nDisplaySize = 0;
  m_nMADCTL = 0x10 ; // no mirroring for x and y. line progression top to bottom. RGB colour
  m_nBGCol = 0x0F00 ; // Red
  m_nFGCol = 0x0FFF ; // White
}

PCF8833LCD::~PCF8833LCD()
{
  if (m_pDisplay) delete[] m_pDisplay ;
}

void PCF8833LCD::setGPIO(IHardwareGPIO &gpio)
{
  m_pGPIO = &gpio ;
}

void PCF8833LCD::setSPI(IHardwareSPI &spi)
{
  m_pSPI = &spi ;
}

bool PCF8833LCD::setup(unsigned int width, unsigned int height, unsigned int reset_pin)
{
  m_resetpin = reset_pin ;
  m_width = width ;
  m_height = height;

  if (!verify()) return false ;

  if (height > 132 || width > 132){
    fprintf(stderr, "Hardware doesn't support dimensions specified\n") ;
    return false;
  }
  
  // Allocate memory
  m_nDisplaySize = width * height * 3 ;
  m_pDisplay = new uint8_t[m_nDisplaySize] ;
  if (!m_pDisplay){
    fprintf(stderr, "setup: Memory error\n") ; 
    return false ;
  }

  m_pGPIO->setup(m_resetpin, IHardwareGPIO::gpio_output) ;
  
  return true ;
}

bool PCF8833LCD::verify()
{
  // Internal check that this object is configured

  if (m_resetpin == 0) return false;
  if (!m_pGPIO) return false ;
  if (!m_pSPI) return false ;
  if (!m_pTime) return false ;

  return true ; 
}

bool PCF8833LCD::writeCmd(uint8_t byte)
{
  // Command is a 0 bit. Use the 9bit buffered routine
  return m_pSPI->write9bit(0, byte) ;
}

bool PCF8833LCD::writeData(uint8_t byte)
{
  // Data is a 1 bit. Use the 9bit buffered routine
  return m_pSPI->write9bit(1, byte) ;
}

bool PCF8833LCD::turnOff()
{
  if (!writeCmd(0x28)) return false ;
  if (!writeCmd(0x10)) return false ;

  m_pSPI->flush9bit(0, 0x00) ;
  return true ;
}

bool PCF8833LCD::turnOn()
{
  if (!writeCmd(0x11)) return false ;
  if (!writeCmd(0x03)) return false ;
  m_pTime->milliSleep(1000) ;
  if (!writeCmd(0x29)) return false ;

  m_pSPI->flush9bit(0, 0x00) ;
  return true ;
}

bool PCF8833LCD::initialise()
{
  if (!verify()) return false ;

  m_pSPI->setMode(0) ; // CPOL = 0, CPHA = 0
  m_pSPI->setCSHigh(false) ; // CS is low for writing data

  m_pGPIO->setup(m_resetpin, IHardwareGPIO::gpio_output) ;
  
  m_pGPIO->output(m_resetpin, IHardwareGPIO::low) ;
  m_pTime->milliSleep(2000) ; 
  m_pGPIO->output(m_resetpin, IHardwareGPIO::high) ;
  m_pTime->milliSleep(2000) ;

  // SLEEPOUT - turn on booster circuits
  if (!writeCmd(0x11)) return false ;

  // turn on booster voltage
  writeCmd(0x03) ;

  // Inversion off - not sure why it needs to be on
  writeCmd(0x20) ;

  // Colour pixel format 12 bits
  writeCmd(0x3A) ;
  writeData(0x03) ;

  // Set up memory access controller. MADCTL 0x36
  // Data is 0xX8 - BGR
  // 0xX0 - RGB
  // 0x1X - no mirror x or y. RAM write in x direction. Line addressing top to bottom
  // 0x9X - mirror y. x not mirrored. RAM write in x direction. Line addressing top to bottom
  // 0xCX - mirror x and y. RAM write in x direction. Line addressing top to bottom
  // 0x0X - no mirror x or y. RAM write in x direction. Line addressing bottom to top.
  writeCmd(0x36) ;
  writeData(m_nMADCTL) ;

  // Set contrast
  writeCmd(0x25) ;
  //writeData(0x30) ;
  writeData(0x3F) ; // Increased a bit more for images

  m_pTime->milliSleep(2000) ;

  // display on
  writeCmd(0x29) ;

  // Flush out NOOP command to bus
  m_pSPI->flush9bit(0, 0x00) ;
  
  return true ;
}

bool PCF8833LCD::setYOrigin(bool bTop)
{
  if (!verify()) return false ;

  writeCmd(0x36) ;
  // COM scan mapping (ascending or descending)
  if (bTop) m_nMADCTL |= 0x80 ; 
  else m_nMADCTL &= ~0x80 ;

  writeData(m_nMADCTL) ; 

  // Flush out NOOP command to bus
  m_pSPI->flush9bit(0, 0x00) ;

  return true ;
}

bool PCF8833LCD::clearImage()
{
  if (!verify()) return false ;

  uint32_t i = 0 ;
  while (i < m_nDisplaySize){
    m_pDisplay[i++] = 0x0F & (m_nBGCol >> 8) ; // red
    m_pDisplay[i++] = 0x0F & (m_nBGCol >> 4) ; // green    
    m_pDisplay[i++] = 0x0F & (m_nBGCol) ; // blue
  }
  
  return true ;
}

bool PCF8833LCD::writeImage(DisplayImage &img, enum enMode eMode, int xoffset, int yoffset)
{
  int val = 0;
  int imgx=0, imgy =0;
  uint32_t pixel =0, srcpixel = 0;

  if (!verify()) return false ;

  if (img.m_colourbitdepth != 1 && img.m_colourbitdepth != 32){
    fprintf(stderr, "Unsupported colour bit depth\n") ;
    return false ;
  }

  if (!m_pDisplay){
    fprintf(stderr, "writeImage: display not setup\n") ;
    return false ;
  }

  // iterate through the display buffer not the image buffer being written.
  // This should auto clip any parts of the image outside of the buffer region
  for (int cy=0; cy < (int)m_height; cy++){
    for (int cx=0; cx < (int)m_width; cx++){
      imgx = cx - xoffset ;
      imgy = cy - yoffset ;
      if (imgx < 0 || imgx >= (int)img.m_width || imgy < 0 || imgy >= (int)img.m_height){
	val = -1; // outside of image
      }
      else{
	if (img.m_colourbitdepth == 1){
	  // Image buffer is a bit array with bytes assigned along rows
	  val = img.m_img[imgx/8+(imgy*img.m_stride)] & (1 << (imgx % 8)) ;
	}else{
	  val = 1 ;
	}
      }

      pixel += 3 ;

      if (img.m_colourbitdepth == 1){
	// val is greater than zero is set or zero if unset pixel
	if (val > 0){
	  if(eMode == exclusive){
	    m_pDisplay[pixel] ^= 0x0F & (m_nFGCol >> 8) ;
	    m_pDisplay[pixel+1] ^= 0x0F & (m_nFGCol >> 4) ;
	    m_pDisplay[pixel+2] ^= 0x0F & (m_nFGCol) ;
	  }else{
	    m_pDisplay[pixel] = 0x0F & (m_nFGCol >> 8) ;
	    m_pDisplay[pixel+1] = 0x0F & (m_nFGCol >> 4) ;
	    m_pDisplay[pixel+2] = 0x0F & (m_nFGCol) ;
	  }
	}else if (eMode == overwrite){
	  // If this is an overwrite of the display and not additive (overlayed) 
	  // then remove the pixel setting
	  // Use background colour
	  m_pDisplay[pixel] = 0x0F & (m_nBGCol >> 8); // red
	  m_pDisplay[pixel+1] = 0x0F & (m_nBGCol >> 4); // green
	  m_pDisplay[pixel+2] = 0x0F & (m_nBGCol); // blue
	}
	// else retain the pixels as they were in m_pDisplay
      }else if (img.m_colourbitdepth == 32){
	if (val > 0){
	  srcpixel = (imgx*4) + (imgy*img.m_stride) ;
	  if(eMode == exclusive){
	    m_pDisplay[pixel] ^= DisplayImage::to4bit(img.m_img[srcpixel]) ; // to 4 bit
	    m_pDisplay[pixel+1] ^= DisplayImage::to4bit(img.m_img[srcpixel+1]); // to 4 bit
	    m_pDisplay[pixel+2] ^= DisplayImage::to4bit(img.m_img[srcpixel+2]); // to 4 bit
	  }else{ // overwrite (not overlay written here yet)
	    m_pDisplay[pixel] = DisplayImage::to4bit(img.m_img[srcpixel]) ; // to 4 bit
	    m_pDisplay[pixel+1] = DisplayImage::to4bit(img.m_img[srcpixel+1]) ; // to 4 bit
	    m_pDisplay[pixel+2] = DisplayImage::to4bit(img.m_img[srcpixel+2]) ; // to 4 bit
	  }
	}else if (eMode == overwrite){
	  // If this is an overwrite of the display and not additive (overlayed)
	  // then remove the pixel setting
	  // Use background colour
	  m_pDisplay[pixel] = 0x0F & (m_nBGCol >> 8); // red
	  m_pDisplay[pixel+1] = 0x0F & (m_nBGCol >> 4); // green
	  m_pDisplay[pixel+2] = 0x0F & (m_nBGCol); // blue
	}
	// else retain the pixels as they were in m_pDisplay
	
      }
    }
  }

  
  return true ;
}

bool PCF8833LCD::display()
{
  uint32_t j =0;
  
  // Display OFF
  //writeCmd(0x28);

  // Column address set (command 0x2A)
  writeCmd(0x2A);
  writeData(0);
  writeData(m_width - 1);
  
  // Page address set (command 0x2B)
  writeCmd(0x2B);
  writeData(0);
  writeData(m_height - 1);

  // WRITE MEMORY
  writeCmd(0x2C) ;

  j= 0 ;
  uint8_t byteout = 0x00 ;
  while (j < m_nDisplaySize) {
    byteout = 0x00 ;
    byteout = m_pDisplay[j++] << 4 ; // 
    if (j < m_nDisplaySize){
      byteout |= m_pDisplay[j++] & 0x0F ; //
    }else{
      byteout &= 0xF0 ;
    }
    writeData(byteout);
  }

  // Display On
  //writeCmd(0x29);

  m_pSPI->flush9bit(0,0x00) ;
  return true ;
}

void PCF8833LCD::setBackground(uint8_t red, uint8_t green, uint8_t blue)
{
  m_nBGCol = 0x0FFF & (red << 8 | green << 4 | blue) ; 
}

void PCF8833LCD::setForeground(uint8_t red, uint8_t green, uint8_t blue)
{
  m_nFGCol = 0x0FFF & (red << 8 | green << 4 | blue) ; 
}
