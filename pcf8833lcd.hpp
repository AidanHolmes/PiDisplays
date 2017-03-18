#ifndef __PCF8833_LCD_HW_HPP
#define __PCF8833_LCD_HW_HPP

// macro required for display image to support friend class
#define DISPLAY_PCF8833LCD

#include "hardware.hpp"
#include "displayimage.hpp"
#include <stdint.h>

///////////////////////////////////////////////////
//
// Library for the Philips Nokia 6100 display
// driven over SPI
//
// Copyright Aidan Holmes 2016
//
///////////////////////////////////////////////////


// Generous 10MB image limit for single image files
#define XMB_LOAD_MAX_SIZE 10485760

class PCF8833LCD{
public:
  PCF8833LCD() ;
  ~PCF8833LCD() ;

  enum enMode{overwrite, overlay, exclusive} ;
  
  // GPIO and SPI interfaces must be configured and set 
  // prior to the LCD object using them. This means initialised
  // and opened. This object just drives an already configured interface
  void setGPIO(IHardwareGPIO &gpio) ;
  void setSPI (IHardwareSPI &spi) ;
  void setTime (IHardwareTimer &time){m_pTime = &time;}

  // Call setup after setting interfaces
  bool setup(unsigned int width, 
             unsigned int height, 
	     unsigned int reset_pin);

  // Initialise and turn on the LCD display.
  // Call after setup
  bool initialise(); // Call after setting interfaces


  bool writeCmd(uint8_t byte) ;
  bool writeData(uint8_t byte) ;

  // Turn off the display. Contents still remain in memory
  bool turnOff() ;

  // Turn on the display.
  bool turnOn() ;

  // Set image to background colour
  bool clearImage() ;

  // Set where the 0 row origin is on the display
  // bottom is defined as where the connector ribbon is situated
  bool setYOrigin(bool bTop) ;

  // Load an image object into the display buffer
  // The image can add extra pixels, overwrite or xor pixels already set.
  // Call display() to send image to LCD
  bool writeImage(DisplayImage &img, enum enMode eMode, int xoffset=0, int yoffset=0);

  // Write display buffer to the LCD
  bool display() ;

  // Change background colour. This is used where no known colour is available or
  // for 2 bit images
  void setBackground(uint8_t red, uint8_t green, uint8_t blue);

  // Change background colour. This is used where no known colour is available or
  // for 2 bit images
  void setForeground(uint8_t red, uint8_t green, uint8_t blue);
  
protected:
  bool verify() ;

  IHardwareGPIO *m_pGPIO ;
  IHardwareSPI *m_pSPI ;
  IHardwareTimer *m_pTime ;
  unsigned int m_resetpin, m_width, m_height ;

  // 3 colours with 1 colour per byte
  // Use unpacked bytes for data
  uint8_t *m_pDisplay ;
  unsigned int m_nDisplaySize ;

  // Specific to driver
  uint8_t m_nMADCTL ; // Memory address control
  uint16_t m_nBGCol ; // Default background colour
  uint16_t m_nFGCol ; // Default foreground colour
};


#endif // __PCF8833_LCD_HW_HPP
