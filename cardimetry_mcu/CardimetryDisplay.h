#ifndef __CARDIMETRY_DISPLAY_H__
#define __CARDIMETRY_DISPLAY_H__

#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <FS.h>
#include "Adafruit_GFX.h"
#include "Adafruit_FT6206.h"
#include "TFT_eSPI.h"

#define CARDIMETRY_DISPLAY_TFT_CS   5
#define CARDIMETRY_DISPLAY_TFT_DC   17
#define CARDIMETRY_DISPLAY_TFT_RST  -1
#define CARDIMETRY_DISPLAY_TFT_LED  33
#define CARDIMETRY_DISPLAY_CTP_SENS 40

#define CARDIMETRY_DISPLAY_ROTATION 1
#define CARDIMETRY_DISPLAY_WIDTH    480
#define CARDIMETRY_DISPLAY_HEIGHT   320          

#define CARDIMETRY_DISPLAY_SD_CS              16
#define CARDIMETRY_DISPLAY_SD_INIT_TIMEOUT_MS 3000

#define CARDIMETRY_DISPLAY_LOADSCREEN_SD_READ               0
#define CARDIMETRY_DISPLAY_LOADSCREEN_SD_FAILED             1
#define CARDIMETRY_DISPLAY_LOADSCREEN_SD_INIT               2
#define CARDIMETRY_DISPLAY_LOADSCREEN_SD_CONFIG             3
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN             4
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_FAILED      5
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_SUCCESS     6
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT          7
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_PASSWORD         8
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT_FAILED   9
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT_SUCCESS  10
#define CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS               300

#define CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED  99


namespace cardimetry{

  class CardimetryDisplay{

    private:
      uint8_t wifi_selected = CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED;

    public:
      TFT_eSPI        tft  = TFT_eSPI();
      Adafruit_FT6206 ctp  = Adafruit_FT6206();

      int16_t touch_x, touch_y;
      bool    is_touched;

      CardimetryDisplay();
      ~CardimetryDisplay();
      void begin();
      void getTouch();

      bool configFileExist(fs::FS &fs);
      bool createConfigFile(fs::FS &fs);

      void drawWiFiList(int16_t num, String ssid[], int16_t rssi[], String enc[]);
      void actionWiFiList(int16_t num, String ssid[], int16_t rssi[], String enc[]);
      uint8_t getSelectedWiFi();

      void showKeyboardInput();
      void actionKeyboardInput();
      String getKeyboardInput();
  };
}


#endif