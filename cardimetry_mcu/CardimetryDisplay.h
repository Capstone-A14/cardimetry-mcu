#ifndef __CARDIMETRY_DISPLAY_H__
#define __CARDIMETRY_DISPLAY_H__

#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <SD.h>
#include <FS.h>
#include "Adafruit_GFX.h"
#include "Adafruit_FT6206.h"
#include "TFT_eSPI.h"
#include "TJpg_Decoder.h"
#include "ArduinoJson.h"

#define CARDIMETRY_DISPLAY_TFT_CS   5
#define CARDIMETRY_DISPLAY_TFT_DC   17
#define CARDIMETRY_DISPLAY_TFT_RST  -1
#define CARDIMETRY_DISPLAY_TFT_LED  33
#define CARDIMETRY_DISPLAY_CTP_SENS 40

#define CARDIMETRY_DISPLAY_ROTATION 1
#define CARDIMETRY_DISPLAY_WIDTH    480
#define CARDIMETRY_DISPLAY_HEIGHT   320          

#define CARDIMETRY_DISPLAY_SLEEP_MS 20000

#define CARDIMETRY_DISPLAY_SD_CS              16
#define CARDIMETRY_DISPLAY_SD_INIT_TIMEOUT_MS 3000

#define CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH "/assets/cm_loadscreen.jpg"
#define CARDIMETRY_DISPLAY_MAINMENU_BG_PATH   "/assets/cm_mainmenu.jpg"

#define CARDIMETRY_DISPLAY_LOADSCREEN_SD_READ               0
#define CARDIMETRY_DISPLAY_LOADSCREEN_SD_FAILED             1
#define CARDIMETRY_DISPLAY_LOADSCREEN_SD_INIT               2
#define CARDIMETRY_DISPLAY_LOADSCREEN_SD_CONFIG             3
#define CARDIMETRY_DISPLAY_LOADSCREEN_SD_NOBOOTLOADER       4
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN             5
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_FAILED      6
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_SUCCESS     7
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT          8
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_PASSWORD         9
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT_FAILED   10
#define CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT_SUCCESS  11
#define CARDIMETRY_DISPLAY_LOADSCREEN_SENSOR_INIT           13
#define CARDIMETRY_DISPLAY_LOADSCREEN_SENSOR_INIT_FAILED    14
#define CARDIMETRY_DISPLAY_LOADSCREEN_SENSOR_INIT_SUCCESS   15
#define CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS               300
#define CARDIMETRY_DISPLAY_LOADSCREEN_KEYBOARD_WAIT_MS      500

#define CARDIMETRY_DISPLAY_MAIN_MENU  16
#define CARDIMETRY_DISPLAY_SETTINGS   17
#define CARDIMETRY_DISPLAY_START      18

#define CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED  99

#define CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW  0
#define CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP   1
#define CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1 2
#define CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2 3
#define CARDIMETRY_DISPLAY_KEYBOARD_BACK      0
#define CARDIMETRY_DISPLAY_KEYBOARD_OK        1
#define CARDIMETRY_DISPLAY_KEYBOARD_YET       2

#define CARDIMETRY_DISPLAY_FONT_THIN    1
#define CARDIMETRY_DISPLAY_FONT_LIGHT   2
#define CARDIMETRY_DISPLAY_FONT_REGULAR 3
#define CARDIMETRY_DISPLAY_FONT_MEDIUM  4
#define CARDIMETRY_DISPLAY_FONT_BOLD    5

#define CARDIMETRY_DISPLAY_MAIN_MENU_ACTION_YET       0
#define CARDIMETRY_DISPLAY_MAIN_MENU_ACTION_SETTINGS  1
#define CARDIMETRY_DISPLAY_MAIN_MENU_ACTION_START     2


namespace cardimetry{


  class CardimetryDisplay{

    private:

      String chara_low[26] = {
        "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
        "a", "s", "d", "f", "g", "h", "j", "k", "l",
        "z", "x", "c", "v", "b", "n", "m"
      },
      chara_up[26] = {
        "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
        "A", "S", "D", "F", "G", "H", "J", "K", "L",
        "Z", "X", "C", "V", "B", "N", "M"
      },
      chara_sym1[27] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
        "@", "#", ";", ".", "&", "-", "+", "(", ")", "/",
        "'", "\"", "_", ":", ";", "!", "?"
      },
      chara_sym2[27] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
        "*", "$", "=", "|", "<", ">", "~", "[", "]", "`",
        "{", "}", "\\", ":", ";", "!", "?"
      };

      uint8_t wifi_selected = CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED,
              keyboard_mode = CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW,
              last_mode     = CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW;
      String  keyboard_buf  = "";
      bool    init_info_bar = true;

      void drawKeyboard(uint8_t mode);
      static bool drawJPG(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);

    public:
      TFT_eSPI        tft  = TFT_eSPI();
      Adafruit_FT6206 ctp  = Adafruit_FT6206();

      int16_t touch_x, touch_y;
      bool    is_touched;

      CardimetryDisplay();
      ~CardimetryDisplay();
      void begin();
      void getTouch();
      void sleepCount();

      String fontFile(uint16_t style, uint16_t size, bool italic);

      bool checkBootloader(fs::FS &fs);
      bool configFileExist(fs::FS &fs);
      bool createConfigFile(fs::FS &fs, String uid);

      void drawWiFiList(int16_t num, String ssid[], int16_t rssi[], String enc[]);
      void actionWiFiList(int16_t num, String ssid[], int16_t rssi[], String enc[]);
      uint8_t getSelectedWiFi();
      bool isWiFiTableExist(fs::FS &fs, String* buf, String ssid);

      void showKeyboardInput();
      void actionKeyboardInput();
      uint8_t getKeyboardInput(String* keyboard_buf);

      void initInfoBar();
      void drawInfoBar(uint16_t time_hr, uint16_t time_mnt, int16_t rssi, int16_t bat_perc);
      uint8_t actionMainMenu();

      void drawSettings();
      uint8_t actionSettings();
  };
}


#endif