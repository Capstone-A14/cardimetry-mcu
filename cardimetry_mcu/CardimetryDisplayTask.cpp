#include "Cardimetry.h"



void cardimetry::cardimetry_display_task(void* pvParameters) {
  
  /* Local variables */
  uint8_t   task_state        = CARDIMETRY_DISPLAY_LOADSCREEN_SD_READ,
            task_req          = CARDIMETRY_DISPLAY_REQ_NONE,
            out_req           = 0;
  bool      screen_refresh    = true,
            wifi_table_exist  = false,
            sleep_able        = true,
            in_settings       = false,
            ecg1_plot         = false,
            ecg2_plot         = false,
            imu1_plot         = false,
            imu2_plot         = false,
            stamp_timer_free  = true;
  uint64_t  stamp_timer;
  uint8_t   selected_wifi_buf = CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED;
  String    keyboard_buf;


  /* Initiate display */
  cardimetry::CardimetryDisplay cm_display = cardimetry::CardimetryDisplay();
  cm_display.begin();


  /* Loop */
  while(true) {

    /* Read capacitive touch and count for sleep */
    cm_display.getTouch();
    cm_display.sleepCount(sleep_able);


    /* Read incoming request from another task */
    xQueueReceive(cardimetry::cardimetry_display_req_queue, &task_req, pdMS_TO_TICKS(CARDIMETRY_TASK_REQ_WAIT_MS));
    switch(task_req) {

      case CARDIMETRY_DISPLAY_REQ_NONE:
        break;


      case CARDIMETRY_DISPLAY_REQ_WIFI_SCAN_SUCCESS:
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_SUCCESS;
        screen_refresh  = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_WIFI_SCAN_FAILED:
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_FAILED;
        screen_refresh  = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_WIFI_CONNECT_SUCCESS:
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT_SUCCESS;
        screen_refresh  = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_WIFI_CONNECT_FAILED:
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT_FAILED;
        screen_refresh  = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_SENSOR_INIT_SUCCESS:
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_SENSOR_INIT_SUCCESS;
        screen_refresh  = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_SENSOR_INIT_FAILED:
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_SENSOR_INIT_FAILED;
        screen_refresh  = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_PLOT_ECG1:
        ecg1_plot = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_PLOT_ECG2:
        ecg2_plot = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_PLOT_IMU1:
        imu1_plot = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_PLOT_IMU2:
        imu2_plot = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_PATIENT_SEARCH_SUCCESS:
        task_state      = CARDIMETRY_DISPLAY_PATIENT_LIST;
        screen_refresh  = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_PATIENT_SEARCH_FAILED:
        task_state      = CARDIMETRY_DISPLAY_PATIENT_SEARCH_FAILED;
        screen_refresh  = true;
        break;

      
      case CARDIMETRY_DISPLAY_REQ_ECG_SYNC_SUCCESS:
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN;
        screen_refresh  = true;
        break;


      case CARDIMETRY_DISPLAY_REQ_ECG_SYNC_FAILED:
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_ECG_SYNC_FAILED;
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

          /* Check for booatloader */
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          if(!cm_display.checkBootloader(SD)) {
            task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_SD_NOBOOTLOADER;
            screen_refresh  = true;
            break;
          }
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);

          /* Create unique ID */
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.drawCentreString(F("Insert Unique ID"), 240, 18, 1);
          cm_display.showKeyboardInput();
          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_KEYBOARD_WAIT_MS));
        }

        /* Receive touch and do action */
        cm_display.actionKeyboardInput();

        if(cm_display.getKeyboardInput(&keyboard_buf) == CARDIMETRY_DISPLAY_KEYBOARD_OK) {

          /* Creating configuration file inside SD card */
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);

          if(cm_display.createConfigFile(SD, keyboard_buf)) {
            task_state      = (in_settings) ? CARDIMETRY_DISPLAY_MAIN_MENU : CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN;
            screen_refresh  = true;
          }

          else {
            task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_SD_READ;
            screen_refresh  = true;
          }

          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
        }

        else if(cm_display.getKeyboardInput(&keyboard_buf) == CARDIMETRY_DISPLAY_KEYBOARD_BACK) {
          task_state      = (in_settings) ? CARDIMETRY_DISPLAY_MAIN_MENU : CARDIMETRY_DISPLAY_LOADSCREEN_SD_READ;
          screen_refresh  = true;
        }
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_SD_CONFIG:

        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
          
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Reading config file..."), 240, 280, 1);
          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }

        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_ECG_SYNC_TEST;
        screen_refresh  = true;
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_SD_NOBOOTLOADER:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("No bootloader exists."), 240, 265, 1);
          cm_display.tft.drawCentreString(F("Cardimetry halted."), 240, 290, 1);
          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_ECG_SYNC_TEST:
        
        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
          
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Sync-ing ECG module..."), 240, 280, 1);

          out_req = CARDIMETRY_SENSOR_REQ_ECG_SYNC;
          xQueueSend(cardimetry::cardimetry_sensor_req_queue, &out_req, portMAX_DELAY);

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_ECG_SYNC_FAILED:

        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
          
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Failed to sync ECG module. Turn off"), 240, 265, 1);
          cm_display.tft.drawCentreString(F("the device for 5s and try again."), 240, 290, 1);

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN:

        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
          
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Scanning WiFi AP(s)..."), 240, 280, 1);

          /* Send request to conn task to do WiFi scan */
          out_req = CARDIMETRY_CONN_REQ_WIFI_SCAN;
          xQueueSend(cardimetry::cardimetry_conn_req_queue, &out_req, portMAX_DELAY);

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN_FAILED:

        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
          
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

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
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
            task_state        = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT;
            wifi_table_exist  = true;
            screen_refresh    = true;
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
          cm_display.tft.drawCentreString(F("Insert WiFi Password"), 240, 18, 1);
          cm_display.showKeyboardInput();
          cm_display.is_touched = false;

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_KEYBOARD_WAIT_MS));
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
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
          
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
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
          
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
          cm_display.tft.drawCentreString(String(F("Successfully connected to ")) + cardimetry_conn_wifi_scanned_ssid[selected_wifi_buf].substring(0, 17), 240, 280, 1);
          xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }

        task_state      = (in_settings) ? CARDIMETRY_DISPLAY_MAIN_MENU : CARDIMETRY_DISPLAY_LOADSCREEN_SENSOR_INIT; 
        screen_refresh  = true;
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_CONNECT_FAILED:

        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
          
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
          cm_display.tft.drawCentreString(String(F("Connection to ")) + cardimetry_conn_wifi_scanned_ssid[selected_wifi_buf].substring(0, 17) + String(F(" failed")), 240, 280, 1);
          xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }

        /* Rescan WiFi */
        if(wifi_table_exist) {
          task_state        = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_PASSWORD;
          screen_refresh    = true;
        } 

        else {
          task_state        = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN;
          selected_wifi_buf = CARDIMETRY_DISPLAY_WIFI_NOT_SELECTED;
          screen_refresh    = true;
        }
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_SENSOR_INIT:
        
        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
          
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Initiating sensors..."), 240, 280, 1);

          out_req = CARDIMETRY_SENSOR_REQ_INIT;
          xQueueSend(cardimetry::cardimetry_sensor_req_queue, &out_req, portMAX_DELAY);

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_SENSOR_INIT_SUCCESS:

        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
          
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Sensors initiated successfully"), 240, 280, 1);
          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }

        task_state      = CARDIMETRY_DISPLAY_MAIN_MENU;
        screen_refresh  = true;
        break;




      case CARDIMETRY_DISPLAY_LOADSCREEN_SENSOR_INIT_FAILED:

        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);
          
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Failed to initiate sensors."), 240, 265, 1);
          screen_refresh = false;
        }

        /* Countdown 3s */
        cm_display.tft.drawCentreString(F("Will try again in 3s..."), 240, 290, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        cm_display.tft.drawCentreString(F("Will try again in 2s..."), 240, 290, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        cm_display.tft.drawCentreString(F("Will try again in 1s..."), 240, 290, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));

        /* Jump back to sensor init */
        task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_SENSOR_INIT;
        screen_refresh  = true;
        break;

      
      
      
      case CARDIMETRY_DISPLAY_MAIN_MENU:

        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_MAINMENU_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);

          cm_display.is_touched = false;
          cm_display.initInfoBar();
          screen_refresh = false;
        }


        /* Draw info bar */
        xSemaphoreTake(cardimetry::cardimetry_info_mutex, portMAX_DELAY);
        cm_display.drawInfoBar(
          cardimetry::cardimetry_conn_time_hr,
          cardimetry::cardimetry_conn_time_mnt,
          cardimetry::cardimetry_conn_signal,
          cardimetry::cardimetry_conn_bat_perc
        );
        xSemaphoreGive(cardimetry::cardimetry_info_mutex);


        /* Read action */
        if(cm_display.actionMainMenu() == CARDIMETRY_DISPLAY_MAIN_MENU_ACTION_SETTINGS) {
          task_state      = CARDIMETRY_DISPLAY_SETTINGS; 
          screen_refresh  = true;
        }

        else if(cm_display.actionMainMenu() == CARDIMETRY_DISPLAY_MAIN_MENU_ACTION_START) {
          task_state      = CARDIMETRY_DISPLAY_START;
          screen_refresh  = true;
        }
        
        break;




      case CARDIMETRY_DISPLAY_SETTINGS:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillRect(0, 50, 480, 270, 0xFFFF);
          cm_display.drawSettings();
          cm_display.is_touched = false;
          
          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh  = false;
          in_settings     = true;
        }

        /* Draw info bar */
        xSemaphoreTake(cardimetry::cardimetry_info_mutex, portMAX_DELAY);
        cm_display.drawInfoBar(
          cardimetry::cardimetry_conn_time_hr,
          cardimetry::cardimetry_conn_time_mnt,
          cardimetry::cardimetry_conn_signal,
          cardimetry::cardimetry_conn_bat_perc
        );
        xSemaphoreGive(cardimetry::cardimetry_info_mutex);

        /* Actions */
        if(cm_display.actionSettings() == CARDIMETRY_DISPLAY_SETTINGS_ACTION_WIFI_CONNECT) {
          task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_WIFI_SCAN;
          screen_refresh  = true;
        }

        else if(cm_display.actionSettings() == CARDIMETRY_DISPLAY_SETTINGS_ACTION_CHANGE_UID) {
          task_state      = CARDIMETRY_DISPLAY_LOADSCREEN_SD_INIT;
          screen_refresh  = true;
        }

        else if(cm_display.actionSettings() == CARDIMETRY_DISPLAY_SETTINGS_ACTION_RESTART) {
          esp_restart();
        }

        else if(cm_display.actionSettings() == CARDIMETRY_DISPLAY_SETTINGS_ACTION_BACK) {
          task_state      = CARDIMETRY_DISPLAY_MAIN_MENU;
          screen_refresh  = true;
        }
        break;




      case CARDIMETRY_DISPLAY_START:
        
        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillRect(0, 50, 480, 270, 0xFFFF);
          cm_display.drawStart();
          cm_display.is_touched = false;

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }

        /* Draw info bar */
        xSemaphoreTake(cardimetry::cardimetry_info_mutex, portMAX_DELAY);
        cm_display.drawInfoBar(
          cardimetry::cardimetry_conn_time_hr,
          cardimetry::cardimetry_conn_time_mnt,
          cardimetry::cardimetry_conn_signal,
          cardimetry::cardimetry_conn_bat_perc
        );
        xSemaphoreGive(cardimetry::cardimetry_info_mutex);

        /* Actions */
        if(cm_display.actionStart() == CARDIMETRY_DISPLAY_START_ACTION_START_PUBLISH) {
          task_state      = CARDIMETRY_DISPLAY_PATIENT_NAME_INPUT;
          screen_refresh  = true;
        }

        else if(cm_display.actionStart() == CARDIMETRY_DISPLAY_START_ACTION_ECG_TEST) {
          task_state      = CARDIMETRY_DISPLAY_ECG_TEST;
          screen_refresh  = true;
        }

        else if(cm_display.actionStart() == CARDIMETRY_DISPLAY_START_ACTION_IMU_TEST) {
          /* Send request */
          out_req = CARDIMETRY_SENSOR_REQ_IMU_TEST;
          xQueueSend(cardimetry::cardimetry_sensor_req_queue, &out_req, portMAX_DELAY);

          task_state      = CARDIMETRY_DISPLAY_IMU_TEST;
          screen_refresh  = true;
        }

        else if(cm_display.actionStart() == CARDIMETRY_DISPLAY_SETTINGS_ACTION_BACK) {
          task_state      = CARDIMETRY_DISPLAY_MAIN_MENU;
          screen_refresh  = true;
        }
        break;




      case CARDIMETRY_DISPLAY_ECG_TEST:

        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_spi_mutex, portMAX_DELAY);
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.drawEcgTest();
          xSemaphoreGive(cardimetry::cardimetry_spi_mutex);
          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh  = false;
          sleep_able      = false;

          /* Send request */
          out_req = CARDIMETRY_SENSOR_REQ_ECG_TEST;
          xQueueSend(cardimetry::cardimetry_sensor_req_queue, &out_req, portMAX_DELAY);
        }


        /* Plot ECG */
        if(ecg1_plot) {
          xSemaphoreTake(cardimetry::cardimetry_spi_mutex, portMAX_DELAY);
          cm_display.ecgPlot(
            cardimetry::cardimetry_sensor_ecg_ts_qt1,
            cardimetry::cardimetry_sensor_ecg_lead1_qt1,
            cardimetry::cardimetry_sensor_ecg_lead2_qt1,
            cardimetry::cardimetry_sensor_ecg_lead3_qt1
          );
          xSemaphoreGive(cardimetry::cardimetry_spi_mutex);
          ecg1_plot = false;
        }

        if(ecg2_plot) {
          xSemaphoreTake(cardimetry::cardimetry_spi_mutex, portMAX_DELAY);
          cm_display.ecgPlot(
            cardimetry::cardimetry_sensor_ecg_ts_qt2,
            cardimetry::cardimetry_sensor_ecg_lead1_qt2,
            cardimetry::cardimetry_sensor_ecg_lead2_qt2,
            cardimetry::cardimetry_sensor_ecg_lead3_qt2
          );
          xSemaphoreGive(cardimetry::cardimetry_spi_mutex);
          ecg2_plot   = false;
        }


        /* Get action */
        if(cm_display.actionEcgTest() == CARDIMETRY_DISPLAY_ECG_TEST_ACTION_BACK) {
          out_req = CARDIMETRY_SENSOR_REQ_ECG_HALT;
          xQueueSend(cardimetry::cardimetry_sensor_req_queue, &out_req, portMAX_DELAY);

          task_state      = CARDIMETRY_DISPLAY_MAIN_MENU;
          screen_refresh  = true;
          sleep_able      = true;
        }
        break;




      case CARDIMETRY_DISPLAY_IMU_TEST:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.drawImuTest();
          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh  = false;
          sleep_able      = false;

          /* Send request */
          out_req = CARDIMETRY_SENSOR_REQ_IMU_TEST;
          xQueueSend(cardimetry::cardimetry_sensor_req_queue, &out_req, portMAX_DELAY);
        }

        /* Plot IMU */
        if(imu1_plot) {
          cm_display.imuPlot(
            cardimetry::cardimetry_sensor_imu_ts_qt1,
            cardimetry::cardimetry_sensor_imu_qw_qt1,
            cardimetry::cardimetry_sensor_imu_qx_qt1,
            cardimetry::cardimetry_sensor_imu_qy_qt1,
            cardimetry::cardimetry_sensor_imu_qz_qt1
          );
          imu1_plot = false;
        }

        if(imu2_plot) {
          cm_display.imuPlot(
            cardimetry::cardimetry_sensor_imu_ts_qt2,
            cardimetry::cardimetry_sensor_imu_qw_qt2,
            cardimetry::cardimetry_sensor_imu_qx_qt2,
            cardimetry::cardimetry_sensor_imu_qy_qt2,
            cardimetry::cardimetry_sensor_imu_qz_qt2
          );
          imu2_plot = false;
        }

        /* Get action */
        if(cm_display.actionEcgTest() == CARDIMETRY_DISPLAY_ECG_TEST_ACTION_BACK) {
          out_req = CARDIMETRY_SENSOR_REQ_IMU_HALT;
          xQueueSend(cardimetry::cardimetry_sensor_req_queue, &out_req, portMAX_DELAY);

          task_state      = CARDIMETRY_DISPLAY_MAIN_MENU;
          screen_refresh  = true;
          sleep_able      = true;
        }
        break;
    
    
    
    
      case CARDIMETRY_DISPLAY_PATIENT_NAME_INPUT:
        
        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Insert Patient Name"), 240, 18, 1);
          cm_display.showKeyboardInput();

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_KEYBOARD_WAIT_MS));
          screen_refresh = false;
        }

        /* Receive touch and do action */
        cm_display.actionKeyboardInput();

        /* Get keyboard input */
        if(cm_display.getKeyboardInput(&keyboard_buf) == CARDIMETRY_DISPLAY_KEYBOARD_OK) {
          
          /* Set key from keyboard input */
          cardimetry::cardimetry_conn_patient_search_key = keyboard_buf;

          task_state      = CARDIMETRY_DISPLAY_PATIENT_SEARCH;
          screen_refresh  = true;
        }

        else if(cm_display.getKeyboardInput(&keyboard_buf) == CARDIMETRY_DISPLAY_KEYBOARD_BACK) {
          task_state      = CARDIMETRY_DISPLAY_MAIN_MENU;
          screen_refresh  = true;
        }
        break;
      
      
      
      
      case CARDIMETRY_DISPLAY_PATIENT_SEARCH:
        
        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);

          cm_display.tft.loadFont(cm_display.fontFile(CARDIMETRY_DISPLAY_FONT_LIGHT, 22, false), SD);
          cm_display.tft.drawCentreString(F("Retrieving patient data..."), 240, 280, 1);
          cm_display.tft.unloadFont();

          out_req = CARDIMETRY_CONN_REQ_PATIENT_SEARCH;
          xQueueSend(cardimetry::cardimetry_conn_req_queue, &out_req, portMAX_DELAY);

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }
        break;
    
    
    

      case CARDIMETRY_DISPLAY_PATIENT_LIST:

        /* On-screen notif */
        if(screen_refresh) {
          cm_display.tft.fillScreen(0xFFFF);
          cm_display.tft.setTextColor(0x0000, 0xFFFF);
          cm_display.tft.setTextSize(2);
          cm_display.tft.drawCentreString(F("Patient List"), 240, 17, 1);

          cm_display.drawPatientList(
            cardimetry::cardimetry_conn_patient_search_id, 
            cardimetry::cardimetry_conn_patient_search_name, 
            0
          );

          vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_LOADSCREEN_WAIT_MS));
          screen_refresh = false;
        }
        break;




      case CARDIMETRY_DISPLAY_PATIENT_SEARCH_FAILED:

        /* On-screen notif */
        if(screen_refresh) {
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          TJpgDec.drawSdJpg(0, 0, CARDIMETRY_DISPLAY_LOADSCREEN_BG_PATH);
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);

          cm_display.tft.loadFont(cm_display.fontFile(CARDIMETRY_DISPLAY_FONT_LIGHT, 22, false), SD);
          cm_display.tft.drawCentreString(F("Retrieving patient data failed."), 240, 280, 1);
          cm_display.tft.unloadFont();
          
          vTaskDelay(pdMS_TO_TICKS(500));
          task_state      = CARDIMETRY_DISPLAY_MAIN_MENU;
        }
        break;
    }


    /* Task delay, room for another task to run */
    vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_DISPLAY_TASK_DELAY));
  }
}