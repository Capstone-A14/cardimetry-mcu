#ifndef __PNG_TEST_H__
#define __PNG_TEST_H__

#include <SPI.h>
#include <SD.h>
#include "TFT_eSPI.h"
#include "PNGdec.h"


TFT_eSPI tft = TFT_eSPI();
PNG png;


void png_show(const char* filename, uint16_t x, uint16_t y) {
  File file = SD.open(filename, FILE_READ);
  
  if(file) {
  
    if(png.begin(file)) {
      int width   = png.width(),
          height  = png.height();

      tft.setSwapBytes(true);

      for(int row = 0; row < height; row++) {
        if(png.drawRow(row)) {
          tft.pushColors(PNG.row_buffer, width);
        }
      }
    }

    png.end();
    file.close();
  }
}


void png_begin() {
  /* Turn ON LED */
  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);

  /* Initiate TFT instance */
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);

  /* Show image */
  png_show("/assets/cm_loadscreen.png", 0, 0);
}


void png_loop() {

}

#endif