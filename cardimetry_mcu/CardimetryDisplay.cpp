#include "CardimetryDisplay.h"




cardimetry::CardimetryDisplay::CardimetryDisplay() {}
cardimetry::CardimetryDisplay::~CardimetryDisplay() {}




void cardimetry::CardimetryDisplay::begin() {

  /* Pinmoding nessecary pins */
  pinMode(CARDIMETRY_DISPLAY_TFT_LED, OUTPUT);


  /* Start TFT display */
  this->tft.begin();
  this->tft.setRotation(CARDIMETRY_DISPLAY_ROTATION);
  this->tft.fillScreen(0xFFFF);


  /* Start capcitive touch */
  this->ctp.begin(CARDIMETRY_DISPLAY_CTP_SENS, &Wire);
}




void cardimetry::CardimetryDisplay::getTouch() {
  this->is_touched = this->ctp.touched();
  if(this->is_touched) {
    TS_Point tp   = this->ctp.getPoint();
    this->touch_x = tp.y;
    this->touch_y = CARDIMETRY_DISPLAY_HEIGHT - tp.x;
  }
  else {
    this->touch_x = 0;
    this->touch_y = 0;
  }  
}




bool cardimetry::CardimetryDisplay::configFileExist(fs::FS &fs) {
  return fs.exists(F("/cmconfig.cfg"));
}




bool cardimetry::CardimetryDisplay::createConfigFile(fs::FS &fs) {
  File config_file = fs.open(F("/cmconfig.cfg"), FILE_WRITE);

  if(config_file) {
    /* Close file */
    config_file.close();
    return true;
  }
  
  else {
    return false;
  }
}




void cardimetry::CardimetryDisplay::drawWiFiList(int16_t num, String ssid[], int16_t rssi[], String enc[]) {

  /* Draw column header */
  this->tft.fillRect(10, 50, 460, 38, 0x0000);
  this->tft.setTextColor(0xFFFF, 0x0000);
  this->tft.setTextSize(2);
  this->tft.drawCentreString(F("No."), 35, 60, 1);
  this->tft.drawCentreString(F("SSID"), 180, 60, 1);
  this->tft.drawCentreString(F("RSSI"), 330, 60, 1);
  this->tft.drawCentreString(F("ENC."), 420, 60, 1);


  /* Draw table entries */
  for(uint8_t i = 0; i < (uint8_t)min(7, (int)num); ++i) {

    /* Draw a rectangle */
    this->tft.drawRect(10, 88 + 31*i, 460, 31, 0x7BCF);

    /* Show number */
    this->tft.setTextColor(0x0000, 0xFFFF);
    this->tft.drawCentreString(String(i + 1), 35, 97 + 31*i, 1);
    
    /* Set text color based on RSSI */
    if(rssi[i] >= -75)      this->tft.setTextColor(0x2660);
    else if(rssi[i] >= -90) this->tft.setTextColor(0xD6A0);
    else                    this->tft.setTextColor(0xF000);

    /* Print SSID */
    if(ssid[i].length() > 17) {
      this->tft.drawCentreString(ssid[i].substring(0, 14) + String("..."), 180, 97 + 31*i, 1);
    }
    else {
      this->tft.drawCentreString(ssid[i], 180, 97 + 31*i, 1);
    }

    /* Print RSSI */
    this->tft.drawCentreString(String(rssi[i]), 330, 97 + 31*i, 1);

    /* Print ENC */
    this->tft.drawCentreString(enc[i], 420, 97 + 31*i, 1);
  }

  /* Draw outer rectangle*/
  this->tft.drawRect(10, 50, 460, 256, 0x0000);
}




void cardimetry::CardimetryDisplay::actionWiFiList(int16_t num, String ssid[], int16_t rssi[], String enc[]) {

  if(this->is_touched) {
    
    /* Evaluate only y-touch value */
    for(uint8_t i = 0; i < (uint8_t)min(7, (int)num); ++i) {

      if((88 + 31*i) <= this->touch_y && this->touch_y < (88 + 31*(i + 1))) {
        
        if(this->wifi_selected == CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED) {
          
          /* Select the wifi */
          this->wifi_selected = i;

          /* Draw button to proceed */
          this->tft.fillRoundRect(375, 9, 85, 35, 5, 0x0000);
          this->tft.setTextColor(0xFFFF, 0x0000);
          this->tft.setTextSize(2);
          this->tft.drawCentreString(F("OK >"), 423, 20, 1);
        }

        else if(this->wifi_selected != i) {
        
          /* Return the previous selection */
          this->tft.fillRect(10, 88 + 31*this->wifi_selected, 460, 31, 0xFFFF);
          this->tft.drawRect(10, 88 + 31*this->wifi_selected, 460, 31, 0x7BCF);
          this->tft.setTextColor(0x0000, 0xFFFF);
          this->tft.setTextSize(2);
          this->tft.drawCentreString(String(this->wifi_selected + 1), 35, 97 + 31*this->wifi_selected, 1);

          if(rssi[this->wifi_selected] >= -75)      this->tft.setTextColor(0x2660);
          else if(rssi[this->wifi_selected] >= -90) this->tft.setTextColor(0xD6A0);
          else                                      this->tft.setTextColor(0xF000);

          if(ssid[this->wifi_selected].length() > 17) {
            this->tft.drawCentreString(ssid[this->wifi_selected].substring(0, 14) + String("..."), 180, 97 + 31*this->wifi_selected, 1);
          }
          else {
            this->tft.drawCentreString(ssid[this->wifi_selected], 180, 97 + 31*this->wifi_selected, 1);
          }

          this->tft.drawCentreString(String(rssi[this->wifi_selected]), 330, 97 + 31*this->wifi_selected, 1);
          this->tft.drawCentreString(enc[this->wifi_selected], 420, 97 + 31*this->wifi_selected, 1);

          /* Select the wifi */
          this->wifi_selected = i;
        }

        /* Add touched effect */
        this->tft.fillRect(10, 88 + 31*i, 460, 31, 0x6B4D);
        this->tft.setTextColor(0xFFFF, 0x6B4D);
        this->tft.drawCentreString(String(i + 1), 35, 97 + 31*i, 1);
        if(ssid[i].length() > 17) {
          this->tft.drawCentreString(ssid[i].substring(0, 14) + String("..."), 180, 97 + 31*i, 1);
        }
        else {
          this->tft.drawCentreString(ssid[i], 180, 97 + 31*i, 1);
        }
        this->tft.drawCentreString(String(rssi[i]), 330, 97 + 31*i, 1);
        this->tft.drawCentreString(enc[i], 420, 97 + 31*i, 1);
        this->tft.drawRect(10, 50, 460, 256, 0x0000);

        break;
      }
    }
  }
}




uint8_t cardimetry::CardimetryDisplay::getSelectedWiFi() {
  /* If a wifi is selected, check for activity on the button */
  if(this->wifi_selected != 99 && 375 <= this->touch_x && this->touch_x <= 460 && 9 <= this->touch_y && this->touch_y <= 44) {
    return this->wifi_selected;
  }
  else {
    return CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED;
  }
}




void cardimetry::CardimetryDisplay::showKeyboardInput() {

}




void cardimetry::CardimetryDisplay::actionKeyboardInput() {

}




String cardimetry::CardimetryDisplay::getKeyboardInput() {
  return String("");
}