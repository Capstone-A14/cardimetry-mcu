#include "CardimetryDisplay.h"




cardimetry::CardimetryDisplay::CardimetryDisplay() {}
cardimetry::CardimetryDisplay::~CardimetryDisplay() {}




void cardimetry::CardimetryDisplay::begin() {

  /* Pinmoding nessecary pins */
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




void cardimetry::CardimetryDisplay::sleepCount(bool sleep_able) {
  static uint64_t  last_touch_ms;
  static bool      is_sleep = false,
                   is_init  = true;

  if(sleep_able) {
    if(is_init) {
      last_touch_ms = millis();
      is_init = false;
    }
    if(this->is_touched) {
      last_touch_ms = millis();
      if(is_sleep) {
        digitalWrite(CARDIMETRY_DISPLAY_TFT_LED, HIGH);
        this->is_touched  = false;
        this->touch_x     = 0;
        this->touch_y     = 0;
        is_sleep          = false;
      }
    }
    else {
      if(millis() - last_touch_ms >= CARDIMETRY_DISPLAY_SLEEP_MS) {
        if(!is_sleep) {
          digitalWrite(CARDIMETRY_DISPLAY_TFT_LED, LOW);
          is_sleep = true;
        }
      }
    }
  }

  else {
    is_init = true;
  }
}




String cardimetry::CardimetryDisplay::fontFile(uint16_t style, uint16_t size, bool italic) {
  String ret = "fonts/Poppins";

  switch(style) {
    case CARDIMETRY_DISPLAY_FONT_THIN:
      ret += "Thin";
      break;

    case CARDIMETRY_DISPLAY_FONT_LIGHT:
      ret += "Light";
      break;

    case CARDIMETRY_DISPLAY_FONT_REGULAR:
      ret += "Regular";
      break;

    case CARDIMETRY_DISPLAY_FONT_MEDIUM:
      ret += "Medium";
      break;

    case CARDIMETRY_DISPLAY_FONT_BOLD:
      ret += "SemiBold";
      break;
  }

  if(italic) {
    ret += "Italic";
  }

  ret += String(size);
  return ret;
}




bool cardimetry::CardimetryDisplay::checkBootloader(fs::FS &fs) {
  return (fs.exists(F("/assets")) && fs.exists(F("/data")) && fs.exists(F("/fonts")));
}




bool cardimetry::CardimetryDisplay::configFileExist(fs::FS &fs) {
  return fs.exists(F("/cmconfig.json"));
}




bool cardimetry::CardimetryDisplay::createConfigFile(fs::FS &fs, String uid) {
  if(fs.exists(F("/cmconfig.json"))) fs.remove(F("/cmconfig.json"));
  File cmconfig = fs.open(F("/cmconfig.json"), FILE_WRITE);

  if(cmconfig) {
    /* Create JSON file */
    DynamicJsonDocument json_doc(256);
    json_doc["mac_address"] = String(WiFi.macAddress());
    json_doc["uid"] = uid;
    String json_str;
    serializeJson(json_doc, json_str);

    /* Write file */
    cmconfig.println(json_str);

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


  /* Draw button to proceed */
  this->tft.fillRoundRect(20, 9, 85, 35, 5, 0x0000);
  this->tft.setTextColor(0xFFFF, 0x0000);
  this->tft.setTextSize(2);
  this->tft.drawCentreString(F("< SKIP"), 61, 20, 1);


  /* Draw column header */
  this->tft.fillRect(10, 50, 460, 38, 0x0000);
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


  /* Draw outer rectangle */
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

  else if(20 <= this->touch_x && this->touch_x <= 105 && 9 <= this->touch_y && this->touch_y <= 44) {
    return CARDIMETRY_DISPLAY_WIFI_SKIP;
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




void cardimetry::CardimetryDisplay::initPatientList() {
  this->patient_selected    = CARDIMETRY_DISPLAY_PATIENT_NOT_SELECTED;
  this->init_patient_list   = true;
  this->patient_offset      = 0;
}




void cardimetry::CardimetryDisplay::drawPatientList(uint16_t num, uint16_t id[], String name[], uint8_t offset) {

  /* Reset */
  if(this->init_patient_list) {

    /* Back button */
    this->tft.fillRoundRect(20, 9, 85, 35, 5, 0x0000);
    this->tft.setTextColor(0xFFFF, 0x0000);
    this->tft.setTextSize(2);
    this->tft.drawCentreString(F("< BACK"), 61, 20, 1);

    /* Draw column header */
    this->tft.fillRect(40, 50, 430, 38, 0x0000);
    this->tft.drawCentreString(F("ID"), 80, 60, 1);
    this->tft.drawCentreString(F("Name"), 310, 60, 1);

    /* Draw up and down arrow */
    this->tft.fillRect(10, 50, 30, 30, 0x39C7);
    this->tft.fillRect(10, 276, 30, 30, 0x39C7);
    this->tft.setTextColor(0xFFFF, 0x39C7);
    this->tft.drawCentreString(F("V"), 26, 287, 1);
    this->tft.setTextSize(3);
    this->tft.drawCentreString(F("^"), 26, 67, 1);

    /* Calculate the partition */
    this->patient_bar_part  = num/7 + ((num%7 > 0) ? 1 : 0);
    this->patient_bar_h     = 194/this->patient_bar_part;

    /* Turn off init */
    this->init_patient_list = false;
  }

  /* Draw scrollbar */
  this->tft.fillRect(10, 80, 30, 196, 0x0000);
  this->tft.drawRect(10, 80, 30, 196, 0x52AA);
  this->tft.fillRect(11, 81 + (offset/7)*this->patient_bar_h, 28, 194/this->patient_bar_part, 0x52AA);

  /* Draw table entries */
  for(uint8_t i = 0; i < (uint8_t)min((int)7, (int)(num - offset)); ++i) {

    /* Draw a rectangle */
    this->tft.fillRect(40, 88 + 31*i, 430, 31, 0xFFFF);
    this->tft.drawRect(40, 88 + 31*i, 430, 31, 0x7BCF);

    /* Show number */
    this->tft.setTextColor(0x0000, 0xFFFF);
    this->tft.setTextSize(2);
    this->tft.drawCentreString(String(id[offset + i]), 80, 97 + 31*i, 1);
    
    /* Show name */
    if(name[offset + i].length() > 25)
      this->tft.drawCentreString(name[offset + i].substring(0, 22) + String("..."), 310, 97 + 31*i, 1);
    else
      this->tft.drawCentreString(name[offset + i], 310, 97 + 31*i, 1);
  }

  /* Draw outer rectangle */
  this->tft.drawRect(40, 50, 430, 256, 0x0000);
}




void cardimetry::CardimetryDisplay::actionPatientList(uint16_t num, uint16_t id[], String name[]) {

  if(this->is_touched) {

    if(10 <= this->touch_x && this->touch_x <= 40) {

      if(50 <= this->touch_y && this->touch_y <= 80) {
        this->patient_offset = (this->patient_offset >= 7) ? this->patient_offset - 7 : 0; 
      }

      else if(276 <= this->touch_y && this->touch_y <= 306) {
        this->patient_offset = (this->patient_offset == this->patient_bar_part) ? this->patient_bar_part : this->patient_offset + 7;
      }

      else if(80 <= this->touch_y && this->touch_y <= 276) {
        for(uint16_t i = 0; i < this->patient_bar_part; ++i) {
          if(80 + i*this->patient_bar_h <= this->touch_y && this->touch_y <= 80 + i*this->patient_bar_h + 195/this->patient_bar_part) {
            this->patient_offset = i*7;
            break;
          }
        }
      }

      this->drawPatientList(num, id, name, this->patient_offset);
    }

    else if(40 <= this->touch_x && this->touch_x <= 470) {

      /* Evaluate only y-touch value */
      for(uint8_t i = 0; i < (uint8_t)min(7, (int)num); ++i) {

        if((88 + 31*i) <= this->touch_y && this->touch_y < (88 + 31*(i + 1))) {
          
          if(this->patient_selected == CARDIMETRY_DISPLAY_PATIENT_NOT_SELECTED) {
            
            /* Select the patient */
            this->patient_selected = this->patient_offset + i;

            /* Draw button to proceed */
            this->tft.fillRoundRect(375, 9, 85, 35, 5, 0x0000);
            this->tft.setTextColor(0xFFFF, 0x0000);
            this->tft.setTextSize(2);
            this->tft.drawCentreString(F("OK >"), 423, 20, 1);
          }

          else if(this->patient_selected != i) {
          
            /* Return the previous selection */
            this->tft.fillRect(40, 88 + 31*(this->patient_selected % 7), 430, 31, 0xFFFF);
            this->tft.drawRect(40, 88 + 31*(this->patient_selected % 7), 430, 31, 0x7BCF);
            this->tft.setTextColor(0x0000, 0xFFFF);
            this->tft.setTextSize(2);
            this->tft.drawCentreString(String(id[this->patient_selected]), 80, 97 + 31*(this->patient_selected%7), 1);

            /* Show name */
            if(name[this->patient_selected].length() > 25)
              this->tft.drawCentreString(name[this->patient_selected].substring(0, 22) + String("..."), 310, 97 + 31*(this->patient_selected%7), 1);
            else
              this->tft.drawCentreString(name[this->patient_selected], 310, 97 + 31*(this->patient_selected%7), 1);

            /* Select the patient */
            this->patient_selected = this->patient_offset + i;
          }

          /* Add touched effect */
          this->tft.fillRect(40, 88 + 31*i, 430, 31, 0x6B4D);
          this->tft.setTextColor(0xFFFF, 0x6B4D);
          this->tft.drawCentreString(String(id[this->patient_selected]), 80, 97 + 31*i, 1);

          /* Show name */
          if(name[this->patient_selected].length() > 25)
            this->tft.drawCentreString(name[this->patient_selected].substring(0, 22) + String("..."), 310, 97 + 31*i, 1);
          else
            this->tft.drawCentreString(name[this->patient_selected], 310, 97 + 31*i, 1);
            
          /* Outer rectangle */
          this->tft.drawRect(40, 50, 430, 256, 0x0000);

          break;
        }
      }
    }
  }
}




uint8_t cardimetry::CardimetryDisplay::getSelectedPatient() {

  if(this->is_touched) {

    if(9 <= this->touch_y && this->touch_y <= 44) {

      if(20 <= this->touch_x && this->touch_x <= 105) {
        return CARDIMETRY_DISPLAY_PATIENT_BACK;
      }

      else if(375 <= this->touch_x && this->touch_x <= 460) {
        return this->patient_selected;
      }
    }
  }

  return CARDIMETRY_DISPLAY_PATIENT_NOT_SELECTED;
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
  static bool     keyboard_redraw = false,
                  input_update    = false,
                  accept_touch    = true;
  
  if(this->is_touched && accept_touch) {

    if(120 <= this->touch_y && this->touch_y < 170) {

      for(uint8_t i = 0; i < 10; ++i) {

        if(48*i <= this->touch_x && this->touch_x < 48*(i + 1)) {
          
          if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW) {
            this->keyboard_buf += this->chara_low[i];
          }

          else if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP) {
            this->keyboard_buf += this->chara_up[i];
          }

          else if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1) {
            this->keyboard_buf += this->chara_sym1[i];
          }

          else if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) {
            this->keyboard_buf += this->chara_sym2[i];
          }

          input_update = true;
        }
      }
    }

    else if(170 <= this->touch_y && this->touch_y < 220) {

      if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1 || this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) {
        
        for(uint8_t i = 0; i < 10; ++i) {

          if(48*i <= this->touch_x && this->touch_x < 48*(i + 1)) {

            if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1) {
              this->keyboard_buf += this->chara_sym1[i + 10];
            }

            else if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) {
              this->keyboard_buf += this->chara_sym2[i + 10];
            }

            input_update = true;
          }
        }
      }

      else {

        for(uint8_t i = 0; i < 9; ++i) {

          if(24 + 48*i <= this->touch_x && this->touch_x < 24 + 48*(i + 1)) {

            if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW) {
              this->keyboard_buf += this->chara_low[i + 10];
            }

            else if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP) {
              this->keyboard_buf += this->chara_up[i + 10];
            }

            input_update = true;
          }
        }
      }
    }

    else if(220 <= this->touch_y && this->touch_y < 270) {

      if(0 <= this->touch_x && this->touch_x < 72) {
        this->last_mode = this->keyboard_mode;
        if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW) {
          this->keyboard_mode = CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP;
        }
        else if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP) {
          this->keyboard_mode = CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW;
        }
        else if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1) {
          this->keyboard_mode = CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2;
        }
        else if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) {
          this->keyboard_mode = CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1;
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

            if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW) {
              this->keyboard_buf += this->chara_low[i + 19];
            }

            else if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_UP) {
              this->keyboard_buf += this->chara_up[i + 19];
            }

            else if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1) {
              this->keyboard_buf += this->chara_sym1[i + 20];
            }

            else if(this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) {
              this->keyboard_buf += this->chara_sym2[i + 20];
            }

            input_update = true;
          }
        }
      }
    }

    else if(270 <= this->touch_y && this->touch_y < 320) {
      
      if(0 <= this->touch_x && this->touch_x < 72) {
        this->last_mode       = this->keyboard_mode;
        this->keyboard_mode   = (this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1 || this->keyboard_mode == CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM2) ? CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW : CARDIMETRY_DISPLAY_KEYBOARD_MODE_SYM1;
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
    this->drawKeyboard(this->keyboard_mode);
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

    /* Reset */
    this->keyboard_mode = CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW;
    this->last_mode     = CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW;

    *keyboard_buf = this->keyboard_buf;
    this->keyboard_buf.remove(0, this->keyboard_buf.length());
    return CARDIMETRY_DISPLAY_KEYBOARD_OK;
  }

  else if((9 <= this->touch_y && this->touch_y < 44) && (20 <= this->touch_x && this->touch_x < 105)) {

    /* Reset */
    this->keyboard_mode = CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW;
    this->last_mode     = CARDIMETRY_DISPLAY_KEYBOARD_MODE_LOW;

    this->keyboard_buf.remove(0, this->keyboard_buf.length());
    return CARDIMETRY_DISPLAY_KEYBOARD_BACK;
  }

  else {
    return CARDIMETRY_DISPLAY_KEYBOARD_YET;
  }
}




void cardimetry::CardimetryDisplay::initInfoBar() {
  this->init_info_bar = true;
}




void cardimetry::CardimetryDisplay::drawInfoBar(uint16_t time_hr, uint16_t time_mnt, int16_t rssi, int16_t bat_perc) {
  static uint16_t last_mnt,
                  last_hr,
                  last_sig;
  static int16_t  last_bat;

  /* Determine signal level */
  uint16_t sig;
  if(rssi > -70)          sig = 4;
  else if(rssi > -85)     sig = 3;
  else if(rssi > -100)    sig = 2;
  else if(rssi > -115)    sig = 1;
  else if(rssi == -9999)  sig = 69; 
  else                    sig = 0;

  /* Determine battery level */
  int16_t bat = (int16_t)(((float)bat_perc)*0.56);

  if(this->init_info_bar) {
    /* Clear */
    this->tft.fillSmoothRoundRect(160, 0, 380, 40, 15, 0x018C, 0xFFFF);
    this->tft.setTextColor(0xFFFF, 0x018C);
    this->tft.loadFont(this->fontFile(CARDIMETRY_DISPLAY_FONT_LIGHT, 22, false), SD);

    /* Draw time */
    this->tft.setCursor(185, 11);
    this->tft.printf("%02d:%02d WIB", time_hr, time_mnt);

    /* Unload font to create space on RAM */
    this->tft.unloadFont();

    /* Draw signal strength */
    if(sig == 4) {
      this->tft.fillRect(340, 25, 10, 5, 0xFFFF);
      this->tft.fillRect(353, 20, 10, 10, 0xFFFF);
      this->tft.fillRect(366, 15, 10, 15, 0xFFFF);
      this->tft.fillRect(379, 10, 10, 20, 0xFFFF);
    }
    else if(sig == 3) {
      this->tft.fillRect(340, 25, 10, 5, 0xFFFF);
      this->tft.fillRect(353, 20, 10, 10, 0xFFFF);
      this->tft.fillRect(366, 15, 10, 15, 0xFFFF);
      this->tft.drawRect(379, 10, 10, 20, 0xFFFF);
    }
    else if(sig == 2) {
      this->tft.fillRect(340, 25, 10, 5, 0xFFFF);
      this->tft.fillRect(353, 20, 10, 10, 0xFFFF);
      this->tft.drawRect(366, 15, 10, 15, 0xFFFF);
      this->tft.drawRect(379, 10, 10, 20, 0xFFFF);
    }
    else if(sig == 1) {
      this->tft.fillRect(340, 25, 10, 5, 0xFFFF);
      this->tft.drawRect(353, 20, 10, 10, 0xFFFF);
      this->tft.drawRect(366, 15, 10, 15, 0xFFFF);
      this->tft.drawRect(379, 10, 10, 20, 0xFFFF);
    }
    else if(sig == 0) {
      this->tft.drawRect(340, 25, 10, 5, 0xFFFF);
      this->tft.drawRect(353, 20, 10, 10, 0xFFFF);
      this->tft.drawRect(366, 15, 10, 15, 0xFFFF);
      this->tft.drawRect(379, 10, 10, 20, 0xFFFF);
    }
    else if(sig == 69) {
      this->tft.drawRect(340, 25, 10, 5, 0xFFFF);
      this->tft.drawRect(353, 20, 10, 10, 0xFFFF);
      this->tft.drawRect(366, 15, 10, 15, 0xFFFF);
      this->tft.drawRect(379, 10, 10, 20, 0xFFFF);
      this->tft.setTextColor(0xE800);
      this->tft.setTextSize(3);
      this->tft.drawCentreString("X", 378, 8, 1);
    }

    /* Draw battery percentage */
    this->tft.fillRect(405, 10, 60, 20, 0xFFFF);
    this->tft.fillRect(465, 15, 5, 10, 0xFFFF);
    if(bat > 10)  this->tft.fillRect(407, 12, bat, 16, 0x46C0);
    else          this->tft.fillRect(407, 12, bat, 16, 0xE800);

    /* Save values as last values */
    this->init_info_bar = false;
    last_mnt            = time_mnt;
    last_hr             = time_hr;
    last_sig            = sig;
    last_bat            = bat;
  }

  if(last_hr != time_hr || last_mnt != time_mnt) {
    /* Reset */
    this->tft.fillRect(180, 5, 140, 30, 0x018C);

    /* Draw time */
    this->tft.loadFont(this->fontFile(CARDIMETRY_DISPLAY_FONT_LIGHT, 22, false), SD);
    this->tft.setTextColor(0xFFFF, 0x018C);
    this->tft.setCursor(185, 11);
    this->tft.printf("%02d:%02d WIB", time_hr, time_mnt);
    this->tft.unloadFont();

    /* Update last value */
    last_mnt = time_mnt;
    last_hr  = time_hr;
  }

  if(last_sig != sig) {
    /* Reset */
    this->tft.fillRect(339, 5, 50, 30, 0x018C);

    /* Draw signal strength */
    if(sig == 4) {
      this->tft.fillRect(340, 25, 10, 5, 0xFFFF);
      this->tft.fillRect(353, 20, 10, 10, 0xFFFF);
      this->tft.fillRect(366, 15, 10, 15, 0xFFFF);
      this->tft.fillRect(379, 10, 10, 20, 0xFFFF);
    }
    else if(sig == 3) {
      this->tft.fillRect(340, 25, 10, 5, 0xFFFF);
      this->tft.fillRect(353, 20, 10, 10, 0xFFFF);
      this->tft.fillRect(366, 15, 10, 15, 0xFFFF);
      this->tft.drawRect(379, 10, 10, 20, 0xFFFF);
    }
    else if(sig == 2) {
      this->tft.fillRect(340, 25, 10, 5, 0xFFFF);
      this->tft.fillRect(353, 20, 10, 10, 0xFFFF);
      this->tft.drawRect(366, 15, 10, 15, 0xFFFF);
      this->tft.drawRect(379, 10, 10, 20, 0xFFFF);
    }
    else if(sig == 1) {
      this->tft.fillRect(340, 25, 10, 5, 0xFFFF);
      this->tft.drawRect(353, 20, 10, 10, 0xFFFF);
      this->tft.drawRect(366, 15, 10, 15, 0xFFFF);
      this->tft.drawRect(379, 10, 10, 20, 0xFFFF);
    }
    else if(sig == 0) {
      this->tft.drawRect(340, 25, 10, 5, 0xFFFF);
      this->tft.drawRect(353, 20, 10, 10, 0xFFFF);
      this->tft.drawRect(366, 15, 10, 15, 0xFFFF);
      this->tft.drawRect(379, 10, 10, 20, 0xFFFF);
    }
    else if(sig == 69) {
      this->tft.drawRect(340, 25, 10, 5, 0xFFFF);
      this->tft.drawRect(353, 20, 10, 10, 0xFFFF);
      this->tft.drawRect(366, 15, 10, 15, 0xFFFF);
      this->tft.drawRect(379, 10, 10, 20, 0xFFFF);
      this->tft.setTextColor(0xE800);
      this->tft.setTextSize(3);
      this->tft.drawCentreString("X", 378, 8, 1);
    }

    /* Update last value */
    last_sig = sig;
  }

  if(last_bat != bat) {
    /* Draw battery percentage */
    this->tft.fillRect(405, 10, 60, 20, 0xFFFF);
    this->tft.fillRect(465, 15, 5, 10, 0xFFFF);
    if(bat > 10)  this->tft.fillRect(407, 12, bat, 16, 0x46C0);
    else          this->tft.fillRect(407, 12, bat, 16, 0xE800);

    /* Update last value */
    last_bat = bat;
  }
}




uint8_t cardimetry::CardimetryDisplay::actionMainMenu() {

  /* Check for input */
  if(this->is_touched) {

    if(80 <= this->touch_y && this->touch_y <= 280) {
      
      if(40 <= this->touch_x && this->touch_x <= 220) {
        return CARDIMETRY_DISPLAY_MAIN_MENU_ACTION_START;
      }

      else if(260 <= this->touch_x && this->touch_x <= 440) {
        return CARDIMETRY_DISPLAY_MAIN_MENU_ACTION_SETTINGS;
      }
    }
  }

  return CARDIMETRY_DISPLAY_MAIN_MENU_ACTION_YET;
}




void cardimetry::CardimetryDisplay::drawSettings() {

  /* Draw the button boxes */
  this->tft.fillSmoothRoundRect(200, 80, 240, 60, 15, 0x19AA, 0xFFFF);
  this->tft.fillSmoothRoundRect(200, 150, 240, 60, 15, 0x19AA, 0xFFFF);
  this->tft.fillSmoothRoundRect(200, 220, 240, 60, 15, 0x19AA, 0xFFFF);
  this->tft.fillSmoothRoundRect(40, 150, 120, 60, 15, 0x0000, 0xFFFF);

  /* Print text */
  this->tft.setTextColor(0xFFFF, 0x018C);
  this->tft.loadFont(this->fontFile(CARDIMETRY_DISPLAY_FONT_REGULAR, 22, false), SD);
  this->tft.setCursor(225, 101);
  this->tft.print(F("Connect to WiFi"));
  this->tft.setCursor(225, 171);
  this->tft.print(F("Change UID"));
  this->tft.setCursor(225, 241);
  this->tft.print(F("Restart Device"));
  this->tft.setCursor(57, 171);
  this->tft.print(F("< Back"));
  this->tft.unloadFont();
}




uint8_t cardimetry::CardimetryDisplay::actionSettings() {
  
  /* Check for touch region */
  if(this->is_touched) {

    if(40 <= this->touch_x && this->touch_x <= 160) {

      if(150 <= this->touch_y && this->touch_x <= 210) {
        return CARDIMETRY_DISPLAY_SETTINGS_ACTION_BACK;
      }
    }

    else if(200 <= this->touch_x && this->touch_x <= 440) {

      if(80 <= this->touch_y && this->touch_y <= 140) {
        return CARDIMETRY_DISPLAY_SETTINGS_ACTION_WIFI_CONNECT;
      }

      else if(150 <= this->touch_y && this->touch_y <= 210) {
        return CARDIMETRY_DISPLAY_SETTINGS_ACTION_CHANGE_UID;
      }

      else if(220 <= this->touch_y && this->touch_y <= 280) {
        return CARDIMETRY_DISPLAY_SETTINGS_ACTION_RESTART;
      }
    }
  }

  return CARDIMETRY_DISPLAY_SETTINGS_ACTION_YET;
}




void cardimetry::CardimetryDisplay::drawStart() {

  /* Draw the button boxes */
  this->tft.fillSmoothRoundRect(200, 80, 240, 60, 15, 0x19AA, 0xFFFF);
  this->tft.fillSmoothRoundRect(200, 150, 240, 60, 15, 0x19AA, 0xFFFF);
  this->tft.fillSmoothRoundRect(200, 220, 240, 60, 15, 0x19AA, 0xFFFF);
  this->tft.fillSmoothRoundRect(40, 150, 120, 60, 15, 0x0000, 0xFFFF);

  /* Print text */
  this->tft.setTextColor(0xFFFF, 0x018C);
  this->tft.loadFont(this->fontFile(CARDIMETRY_DISPLAY_FONT_REGULAR, 22, false), SD);
  this->tft.setCursor(225, 101);
  this->tft.print(F("Start Publish"));
  this->tft.setCursor(225, 171);
  this->tft.print(F("ECG Test"));
  this->tft.setCursor(225, 241);
  this->tft.print(F("IMU Test"));
  this->tft.setCursor(57, 171);
  this->tft.print(F("< Back"));
  this->tft.unloadFont();
}




uint8_t cardimetry::CardimetryDisplay::actionStart() {

  /* Check for touch region */
  if(this->is_touched) {

    if(40 <= this->touch_x && this->touch_x <= 160) {

      if(150 <= this->touch_y && this->touch_x <= 210) {
        return CARDIMETRY_DISPLAY_START_ACTION_BACK;
      }
    }

    else if(200 <= this->touch_x && this->touch_x <= 440) {

      if(80 <= this->touch_y && this->touch_y <= 140) {
        return CARDIMETRY_DISPLAY_START_ACTION_START_PUBLISH;
      }

      else if(150 <= this->touch_y && this->touch_y <= 210) {
        return CARDIMETRY_DISPLAY_START_ACTION_ECG_TEST;
      }

      else if(220 <= this->touch_y && this->touch_y <= 280) {
        return CARDIMETRY_DISPLAY_START_ACTION_IMU_TEST;
      }
    }
  }

  return CARDIMETRY_DISPLAY_START_ACTION_YET;
}




void cardimetry::CardimetryDisplay::drawEcgTest() {
  
  /* Reset */
  this->init_ecg_test = true;

  /* Back button */
  this->tft.fillSmoothRoundRect(9, 9, 135, 35, 10, 0x0000, 0xFFFF);
  this->tft.fillSmoothRoundRect(164, 9, 307, 35, 10, 0x19AA, 0xFFFF);
  this->tft.setTextColor(0xFFFF, 0x0000);
  this->tft.loadFont(this->fontFile(CARDIMETRY_DISPLAY_FONT_REGULAR, 20, false), SD);
  this->tft.drawCentreString(F("< BACK"), 66, 19, 1);
  this->tft.unloadFont();
  this->tft.loadFont(this->fontFile(CARDIMETRY_DISPLAY_FONT_REGULAR, 16, false), SD);
  this->tft.drawCentreString(F("ECG TEST PLOT (25 mm/s)"), 320, 20, 1);
  this->tft.unloadFont();

  /* Axis values */
  this->tft.setTextColor(0x0000, 0xFFFF);
  this->tft.loadFont(this->fontFile(CARDIMETRY_DISPLAY_FONT_REGULAR, 8, false), SD);

  uint16_t axis_time_ms = 0;
  for(uint16_t i = 10; i < 470; i += 50) {
    this->tft.drawCentreString(String(axis_time_ms), i, 312, 1);
    axis_time_ms += 200;
  }

  this->tft.unloadFont();

  /* Draw the plot */
  this->drawEcgPlot();
}




void cardimetry::CardimetryDisplay::ecgPlot(uint64_t* timestamp, int32_t* lead1, int32_t* lead2, int32_t* lead3) {

  /* Variables holding coordinates */
  static uint64_t zero_ts;
  static uint16_t current_x, last_x,
                  current_y1, last_y1,
                  current_y2, last_y2,
                  current_y3, last_y3;

  /* Initiate */
  if(this->init_ecg_test) {

    zero_ts = timestamp[0];
    
    last_x  = this->convertEcgTs2Plot(0);

    last_y1 = this->convertEcgLead2Plot(lead1[0] - 724);
    last_y2 = this->convertEcgLead2Plot(lead2[0] - 78);
    last_y3 = this->convertEcgLead2Plot(lead3[0] - 670);
  }

  for(uint8_t i = 0; i < 50; ++i) {

    /* If init, forget the zero */
    if(this->init_ecg_test) {
      this->init_ecg_test = false;
      continue;
    }

    /* Determine current value*/
    current_x   = this->convertEcgTs2Plot(timestamp[i] - zero_ts);
    current_y1  = this->convertEcgLead2Plot(lead1[i] - 724);
    current_y2  = this->convertEcgLead2Plot(lead2[i] - 78);
    current_y3  = this->convertEcgLead2Plot(lead3[i] - 670);
    // Serial.println(lead1[i]);
    // Serial.println(lead2[i]);
    // Serial.println(lead3[i]);

    /* If overflow */
    if(current_x > 470) {
      if(i > 0) --i;
      zero_ts = timestamp[i];
      last_x  = this->convertEcgTs2Plot(0);
      this->drawEcgPlot();
      continue;
    }

    /* Draw */
    this->tft.drawLine(
      last_x, last_y1,
      current_x, current_y1,
      0xE000
    );
    this->tft.drawLine(
      last_x, last_y2,
      current_x, current_y2,
      0x0543
    );
    this->tft.drawLine(
      last_x, last_y3,
      current_x, current_y3,
      0x5015
    );

    /* Switch */
    last_x  = current_x;
    last_y1 = current_y1;
    last_y2 = current_y2;
    last_y3 = current_y3;
  }
}




uint8_t cardimetry::CardimetryDisplay::actionEcgTest() {
  
  if(this->is_touched) {

    if((9 <= this->touch_x && this->touch_x <= 144) && (9 <= this->touch_y && this->touch_y <= 44)) {
      return CARDIMETRY_DISPLAY_ECG_TEST_ACTION_BACK;
    }  
  }

  return CARDIMETRY_DISPLAY_ECG_TEST_ACTION_YET;
}




void cardimetry::CardimetryDisplay::drawImuTest() {
  /* Reset */
  this->init_imu_test = true;

  /* Back button */
  this->tft.fillSmoothRoundRect(9, 9, 135, 35, 10, 0x0000, 0xFFFF);
  this->tft.fillSmoothRoundRect(164, 9, 307, 35, 10, 0x19AA, 0xFFFF);
  this->tft.setTextColor(0xFFFF, 0x0000);
  this->tft.loadFont(this->fontFile(CARDIMETRY_DISPLAY_FONT_REGULAR, 20, false), SD);
  this->tft.drawCentreString(F("< BACK"), 66, 19, 1);
  this->tft.unloadFont();
  this->tft.loadFont(this->fontFile(CARDIMETRY_DISPLAY_FONT_REGULAR, 16, false), SD);
  this->tft.drawCentreString(F("IMU TEST PLOT"), 320, 20, 1);
  this->tft.unloadFont();

  /* Axis values */
  this->tft.setTextColor(0x0000, 0xFFFF);
  this->tft.loadFont(this->fontFile(CARDIMETRY_DISPLAY_FONT_REGULAR, 8, false), SD);

  uint16_t axis_time_ms = 0;
  for(uint16_t i = 10; i < 470; i += 50) {
    this->tft.drawCentreString(String(axis_time_ms), i, 312, 1);
    axis_time_ms += 200;
  }

  this->tft.unloadFont();

  /* Draw the plot */
  this->drawImuPlot();
}




void cardimetry::CardimetryDisplay::imuPlot(uint64_t* timestamp, float* qw, float* qx, float* qy, float* qz) {
  
  /* Variables holding coordinates */
  static uint64_t zero_ts;
  static uint16_t current_x, last_x,
                  current_y1, last_y1,
                  current_y2, last_y2,
                  current_y3, last_y3,
                  current_y4, last_y4;

  /* Initiate */
  if(this->init_imu_test) {

    zero_ts = timestamp[0];
    
    last_x  = this->convertImuTs2Plot(0);

    last_y1 = this->convertImuData2Plot(qw[0]);
    last_y2 = this->convertImuData2Plot(qx[0]);
    last_y3 = this->convertImuData2Plot(qy[0]);
    last_y4 = this->convertImuData2Plot(qz[0]);
  }

  for(uint8_t i = 0; i < 25; ++i) {

    /* If init, forget the zero */
    if(this->init_imu_test) {
      this->init_imu_test = false;
      continue;
    }

    /* Determine current value */
    current_x   = this->convertImuTs2Plot(timestamp[i] - zero_ts);
    current_y1  = this->convertImuData2Plot(qw[i]);
    current_y2  = this->convertImuData2Plot(qx[i]);
    current_y3  = this->convertImuData2Plot(qy[i]);
    current_y4  = this->convertImuData2Plot(qz[i]);

    /* If overflow */
    if(current_x > 470) {
      if(i > 0) --i;
      zero_ts = timestamp[i];
      last_x  = this->convertImuTs2Plot(0);
      this->drawImuPlot();
      continue;
    }

    /* Draw */
    this->tft.drawLine(
      last_x, last_y1,
      current_x, current_y1,
      0xF800
    );
    this->tft.drawLine(
      last_x, last_y2,
      current_x, current_y2,
      0xE540
    );
    this->tft.drawLine(
      last_x, last_y3,
      current_x, current_y3,
      0x06A3
    );
    this->tft.drawLine(
      last_x, last_y4,
      current_x, current_y4,
      0x201C
    );

    /* Switch */
    last_x  = current_x;
    last_y1 = current_y1;
    last_y2 = current_y2;
    last_y3 = current_y3;
    last_y4 = current_y4;
  }
}




uint8_t cardimetry::CardimetryDisplay::actionImuTest() {

  if(this->is_touched) {

    if((9 <= this->touch_x && this->touch_x <= 144) && (9 <= this->touch_y && this->touch_y <= 44)) {
      return CARDIMETRY_DISPLAY_IMU_TEST_ACTION_BACK;
    }  
  }

  return CARDIMETRY_DISPLAY_IMU_TEST_ACTION_YET;
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




void cardimetry::CardimetryDisplay::drawEcgPlot() {

  /* Draw axis */
  this->tft.fillRect(10, 60, 460, 250, 0xFFFF);
  this->tft.drawRect(9, 59, 462, 252, 0xFEDB);
  this->tft.drawRect(10, 60, 460, 250, 0xFEDB);
  this->tft.drawRect(11, 61, 458, 248, 0xFEDB);
  
  uint8_t bold_cnt = 0;
  for(uint16_t i = 20; i < 470; i += 10) {
    
    /* Bold for every 5 */
    bold_cnt = (bold_cnt + 1)%5;
    if(bold_cnt == 0) {
      this->tft.drawFastVLine(i - 1, 60, 250, 0xFEDB);
      this->tft.drawFastVLine(i, 60, 250, 0xFEDB);
      this->tft.drawFastVLine(i + 1, 60, 250, 0xFEDB);
    }
    else {
      this->tft.drawFastVLine(i, 60, 250, 0xFEDB);
    }
  }

  bold_cnt = 0;
  for(uint16_t i = 300; i > 60; i -= 10) {
    
    /* Bold for every 5 */
    bold_cnt = (bold_cnt + 1)%5;
    if(bold_cnt == 0) {
      this->tft.drawFastHLine(10, i - 1, 460, 0xFEDB);
      this->tft.drawFastHLine(10, i, 460, 0xFEDB);
      this->tft.drawFastHLine(10, i + 1, 460, 0xFEDB);
    }
    else {
      this->tft.drawFastHLine(10, i, 460, 0xFEDB);
    }
  }
}




void cardimetry::CardimetryDisplay::drawImuPlot() {
  /* Draw axis */
  this->tft.fillRect(10, 60, 460, 250, 0xFFFF);
  this->tft.drawRect(9, 59, 462, 252, 0xC618);
  this->tft.drawRect(10, 60, 460, 250, 0xC618);
  this->tft.drawRect(11, 61, 458, 248, 0xC618);
  
  uint8_t bold_cnt = 0;
  for(uint16_t i = 20; i < 470; i += 10) {
    
    /* Bold for every 5 */
    bold_cnt = (bold_cnt + 1)%5;
    if(bold_cnt == 0) {
      this->tft.drawFastVLine(i - 1, 60, 250, 0xC618);
      this->tft.drawFastVLine(i, 60, 250, 0xC618);
      this->tft.drawFastVLine(i + 1, 60, 250, 0xC618);
    }
    else {
      this->tft.drawFastVLine(i, 60, 250, 0xC618);
    }
  }

  bold_cnt = 0;
  for(uint16_t i = 300; i > 60; i -= 10) {
    
    /* Bold for every 5 */
    bold_cnt = (bold_cnt + 1)%5;
    if(bold_cnt == 0) {
      this->tft.drawFastHLine(10, i - 1, 460, 0xC618);
      this->tft.drawFastHLine(10, i, 460, 0xC618);
      this->tft.drawFastHLine(10, i + 1, 460, 0xC618);
    }
    else {
      this->tft.drawFastHLine(10, i, 460, 0xC618);
    }
  }
}




uint16_t cardimetry::CardimetryDisplay::convertEcgTs2Plot(uint64_t time) {
  return 10 + ((uint16_t)time)/4;
}




uint16_t cardimetry::CardimetryDisplay::convertEcgLead2Plot(int32_t lead) {
  int32_t res = lead;
  if(res < 0) {
    res = 0;
  }
  else if (res > 250) {
    res = 250;
  }
  return 310 - (uint16_t)res;
}




uint16_t cardimetry::CardimetryDisplay::convertImuTs2Plot(uint64_t time) {
  return 10 + ((uint16_t)time)/4;
}




uint16_t cardimetry::CardimetryDisplay::convertImuData2Plot(float q) {
  float res = q*250;
  if(res < 0) {
    res = 0;
  }
  else if (res > 250) {
    res = 250;
  }
  return 310 - (uint16_t)res;
}




bool cardimetry::CardimetryDisplay::drawJPG(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  TFT_eSPI tftemp  = TFT_eSPI();
  tftemp.setRotation(CARDIMETRY_DISPLAY_ROTATION);
  tftemp.setSwapBytes(true);
  if (y >= CARDIMETRY_DISPLAY_HEIGHT || x >= CARDIMETRY_DISPLAY_WIDTH) return 0;
  tftemp.pushImage(x, y, w, h, bitmap);
  return true;
}