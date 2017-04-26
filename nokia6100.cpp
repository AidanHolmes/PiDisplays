#include "hardware.hpp"
#include "wpihardware.hpp"
#include "spihardware.hpp"
#include "pcf8833lcd.hpp"
#include "displayimage.hpp"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "images/orbitalfruit.xbm"

bool textTest(PCF8833LCD &lcd)
{
  DisplayFont fnt ;
  wPi pi ;
  
  int fontfile = 0;
  char szStr1[] = "Only\nOne" ;
  char szStr2[] = "Once upon a time...\nThere was a test\nAnd this displayed\nSome text" ;
  
  if ((fontfile=open("fonts/6x9.fnt", O_RDONLY)) < 0) return false ;
  if (!fnt.loadFile(fontfile)) return false ;

  DisplayImage *img = fnt.createText(szStr2) ;
  if (!img) return false ;

  lcd.setBackground(0xFF, 0xFF, 0xFF) ;
  lcd.setForeground(0x00, 0x00, 0x00) ;
  
  lcd.writeImage(*img, PCF8833LCD::overwrite, 5,10) ;
  lcd.display() ;
  pi.milliSleep(1000) ;

  // Reuse old image, but this will truncate
  fnt.createText(szStr1, img) ;
  lcd.writeImage(*img, PCF8833LCD::overwrite, 5,10) ;
  lcd.display() ;
  
  delete img ;
  
  
  return true ;
}

bool resTest(PCF8833LCD &lcd)
{
  DisplayImage img ;

  if (!img.loadXBM(Xbitmap_width, Xbitmap_height, Xbitmap_bits)) fprintf(stderr, "Failed to load XBM image from resource\n") ;

  printf("Load XBM resource test...\n");
  if (!lcd.writeImage(img,PCF8833LCD::overwrite)){
    fprintf(stderr, "Cannot display XBM\n") ;
    return false ;
  }

  if (!lcd.display()){
    fprintf(stderr, "Display failed\n") ;
    return false ;
  }
  return true ;
}

bool aniTest(PCF8833LCD &lcd, IHardwareTimer &pi)
{
  DisplayImage img ;

  int testfile =0, testfile2 = 0;
  testfile=open("images/gear1.bin", O_RDONLY) ;
  if ((testfile2=open("images/gear2.bin", O_RDONLY)) < 0 || testfile < 0){
    fprintf(stdout, "Cannot open test files, skipping this test\n") ;
    return false ;
  }else{
    DisplayImage img2 ;
    if (!img.loadFile(testfile)) fprintf(stderr, "Failed to load test file 1 into image object\n") ;
    if (!img2.loadFile(testfile2)) fprintf(stderr, "Failed to load test file 2 into image object\n") ;
    for (int i=0; i < 5; i++){ // iterate loops of animation
      if (!lcd.writeImage(img2, PCF8833LCD::overwrite)){
	fprintf(stderr, "Failed to write image 2 to LCD object\n") ;
	return false ;
      }
      if (!lcd.display()) fprintf(stderr, "LCD display write failed\n") ;
      pi.milliSleep(200) ;
      if (!lcd.writeImage(img, PCF8833LCD::overwrite)){
	fprintf(stderr, "Failed to write image 1 to LCD object\n") ;
	return false ;
      }
      if (!lcd.display()) fprintf(stderr, "LCD display write failed\n") ;
      pi.milliSleep(200) ;
    }
  }

  return true ;
}

bool jpgTest(PCF8833LCD &lcd, const char szFile[])
{
  DisplayImage img ;

  if (!img.loadJPG(szFile)){
    return false ;
  }

  if (!lcd.writeImage(img, PCF8833LCD::overwrite)){
    fprintf(stderr, "Failed to write image to LCD image buffer\n") ;
    return false ;
  }

  if (!lcd.display()) fprintf(stderr, "LCD display write failed\n") ;
  
  return true ;
}

int main(int argc, char **argv)
{
  // BCM pin
  const int reset_pin = 25 ;
  
  // Create new wiringPi instance
  wPi pi ; // init GPIO

  // Create spidev instance. wiringPi interface doesn't seem to work
  // The SPIDEV code has more configuration to handle devices. 
  spiHw spi ;

  // Pi has only one bus available on the user pins. 
  // Two devices 0,0 and 0,1 are available (CS0 & CS1). 
  if (!spi.spiopen(0,0)){ // init SPI
    fprintf(stderr, "Cannot Open SPI\n") ;
    return 0;
  }

  // 1 KHz = 1000 Hz
  // 1 MHz = 1000 KHz
  // Speed set in Hz to 6MHz as documented by James P. Lynch
  spi.setSpeed(6000000) ;

  PCF8833LCD lcd ;

  lcd.setGPIO(pi) ;
  lcd.setSPI(spi) ;
  lcd.setTime(pi) ;

  if (!lcd.setup(132,132, reset_pin)){
    fprintf(stderr,"Failed to setup LCD\n") ;
    return -1 ;
  }

  if (!lcd.initialise()){
    fprintf(stderr,"Failed to initialise LCD\n") ;
    return -1 ;
  }

  spi.printState() ;

  lcd.clearImage() ;
  
  printf("Animating display...\n") ;

  aniTest(lcd, pi) ;
  
  pi.milliSleep(1000) ;

  if (!textTest(lcd)) fprintf(stderr, "Failed to run text test\n") ;

  pi.milliSleep(2000) ;
  
  resTest(lcd) ;

  pi.milliSleep(2000) ;
  

  printf("Loading testing JPEG file\n") ;
  //jpgTest(lcd, "images/test.jpg") ;
  
  //pi.milliSleep(3000) ;
  jpgTest(lcd, "images/test2.jpg") ;
  
  pi.milliSleep(3000) ;
  jpgTest(lcd, "images/test3.jpg") ;
  pi.milliSleep(3000) ;
  jpgTest(lcd, "images/test4.jpg") ;

  pi.milliSleep(5000) ;
  
  lcd.turnOff();
  
  return 1 ;
}
