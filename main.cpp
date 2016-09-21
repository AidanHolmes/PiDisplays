#include "hardware.hpp"
#include "wpihardware.hpp"
#include "spihardware.hpp"
#include "sdd1306oled.hpp"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "images/orbitalfruit.xbm"

int main(int argc, char **argv)
{
  const int dc_pin    = 24;
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

  spi.printState() ;

  //spi.setSpeed(500000) ;

  // Create the OLED object
  SDD1306OLED oled ;

  // Set interfaces. wiringPi provides all of these
  oled.setGPIO(pi) ;
  oled.setSPI(spi) ;
  oled.setTime(pi) ;

  // Define pins in use and the screen size
  // This method does the configuration
  if (!oled.setup(64,48,dc_pin,reset_pin)){
    fprintf(stderr, "Failed to setup\n") ;
    return 0;
  }

  // Initialise the display and turn on ready for data.
  // This can be run at any time to also reset the display. 
  if (!oled.initialise()){
    fprintf(stderr, "Failed to initialise the oled\n") ;
    return 0;
  }

  if (argc > 1){
    if (strcmp(argv[1], "-on") == 0){
      // Run a small test to verify the display is on and working
      oled.showAllPixels() ;
      return 1 ;
    }
    if (strcmp(argv[1], "-off") == 0){
      oled.turnOff();
      return 1 ;
    }
  }


  // XBMs assume x,y origin at top left of display
  oled.setYOrigin(true) ;

  printf ("Displaying line...\n") ;
  DisplayImage imgLines ;
  if (!imgLines.createImage(64,48,1)) fprintf(stderr, "Failed to create image\n") ;
  imgLines.drawLine(0,0,63,47) ;
  imgLines.drawLine(63,0,0,47) ;
  imgLines.drawLine(31,47,0,23) ;
  imgLines.drawLine(31,47,63,23) ;
  if (!oled.writeImage(imgLines,SDD1306OLED::overwrite)) fprintf(stderr, "Failed to call writeImage for OLED\n") ;
  oled.display() ;

  printf("Sleeping...\n") ;
  pi.milliSleep(5000) ; // keep display on for 5 sec

  printf ("Displaying text...\n") ;
  DisplayFont fnt, fnt2 ;
  int fontfile = 0, fontfile2 = 0;
  char szStr[] = "6x9 font" ;
  char szStr2[] = "9x15" ;
  if ((fontfile=open("fonts/6x9.fnt", O_RDONLY)) < 0 ||
      (fontfile2=open("fonts/9x15.fnt", O_RDONLY)) < 0){
    fprintf(stdout, "Cannot open font file, skipping this test\n") ;
  }else{
    if (!fnt.loadFile(fontfile)){
      fprintf(stderr, "Failed to load 6x9 font file\n") ;
    }
    if (!fnt2.loadFile(fontfile2)){
      fprintf(stderr, "Failed to load 4x6 font file\n") ;
    }
    DisplayImage *txtimg = fnt.createText(szStr) ;
    if (!txtimg) fprintf(stderr, "failed to create 6x9 font image\n") ;
    if (!oled.writeImage(*txtimg,SDD1306OLED::overwrite,10,10)) fprintf(stderr, "Failed to call writeImage for OLED\n") ;
    delete txtimg ;
    txtimg = fnt2.createText(szStr2) ;
    if (!txtimg) fprintf(stderr, "failed to create 4x6 font image\n") ;
    if (!oled.writeImage(*txtimg,SDD1306OLED::overlay,15,30)) fprintf(stderr, "Failed to call writeImage for OLED\n") ;
    
    oled.display() ;
  }
  
  printf("Sleeping...\n") ;
  pi.milliSleep(5000) ; // keep display on for 5 sec
  
  printf("Text and line mixing...\n") ;
  // Reuse font from above
  if (fontfile2 < 0){
    fprintf(stderr, "Cannot open font file for test, skipping this test\n") ;
  }else{
    // Open all images for display

    char strPercent[5] ;
    DisplayImage imgBar ;
    
    imgBar.createImage(64,48,1);
    imgBar.drawRect(5,5,54,34) ;
    
    for (int percent=0; percent <= 100; percent++){

      imgBar.drawRect(7,7,(50*percent)/100,30, true) ;
      oled.writeImage(imgBar,SDD1306OLED::overwrite);

      // Overlay with text progress
      sprintf(strPercent, "%d%%", percent) ;
      DisplayImage *txtimg = fnt2.createText(strPercent) ;
      oled.writeImage(*txtimg, SDD1306OLED::exclusive, 20,16) ;
      delete txtimg ;
      oled.display() ;
      pi.milliSleep(100) ;
    }
  }

  printf("Sleeping...\n") ;
  pi.milliSleep(5000) ; // keep display on for 5 sec

  DisplayImage img ;
  if (!img.loadXBM(Xbitmap_width, Xbitmap_height, Xbitmap_bits)) fprintf(stderr, "Failed to load XBM image from resource\n") ;

  printf("Load XBM resource test...\n");
  if (!oled.writeImage(img,SDD1306OLED::overwrite)){
    fprintf(stderr, "Cannot display XBM\n") ;
  }

  if (!oled.display()){
    fprintf(stderr, "Display failed\n") ;
  }
 
  printf("Sleeping...\n") ;
  pi.milliSleep(5000) ; // keep display on for 5 sec

  
  printf("Loading test binary image...\n") ;
  int testfile = 0;
  if ((testfile=open("images/gear1.bin", O_RDONLY)) < 0){
    fprintf(stdout, "Cannot open test file, skipping this test\n") ;
  }else{
    if (!img.loadFile(testfile)){
      fprintf(stderr, "Failed to load test file into oled image\n") ;
    }
    if (!oled.writeImage(img, SDD1306OLED::overwrite)) fprintf(stderr, "Cannot write image to OLED object\n");
    if (!oled.display()) fprintf(stderr, "OLED display write failed\n") ;
  }
  
  printf("Sleeping...\n") ;
  pi.milliSleep(5000) ; // keep display on for 5 sec

  printf("Animating display...\n") ;
  int testfile2 = 0;
  if ((testfile2=open("images/gear2.bin", O_RDONLY)) < 0 || testfile < 0){
    fprintf(stdout, "Cannot open test files, skipping this test\n") ;
  }else{
    DisplayImage img2 ;
    if (!img2.loadFile(testfile2)) fprintf(stderr, "Failed to load test file 2 into image object\n") ;
    for (int i=0; i < 25; i++){ // iterate 25 loops of animation
      if (!oled.writeImage(img2, SDD1306OLED::overwrite)){
	fprintf(stderr, "Failed to write image 2 to OLED object\n") ;
	break ;
      }
      if (!oled.display()) fprintf(stderr, "OLED display write failed\n") ;
      pi.milliSleep(200) ;
      if (!oled.writeImage(img, SDD1306OLED::overwrite)){
	fprintf(stderr, "Failed to write image 1 to OLED object\n") ;
	break ;
      }
      if (!oled.display()) fprintf(stderr, "OLED display write failed\n") ;
      pi.milliSleep(200) ;
    }
  }

  oled.turnOff() ; // Turn off the display. 

  return 1 ;
}
