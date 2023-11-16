#include <SPI.h>
#include <TFT_eSPI.h>
#include <PNGdec.h>
#include "CardimetryLogo.h"
#include "CardimetryImg1.h"
#include "CardimetryImg2.h"
#include "CardimetryImg3.h"
#include "CardimetryImg4.h"
#include "CardimetryImg5.h"

#define MAX_IMAGE_WIDTH 480

PNG       png;
TFT_eSPI  tft   = TFT_eSPI();
int16_t   xpos  = 0;
int16_t   ypos  = 0;

void pngDraw(PNGDRAW* pDraw);


void setup() {
  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);

  tft.begin();
  tft.fillScreen(TFT_WHITE);
  tft.setRotation(1);
}


void loop() {
  /* Logo */
  int16_t rc = png.openFLASH((uint8_t *)CardimetryLogo, sizeof(CardimetryLogo), pngDraw);
  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
  }
  delay(3000);

  /* Slide 1 */
  rc = png.openFLASH((uint8_t *)CardimetryImg1, sizeof(CardimetryImg1), pngDraw);
  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
  }
  delay(3000);

  /* Slide 2 */
  rc = png.openFLASH((uint8_t *)CardimetryImg2, sizeof(CardimetryImg2), pngDraw);
  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
  }
  delay(3000);

  /* Slide 3 */
  rc = png.openFLASH((uint8_t *)CardimetryImg3, sizeof(CardimetryImg3), pngDraw);
  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
  }
  delay(3000);

  /* Slide 4 */
  rc = png.openFLASH((uint8_t *)CardimetryImg4, sizeof(CardimetryImg4), pngDraw);
  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
  }
  delay(3000);

  /* Slide 5 */
  rc = png.openFLASH((uint8_t *)CardimetryImg5, sizeof(CardimetryImg5), pngDraw);
  if (rc == PNG_SUCCESS) {
    tft.startWrite();
    rc = png.decode(NULL, 0);
    tft.endWrite();
  }
  delay(3000);
}


void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}