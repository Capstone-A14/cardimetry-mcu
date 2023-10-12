#include "Cardimetry.h"



void cardimetry::cardimetry_conn_task(void* pvParameters) {

  /* Local variables */
  uint8_t task_state  = CARDIMETRY_CONN_IDLE,
          task_req    = CARDIMETRY_CONN_REQ_NONE;


  /* Initiate WiFi */
  cardimetry::CardimetryConn cm_conn = cardimetry::CardimetryConn();
  xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
  cm_conn.begin();
  xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);


  /* Loop */
  while(true) {
    
    /* Read incoming request from another task */
    xQueueReceive(cardimetry::cardimetry_conn_req_queue, &task_req, pdMS_TO_TICKS(CARDIMETRY_TASK_REQ_WAIT_MS));
    switch(task_req) {

      case CARDIMETRY_CONN_REQ_NONE:
        break;

      case CARDIMETRY_CONN_REQ_WIFI_SCAN:
        task_state = CARDIMETRY_CONN_WIFI_SCAN;
        /* Reset number of scanned WiFi */
        xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
        cardimetry::cardimetry_conn_wifi_scanned_num = CARDIMETRY_CONN_WIFI_SCAN_UNDONE;
        xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);
        break;

      case CARDIMETRY_CONN_REQ_WIFI_CONNECT:
        task_state = CARDIMETRY_CONN_WIFI_CONNECT;
        break;
    }
    task_req = CARDIMETRY_CONN_REQ_NONE;




    /* Task states */
    switch(task_state) {

      case CARDIMETRY_CONN_IDLE:
        break;




      case CARDIMETRY_CONN_WIFI_SCAN:

        xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
        cardimetry::cardimetry_conn_wifi_scanned_num = cm_conn.scanWiFi();
        
        if(cardimetry::cardimetry_conn_wifi_scanned_num > 0) {
        
          for(uint8_t i = 0; i < (uint8_t)min(CARDIMETRY_CONN_WIFI_SCAN_MAX, (int)cardimetry::cardimetry_conn_wifi_scanned_num); ++i) {
            cardimetry::cardimetry_conn_wifi_scanned_ssid[i]  = WiFi.SSID(i);
            cardimetry::cardimetry_conn_wifi_scanned_rssi[i]  = WiFi.RSSI(i);
            switch(WiFi.encryptionType(i)) {
              case WIFI_AUTH_OPEN:
                cardimetry::cardimetry_conn_wifi_scanned_enc[i] = "Open";
                break;
              case WIFI_AUTH_WEP:
                cardimetry::cardimetry_conn_wifi_scanned_enc[i] = "WEP";
                break;
              case WIFI_AUTH_WPA_PSK:
                cardimetry::cardimetry_conn_wifi_scanned_enc[i] = "WPA";
                break;
              case WIFI_AUTH_WPA2_PSK:
                cardimetry::cardimetry_conn_wifi_scanned_enc[i] = "WPA2";
                break;
              case WIFI_AUTH_WPA_WPA2_PSK:
                cardimetry::cardimetry_conn_wifi_scanned_enc[i] = "WPA1+2";
                break;
              case WIFI_AUTH_WPA2_ENTERPRISE:
                cardimetry::cardimetry_conn_wifi_scanned_enc[i] = "WPA2-E";
                break;
              case WIFI_AUTH_WPA3_PSK:
                cardimetry::cardimetry_conn_wifi_scanned_enc[i] = "WPA3";
                break;
              case WIFI_AUTH_WPA2_WPA3_PSK:
                cardimetry::cardimetry_conn_wifi_scanned_enc[i] = "WPA2+3";
                break;
              case WIFI_AUTH_WAPI_PSK:
                cardimetry::cardimetry_conn_wifi_scanned_enc[i] = "WAPI";
                break;
              default:
                cardimetry::cardimetry_conn_wifi_scanned_enc[i] = "???";
                break;
            }
          }
        }
        xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);
        task_state = CARDIMETRY_CONN_IDLE;
        break;



      
      case CARDIMETRY_CONN_WIFI_CONNECT:

        break;
    }


    /* Task delay, room for another task to run */
    vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_CONN_TASK_DELAY));
  }
}