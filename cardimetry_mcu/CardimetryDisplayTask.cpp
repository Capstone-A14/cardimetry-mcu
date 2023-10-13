#include "Cardimetry.h"



void cardimetry::cardimetry_display_task(void* pvParameters) {
  
  /* Local variables */
  uint8_t   task_state        = CARDIMETRY_DISPLAY_LOADSCREEN_SD_READ,
            task_req          = CARDIMETRY_DISPLAY_REQ_NONE,
            out_req           = 0;
  bool      screen_refresh    = true,
            stamp_timer_free  = true;
  uint64_t  stamp_timer;
  uint8_t   selected_wifi_buf = CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED;
  String    keyboard_buf;


  /* Initiate display */
  cardimetry::CardimetryDisplay cm_display = cardimetry::CardimetryDisplay();
  cm_display.begin();


  /* Loop */
  while(true) {

    /* Read capacitive touch */
    cm_display.getTouch();

    
    /* Read incoming request from another task */
    xQueueReceive(cardimetry::cardimetry_display_req_queue, &task_req, pdMS_TO_TICKS(CARDIMETRY_TASK_REQ_WAIT_MS));
    switch(task_req) {

      case CARDIMETRY_DISPLAY_REQ_NONE:
        break;


      case CARDIMETRY_DISPLAY_REQ_WIFI_CONNECT_SUCCESS:
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT_SUCCESS;
        screen_refresh  = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_WIFI_CONNECT_FAILED:
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT_FAILED;
        screen_refresh  = true;
        break;
    }
    task_req = CARDIMETRY_DISPLAY_REQ_NONE;


    /* Task states */
    switch(task_state) {

      case CARDIMETRY_DISPLAY_LOADSCREEN_SD_READ:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Reading SD card..."), 240, 280, 1);
          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }

        /* Check for SD card till timeout */
        if(stamp_timer_free) {
          stamp_timer       = millis();
          stamp_timer_free  = false;
        }

        xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);

        if(!SD.begin(CARDIMETRY_DISPLAY_SD_CS)) {
          if(millis() - stamp_timer > CARDIMETRY_DISPLAY_SD_INIT_TIMEOUT_MS) {
            task_state        = CARDIMETRY_DISPLAY_LOADSCREEN_SD_FAILED;
            screen_refresh    = true;
            stamp_timer_free  = true;
          }
        }

        else {
          /* Check for existence of config file inside the SD card */
          if(cm_display.configFileExist(SD)) {
            task_state = CARDIMETRY_DISPLAY_LOADSCREEN_SD_CONFIG;
          }
          else {
            task_state = CARDIMETRY_DISPLAY_LOADSCREEN_SD_INIT;
          }
          screen_refresh    = true;
          stamp_timer_free  = true;
        }

        xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_SD_FAILED:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Failed to read SD card."), 240, 265, 1);
          screen_refresh = false;
        }

        /* Countdown 5 seconds */
        cm_display.tft.drawCentreString(F("Will try again in 5s..."), 240, 290, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        cm_display.tft.drawCentreString(F("Will try again in 4s..."), 240, 290, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        cm_display.tft.drawCentreString(F("Will try again in 3s..."), 240, 290, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        cm_display.tft.drawCentreString(F("Will try again in 2s..."), 240, 290, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        cm_display.tft.drawCentreString(F("Will try again in 1s..."), 240, 290, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        /* Jump back to SD card reading */
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_SD_READ;
        screen_refresh  = true;
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_SD_INIT:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Configuring SD Card..."), 240, 280, 1);
          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }

        /* Creating configuration file inside SD card */
        xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);

        if(cm_display.createConfigFile(SD)) {
          task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN;
          screen_refresh  = true;
        }

        else {
          task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_SD_READ;
          screen_refresh  = true;
        }

        xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_SD_CONFIG:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Reading config file..."), 240, 280, 1);
          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }

        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN;
        screen_refresh  = true;
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Scanning WiFi AP(s)..."), 240, 280, 1);

          /* Send request to conn task to do WiFi scan */
          out_req = CARDIMETRY_CONN_REQ_WIFI_SCAN;
          xQueueSend(cardimetry::cardimetry_conn_req_queue, &out_req, portMAX_DELAY);

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }

        /* Receive number of wifi scanned */
        xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);

        if(cardimetry::cardimetry_conn_wifi_scanned_num == 0) {
          task_state     = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_FAILED;
          screen_refresh = true;
        }

        else if(cardimetry::cardimetry_conn_wifi_scanned_num > 0) {
          task_state     = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_SUCCESS;
          screen_refresh = true;
        }

        xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_FAILED:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Failed to find any WiFi AP."), 240, 265, 1);
          screen_refresh = false;
        }

        /* Countdown 3s */
        cm_display.tft.drawCentreString(F("Will try again in 3s..."), 240, 290, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        cm_display.tft.drawCentreString(F("Will try again in 2s..."), 240, 290, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        cm_display.tft.drawCentreString(F("Will try again in 1s..."), 240, 290, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));

        /* Jump back to WiFi scan */
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN;
        screen_refresh  = true;
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_SUCCESS:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Scanned WiFi(s)"), 240, 17, 1);

          /* Print list of WiFis */
          xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
          cm_display.drawWiFiList(
            cardimetry::cardimetry_conn_wifi_scanned_num,
            cardimetry::cardimetry_conn_wifi_scanned_ssid,
            cardimetry::cardimetry_conn_wifi_scanned_rssi,
            cardimetry::cardimetry_conn_wifi_scanned_enc
          );
          xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);
          
          screen_refresh = false;
        }

        /* Receive touch and do action */
        xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
        cm_display.actionWiFiList(
          cardimetry::cardimetry_conn_wifi_scanned_num,
          cardimetry::cardimetry_conn_wifi_scanned_ssid,
          cardimetry::cardimetry_conn_wifi_scanned_rssi,
          cardimetry::cardimetry_conn_wifi_scanned_enc
        );
        xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);

        /* Establish a WiFi connection */
        selected_wifi_buf = cm_display.getSelectedWiFi();
        if(selected_wifi_buf != CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED) {

          xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          cardimetry::cardimetry_conn_wifi_selected = selected_wifi_buf;

          if(cardimetry::cardimetry_conn_wifi_scanned_enc[selected_wifi_buf] == "Open") {
            cardimetry::cardimetry_conn_wifi_selected_pass = "YEET69";
            task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT;
            screen_refresh  = true;
          }
          else if(cm_display.isWiFiTableExist(SD, &keyboard_buf, cardimetry::cardimetry_conn_wifi_scanned_ssid[selected_wifi_buf])) {
            cardimetry::cardimetry_conn_wifi_selected_pass = keyboard_buf;
            task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT;
            screen_refresh  = true;
          }
          else {
            task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_PASSWORD;
            screen_refresh  = true;
          }
  
          xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
        }
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_PASSWORD:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Insert WiFi Password"), 240, 17, 1);
          cm_display.showKeyboardInput();

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }

        /* Receive touch and do action */
        cm_display.actionKeyboardInput();
        if(cm_display.getKeyboardInput(&keyboard_buf) == CARDIMETRY_DISPLAY_KEYBOARD_OK) {
          xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
          cardimetry::cardimetry_conn_wifi_selected_pass = keyboard_buf;
          xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);
          
          task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT;
          screen_refresh  = true;
        }
        else if(cm_display.getKeyboardInput(&keyboard_buf) == CARDIMETRY_DISPLAY_KEYBOARD_BACK) {
          selected_wifi_buf = CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED;
          task_state        = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_SUCCESS;
          screen_refresh    = true;
        }
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT:
        
        /* On-screen notif*/
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
          cm_display.tft.drawCentreString(String(F("Connecting to ")) + cardimetry_conn_wifi_scanned_ssid[selected_wifi_buf].substring(0, 17) + String(F("...")), 240, 280, 1);
          WiFi.scanDelete();
          xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);

          /* Request to do a wifi connection */
          out_req = CARDIMETRY_CONN_REQ_WIFI_CONNECT;
          xQueueSend(cardimetry::cardimetry_conn_req_queue, &out_req, portMAX_DELAY);

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT_SUCCESS:
        
        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
          cm_display.tft.drawCentreString(String(F("Successfully connected to ")) + cardimetry_conn_wifi_scanned_ssid[selected_wifi_buf].substring(0, 17), 240, 280, 1);
          xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT_FAILED:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
          cm_display.tft.drawCentreString(String(F("Connection to ")) + cardimetry_conn_wifi_scanned_ssid[selected_wifi_buf].substring(0, 17) + String(F(" failed")), 240, 280, 1);
          xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }

        /* Rescan WiFi */
        task_state        = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN;
        selected_wifi_buf = CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED;
        screen_refresh    = true;
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_DOWNLOAD_DATA:
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_SENSOR_BEGIN:
        break;




      case CARDIMETRY_DISPLAY_MAIN_MENU:
        break;
    }


    /* Task delay, room for another task to run */
    vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_TASK_DELAY));
  }
}