#ifndef __DISPLAYIMAGE_HW_HPP
#define __DISPLAYIMAGE_HW_HPP

#include <stdint.h>
#include <stddef.h>

class SDD1306OLED ; // OLED display using SDD1306
class DisplayFont ; 
class PCF8833LCD ; // Philips Colour LCD Display

// Generous 10MB image limit for single image files
#define XMB_LOAD_MAX_SIZE 10485760

class DisplayImage{
public:
  DisplayImage() ;
  ~DisplayImage();
  friend class SDD1306OLED ;
  friend class PCF8833LCD ;
  friend class DisplayFont ;

  // Create blank image. Basically a call to allocateImg but with a follow up to
  // make the image blank. Will destroy old image if already exists.
  bool createImage(unsigned int width, unsigned int height, unsigned int bitdepth) ;

  // Draw a rectangle. Set bFill to true to fill the rectangle
  // false is returned if any of the lines in the rectangle could not be drawn due to being outside the
  // viewable area. Return values can be mostly ignored.
  bool drawRect(int x0, int y0, int width, int height, bool bFill=false);
  
  // Draw straight lines across the image. Start and end points do not need to 
  // be within the bounds of the image and will be clipped.
  // Returns false if the line cannot be drawn at all, but this is more for information than an error.
  bool drawLine(int x0, int y0, int x1, int y1) ;

  // Load from existing header file. Used when XBM resource is built into the
  // executable binary
  bool loadXBM(unsigned int w, unsigned int h, unsigned char *bits) ;

  // Load a 24 bit jpeg into the image object (becomes 32bit with alpha)
  bool loadJPG(char *szFilename) ;
  
  // Load a custom binary representation from file.
  // Use XBM2Bin utility to create
  bool loadFile(int f) ;

  // Set all bits to zero, clearing the image values
  bool zeroImg() ;
  
  // create a character representation of the image for terminal
  // useful for debug and not much else
  void printImg() ;

  // Set the value of a pixel. Use bSet to set as ON of OFF with true/false values
  bool setPixel(unsigned int x, unsigned int y, bool bSet) ;

  // Create distribution. Returns false if no image or colour depth unsupported
  // This is for colour RGB images. Returns false for 1bit images.
  bool createDistribution() ;

  // Get number of red pixels at the value set by intensity contained in the image
  // intensity is 0-255
  unsigned int getRedDistribution(uint8_t intensity) ;

  // Get number of green pixels at the value set by intensity contained in the image
  // intensity is 0-255
  unsigned int getGreenDistribution(uint8_t intensity) ;

  // Get number of blue pixels at the value set by intensity contained in the image
  // intensity is 0-255
  unsigned int getBlueDistribution(uint8_t intensity) ;

  // Utility function. Coverts 8bit colour to 4bit.
  static uint8_t to4bit(uint8_t byte) ;

protected:
  // Draw vertical lines. Used internally, but not needed for users as
  // this is called by draw methods when required
  bool drawV(int x, int y0, int y1);

  // Draw horizonal lines from x0 to x1 along y.
  // Not required for direct user access but used by other draw methods
  bool drawH(int x0, int x1, int y);

  bool allocateImg(unsigned int height, unsigned int width, unsigned int bitdepth) ;
  unsigned char *m_img ;
  unsigned int m_memsize ;
  unsigned int m_width ;
  unsigned int m_height ;
  unsigned int m_stride ;
  bool m_bResourceImage ;
  unsigned int m_colourbitdepth ;
  unsigned int m_red_distribution[256] ;
  unsigned int m_green_distribution[256] ;
  unsigned int m_blue_distribution[256] ;
  
};

class DisplayFont{
public:
  DisplayFont();
  ~DisplayFont();

  // Load font from file. Use utility
  // to convert PSF compressed files to a binary format to load
  bool loadFile(int f) ;

  // Create a buffer with a text string to display
  // This returns an DisplayImage which can be written to the display
  // The returned image will need to be deleted by the caller
  DisplayImage *createText(char *szTxt, DisplayImage *cimg = NULL) ;

protected:
  uint32_t m_nFontWidth ;
  uint32_t m_nFontHeight ;
  uint32_t m_nTotalChars ;
  unsigned char *m_pBuffer ;
};


#endif