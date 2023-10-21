#include "CardimetryDisplay.h"




cardimetry::CardimetryDisplay::CardimetryDisplay() {}
cardimetry::CardimetryDisplay::~CardimetryDisplay() {}




void cardimetry::CardimetryDisplay::begin() {

  /* Pinmoding nessecary pins */
  pinMode(CARDIMETRY_DISPLAY_TFT_LED, OUTPUT);
  pinMode(CARDIMETRY_DISPLAY_TFT_LED, OUTPUT);
  digitalWrite(CARDIMETRY_DISPLAY_TFT_LED, HIGH);


  /* Start TFT display */
  this->tft.begin();
  this->tft.setRotation(CARDIMETRY_DISPLAY_ROTATION);
  this->tft.fillScreen(0xFFFF);
  this->tft.setSwapBytes(true);
  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(this->drawJPG);


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
  return fs.exists(F("/cmconfig.json"));
}




bool cardimetry::CardimetryDisplay::createConfigFile(fs::FS &fs) {
  File cmconfig = fs.open(F("/cmconfig.json"), FILE_WRITE);
  fs.mkdir(F("/assets"));
  fs.mkdir(F("/fonts"));
  fs.mkdir(F("/data"));

  if(cmconfig) {
    /* Close file */
    cmconfig.close();
    return true;
  }
  
  else {
    return false;
  }
}




void cardimetry::CardimetryDisplay::drawWiFiList(int16_t num, String ssid[], int16_t rssi[], String enc[]) {

  /* Reset */
  this->wifi_selected = CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED;

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




bool cardimetry::CardimetryDisplay::isWiFiTableExist(fs::FS &fs, String* buf, String ssid) {
  
  if(fs.exists(F("/cmwifitable.json"))) {
    /* Open the wifi table */
    File cmwifitable = fs.open("/cmwifitable.json", FILE_READ);
    String json_data = cmwifitable.readString();
    cmwifitable.close();

    DynamicJsonDocument json_doc(1024);
    deserializeJson(json_doc, json_data);

    /* Check if the key exists */
    if(json_doc.containsKey(ssid)) {
      *buf = json_doc[ssid].as<String>();
      return true;
    }
    else {
      return false;
    }
  }

  else {
    return false;
  }
} 




void cardimetry::CardimetryDisplay::showKeyboardInput() {
  
  /* Back button */
  this->tft.fillRoundRect(20, 9, 85, 35, 5, 0x0000);
  this->tft.setTextColor(0xFFFF, 0x0000);
  this->tft.setTextSize(2);
  this->tft.drawCentreString(F("< BACK"), 61, 20, 1);


  /* OK button */
  this->tft.fillRoundRect(375, 9, 85, 35, 5, 0x0000);
  this->tft.drawCentreString(F("OK >"), 423, 20, 1);


  /* Show keyboard */
  this->drawKeyboard(CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW);
}




void cardimetry::CardimetryDisplay::actionKeyboardInput() {
  static uint8_t  keyboard_mode   = CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW,
                  last_mode       = CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW,
                  last_idx        = 0;
  static bool     keyboard_redraw = false,
                  input_update    = false,
                  accept_touch    = true;
  
  if(this->is_touched && accept_touch) {

    if(120 <= this->touch_y && this->touch_y < 170) {

      for(uint8_t i = 0; i < 10; ++i) {

        if(48*i <= this->touch_x && this->touch_x < 48*(i + 1)) {
          
          if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW) {
            this->keyboard_buf += this->chara_low[i];
          }

          else if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP) {
            this->keyboard_buf += this->chara_up[i];
          }

          else if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1) {
            this->keyboard_buf += this->chara_sym1[i];
          }

          else if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) {
            this->keyboard_buf += this->chara_sym2[i];
          }

          input_update = true;
        }
      }
    }

    else if(170 <= this->touch_y && this->touch_y < 220) {

      if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1 || keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) {
        
        for(uint8_t i = 0; i < 10; ++i) {

          if(48*i <= this->touch_x && this->touch_x < 48*(i + 1)) {

            if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1) {
              this->keyboard_buf += this->chara_sym1[i + 10];
            }

            else if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) {
              this->keyboard_buf += this->chara_sym2[i + 10];
            }

            input_update = true;
          }
        }
      }

      else {

        for(uint8_t i = 0; i < 9; ++i) {

          if(24 + 48*i <= this->touch_x && this->touch_x < 24 + 48*(i + 1)) {

            if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW) {
              this->keyboard_buf += this->chara_low[i + 10];
            }

            else if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP) {
              this->keyboard_buf += this->chara_up[i + 10];
            }

            input_update = true;
          }
        }
      }
    }

    else if(220 <= this->touch_y && this->touch_y < 270) {

      if(0 <= this->touch_x && this->touch_x < 72) {
        last_mode       = keyboard_mode;
        if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW) {
          keyboard_mode = CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP;
        }
        else if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP) {
          keyboard_mode = CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW;
        }
        else if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1) {
          keyboard_mode = CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2;
        }
        else if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) {
          keyboard_mode = CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1;
        }
        keyboard_redraw = true;
      }

      else if(408 <= this->touch_x && this->touch_x < 480) {
        if(this->keyboard_buf.length() > 0) {
          this->keyboard_buf.remove(this->keyboard_buf.length() - 1, 1);
          input_update = true;
        }
      }

      else {

        for(uint8_t i = 0; i < 7; ++i) {

          if(72 + 48*i <= this->touch_x && this->touch_x < 72 + 48*(i + 1)) {

            if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW) {
              this->keyboard_buf += this->chara_low[i + 19];
            }

            else if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP) {
              this->keyboard_buf += this->chara_up[i + 19];
            }

            else if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1) {
              this->keyboard_buf += this->chara_sym1[i + 20];
            }

            else if(keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) {
              this->keyboard_buf += this->chara_sym2[i + 20];
            }

            input_update = true;
          }
        }
      }
    }

    else if(270 <= this->touch_y && this->touch_y < 320) {
      
      if(0 <= this->touch_x && this->touch_x < 72) {
        last_mode       = keyboard_mode;
        keyboard_mode   = (keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1 || keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) ? CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW : CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1;
        keyboard_redraw = true;
      }

      else if(72 <= this->touch_x && this->touch_x < 408) {
        this->keyboard_buf += " ";
        input_update = true;
      }
    }

    accept_touch = false;
  }

  else if(!this->is_touched) {
    accept_touch = true;
  }

  if(keyboard_redraw) {
    this->drawKeyboard(keyboard_mode);
    input_update = true;
    keyboard_redraw = false;
  }

  if(input_update) {
    this->tft.fillRoundRect(10, 70, 460, 40, 10, 0xFFFF);
    this->tft.setTextColor(0x0000);
    this->tft.setTextSize(2);
    this->tft.drawCentreString(this->keyboard_buf, 240, 83, 1);
    input_update = false;
  }
}




uint8_t cardimetry::CardimetryDisplay::getKeyboardInput(String* keyboard_buf) {
  
  if(((270 <= this->touch_y && this->touch_y < 320) && (408 <= this->touch_x && this->touch_x < 480)) || 
  ((9 <= this->touch_y && this->touch_y < 44) && (375 <= this->touch_x && this->touch_x < 460))) {

    *keyboard_buf = this->keyboard_buf;
    this->keyboard_buf.remove(0, this->keyboard_buf.length());
    return CARDIMETRY_DISPLAY_KEYBOARD_OK;
  }

  else if((9 <= this->touch_y && this->touch_y < 44) && (20 <= this->touch_x && this->touch_x < 105)) {
    this->keyboard_buf.remove(0, this->keyboard_buf.length());
    return CARDIMETRY_DISPLAY_KEYBOARD_BACK;
  }

  else {
    return CARDIMETRY_DISPLAY_KEYBOARD_YET;
  }
}




void cardimetry::CardimetryDisplay::drawKeyboard(uint8_t mode) {

  /* Entry background */
  this->tft.fillRect(0, 60, 480, 60, 0x0000);
  this->tft.fillRoundRect(10, 70, 460, 40, 10, 0xFFFF);

  /* Keyboard background */
  this->tft.fillRect(0, 120, 480, 200, 0x0000);

  /* Draw button layout */
  for(uint8_t i = 0; i < 10; ++i) {
    this->tft.fillRoundRect(4 + 48*i, 124, 42, 42, 8, 0xA514);
  }
  if(mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW || mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP) {
    for(uint8_t i = 0; i < 9; ++i) {
      this->tft.fillRoundRect(28 + 48*i, 174, 42, 42, 8, 0xA514);
    }
  }
  else {
    for(uint8_t i = 0; i < 10; ++i) {
      this->tft.fillRoundRect(4 + 48*i, 174, 42, 42, 8, 0xA514);
    }
  }
  for(uint8_t i = 0; i < 7; ++i) {
    this->tft.fillRoundRect(76 + 48*i, 224, 42, 42, 8, 0xA514);
  }
  this->tft.fillRoundRect(76, 274, 332, 42, 8, 0xA514);
  this->tft.fillRoundRect(4, 224, 66, 42, 8, 0x6B4D);
  this->tft.fillRoundRect(4, 274, 66, 42, 8, 0x6B4D);
  this->tft.fillRoundRect(412, 224, 66, 42, 8, 0x6B4D);
  this->tft.fillRoundRect(412, 274, 66, 42, 8, 0x6B4D);


  this->tft.setTextColor(0xFFFF);
  this->tft.setTextSize(2);
  if(mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW) {
    for(uint8_t i = 0; i < 10; ++i) {
      this->tft.drawCentreString(this->chara_low[i], 25 + 48*i, 137, 1);
    }
    for(uint8_t i = 0; i < 9; ++i) {
      this->tft.drawCentreString(this->chara_low[i + 10], 49 + 48*i, 187, 1);
    }
    for(uint8_t i = 0; i < 7; ++i) {
      this->tft.drawCentreString(this->chara_low[i + 19], 97 + 48*i, 237, 1);
    }
    this->tft.drawCentreString("Caps", 37, 237, 1);
    this->tft.drawCentreString("@", 37, 287, 1);
    this->tft.drawCentreString("<", 443, 237, 1);
    this->tft.drawCentreString("->", 443, 287, 1);
  }


  else if(mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP) {
    for(uint8_t i = 0; i < 10; ++i) {
      this->tft.drawCentreString(this->chara_up[i], 25 + 48*i, 137, 1);
    }
    for(uint8_t i = 0; i < 9; ++i) {
      this->tft.drawCentreString(this->chara_up[i + 10], 49 + 48*i, 187, 1);
    }
    for(uint8_t i = 0; i < 7; ++i) {
      this->tft.drawCentreString(this->chara_up[i + 19], 97 + 48*i, 237, 1);
    }
    this->tft.drawCentreString("Caps", 37, 237, 1);
    this->tft.drawCentreString("@", 37, 287, 1);
    this->tft.drawCentreString("<", 443, 237, 1);
    this->tft.drawCentreString("->", 443, 287, 1);
  }


  else if(mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1) {
    for(uint8_t i = 0; i < 10; ++i) {
      this->tft.drawCentreString(this->chara_sym1[i], 25 + 48*i, 137, 1);
    }
    for(uint8_t i = 0; i < 10; ++i) {
      this->tft.drawCentreString(this->chara_sym1[i + 10], 25 + 48*i, 187, 1);
    }
    for(uint8_t i = 0; i < 7; ++i) {
      this->tft.drawCentreString(this->chara_sym1[i + 20], 97 + 48*i, 237, 1);
    }
    this->tft.drawCentreString("Caps", 37, 237, 1);
    this->tft.drawCentreString("@", 37, 287, 1);
    this->tft.drawCentreString("<", 443, 237, 1);
    this->tft.drawCentreString("->", 443, 287, 1);
  }


  else if(mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) {
    for(uint8_t i = 0; i < 10; ++i) {
      this->tft.drawCentreString(this->chara_sym2[i], 25 + 48*i, 137, 1);
    }
    for(uint8_t i = 0; i < 10; ++i) {
      this->tft.drawCentreString(this->chara_sym2[i + 10], 25 + 48*i, 187, 1);
    }
    for(uint8_t i = 0; i < 7; ++i) {
      this->tft.drawCentreString(this->chara_sym2[i + 20], 97 + 48*i, 237, 1);
    }
    this->tft.drawCentreString("Caps", 37, 237, 1);
    this->tft.drawCentreString("@", 37, 287, 1);
    this->tft.drawCentreString("<", 443, 237, 1);
    this->tft.drawCentreString("->", 443, 287, 1);
  }
}




bool cardimetry::CardimetryDisplay::drawJPG(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  TFT_eSPI tftemp  = TFT_eSPI();
  tftemp.setRotation(CARDIMETRY_DISPLAY_ROTATION);
  tftemp.setSwapBytes(true);
  if (y >= CARDIMETRY_DISPLAY_HEIGHT || x >= CARDIMETRY_DISPLAY_WIDTH) return 0;
  tftemp.pushImage(x, y, w, h, bitmap);
  return true;
}