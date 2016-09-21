#ifndef __SDD1306_OLED_HW_HPP
#define __SDD1306_OLED_HW_HPP

#include "hardware.hpp"
#include "displayimage.hpp"
#include <stdint.h>

class SDD1306OLED{
public:
  SDD1306OLED() ;
  ~SDD1306OLED() ;

  enum enMode{overwrite, overlay, exclusive} ;
  
  // GPIO and SPI interfaces must be configured and set 
  // prior to the oled object using them. This means initialised
  // and opened. This object just drives an already configured interface
  void setGPIO(IHardwareGPIO &gpio) ;
  void setSPI (IHardwareSPI &spi) ;
  void setTime (IHardwareTimer &time){m_pTime = &time;}

  // Call setup after setting interfaces
  bool setup(unsigned int width, 
             unsigned int height, 
	     unsigned int dc_pin, 
	     unsigned int reset_pin);

  // Initialise and turn on the oled display.
  // Call after setup
  bool initialise(); // Call after setting interfaces


  bool writeCmd(uint8_t byte) ;
  bool writeData(uint8_t byte) ;

  // Turn off the display. Contents still remain in memory
  bool turnOff() ;

  // Turn on the display.
  bool turnOn() ;

  // Test function which writes directly to the OLED bypassing
  // the display buffer. Sets all pixels ON
  bool showAllPixels() ;

  // Set where the 0 row origin is on the display
  // bottom is defined as where the connector ribbion is situated
  bool setYOrigin(bool bTop) ;

  // Load an image object into the display buffer
  // The image can add extra pixels, overwrite or xor pixels already set.
  // Call display() to send image to OLED
  bool writeImage(DisplayImage &img, enum enMode eMode, int xoffset=0, int yoffset=0);

  // Write display buffer to the OLED
  bool display() ;

protected:
  bool verify() ;
  bool setColumnAddress(uint16_t address);

  IHardwareGPIO *m_pGPIO ;
  IHardwareSPI *m_pSPI ;
  IHardwareTimer *m_pTime ;
  unsigned int m_dcpin, m_resetpin, m_width, m_height ;

  // Display buffer used in this class is different to the 
  // image buffers used to write images. This is arranged to match the page format
  // used in the SDD. Bytes represent columns of data within the pages
  // A future enhancement may replace this buffer with another DisplayImage object but this
  // would require support in DisplayImage or change to display() function.
  uint8_t *m_pDisplay ;
};

#endif // __SDD1306_OLED_HW_HPP
