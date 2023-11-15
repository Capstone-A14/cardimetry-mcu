#include "Cardimetry.h"



void cardimetry::cardimetry_conn_task(void* pvParameters) {

  /* Local variables */
  uint8_t   task_state        = CARDIMETRY_CONN_IDLE,
            task_req          = CARDIMETRY_CONN_REQ_NONE,
            out_req           = 0;
  bool      ecg_q1            = false,
            ecg_q2            = false,
            imu_q1            = false,
            imu_q2            = false,
            stamp_timer_free  = true;
  uint64_t  stamp_timer;
  float     battery_read  = (float)analogRead(CARDIMETRY_CONN_BAT_PIN),
            battery_perc  = (battery_read - CARDIMETRY_CONN_BAT_LOW)*100./(CARDIMETRY_CONN_BAT_FULL - CARDIMETRY_CONN_BAT_LOW),
            battery_perc_temp;
  String    ecg_msg = "",
            imu_msg = "",
            uid     = "",
            pid     = "";
  struct tm timeinfo;


  /* Battery init */
  if(battery_perc > 100.) battery_perc = 100.;
  else if(battery_perc < 0.) battery_perc = 0.;


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
        WiFi.disconnect();
        
        /* Reset number of scanned WiFi */
        xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
        cardimetry::cardimetry_conn_wifi_scanned_num = CARDIMETRY_CONN_WIFI_SCAN_UNDONE;
        xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);
        break;


      case CARDIMETRY_CONN_REQ_WIFI_CONNECT:
        task_state = CARDIMETRY_CONN_WIFI_CONNECT;
        break;


      case CARDIMETRY_CONN_REQ_LINK_DEVICE:
        if(cm_conn.linkDevice()) {
          out_req = CARDIMETRY_DISPLAY_REQ_LINK_DEVICE_SUCCESS;
        }
        else {
          out_req = CARDIMETRY_DISPLAY_REQ_LINK_DEVICE_FAILED;
        }
        xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
        break;
    
    
      case CARDIMETRY_CONN_REQ_PATIENT_SEARCH:
        task_state = CARDIMETRY_CONN_PATIENT_SEARCH;
        break;



      case CARDIMETRY_CONN_REQ_LINK_PATIENT:
        if(cm_conn.linkPatient(cardimetry::cardimetry_conn_patient_search_id[cardimetry::cardimetry_conn_patient_selected])) {
          out_req = CARDIMETRY_DISPLAY_REQ_LINK_PATIENT_SUCCESS;
        }
        else {
          out_req = CARDIMETRY_DISPLAY_REQ_LINK_PATIENT_FAILED;
        }
        xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
        break;
    
    
      case CARDIMETRY_CONN_REQ_MQTT_PUBLISH:
        uid = cm_conn.getUID(SD);
        pid = String(cardimetry::cardimetry_conn_patient_search_id[cardimetry::cardimetry_conn_patient_selected]);
        cm_conn.mqtt_client.publish((CARDIMETRY_CONN_MQTT_TOPIC_TIME + uid + String("/") + pid).c_str(), String(mktime(&timeinfo)).c_str());
        break;


      case CARDIMETRY_CONN_REQ_PUB_ECG1:
        ecg_q1 = true;
        task_state = CARDIMETRY_CONN_MQTT_PUBLISH;
        break;


      case CARDIMETRY_CONN_REQ_PUB_ECG2:
        ecg_q2 = true;
        task_state = CARDIMETRY_CONN_MQTT_PUBLISH;
        break;


      case CARDIMETRY_CONN_REQ_PUB_IMU1:
        imu_q1 = true;
        task_state = CARDIMETRY_CONN_MQTT_PUBLISH;
        break;


      case CARDIMETRY_CONN_REQ_PUB_IMU2:
        imu_q2 = true;
        task_state = CARDIMETRY_CONN_MQTT_PUBLISH;
        break;
    }
    task_req = CARDIMETRY_CONN_REQ_NONE;




    /* Task states */
    switch(task_state) {

      case CARDIMETRY_CONN_IDLE:
        
        /* Calculate battery percentage */
        battery_read      = (CARDIMETRY_CONN_BAT_COMPFILT)*((float)analogRead(CARDIMETRY_CONN_BAT_PIN)) + (1.0 - CARDIMETRY_CONN_BAT_COMPFILT)*battery_read;
        battery_perc_temp = (battery_read - CARDIMETRY_CONN_BAT_LOW)*100./(CARDIMETRY_CONN_BAT_FULL - CARDIMETRY_CONN_BAT_LOW);
        battery_perc      = (fabs(battery_perc_temp - battery_perc) < CARDIMETRY_CONN_BAT_TOLERANCE) ? battery_perc : round(battery_perc_temp);
        if(battery_perc > 100.) battery_perc = 100.;
        else if(battery_perc < 0.) battery_perc = 0.;

        xSemaphoreTake(cardimetry::cardimetry_info_mutex, portMAX_DELAY);
        cardimetry::cardimetry_conn_bat_perc = (int16_t)battery_perc;

        /* Receive wifi signal strength */
        if(WiFi.status() != WL_CONNECTED) {
          cardimetry::cardimetry_conn_signal = -9999;
          WiFi.disconnect();
        }
        else {
          /* Get WiFi RSSI */
          cardimetry::cardimetry_conn_signal = (int16_t)WiFi.RSSI();

          /* Get time from NTP */
          if(getLocalTime(&timeinfo, 400)) {
            cardimetry::cardimetry_conn_time_sec  = timeinfo.tm_sec;
            cardimetry::cardimetry_conn_time_mnt  = timeinfo.tm_min;
            cardimetry::cardimetry_conn_time_hr   = timeinfo.tm_hour;
            cardimetry::cardimetry_conn_time_wd   = timeinfo.tm_wday;
            cardimetry::cardimetry_conn_time_md   = timeinfo.tm_wday;
            cardimetry::cardimetry_conn_time_mth  = timeinfo.tm_mon;
            cardimetry::cardimetry_conn_time_yr   = timeinfo.tm_year;
          }
        }

        xSemaphoreGive(cardimetry::cardimetry_info_mutex);
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

          out_req = CARDIMETRY_DISPLAY_REQ_WIFI_SCAN_SUCCESS;
          xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
        }

        else {
          out_req = CARDIMETRY_DISPLAY_REQ_WIFI_SCAN_FAILED;
          xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
        }

        xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);
        task_state = CARDIMETRY_CONN_IDLE;
        break;



      
      case CARDIMETRY_CONN_WIFI_CONNECT:
        xSemaphoreTake(cardimetry::cardimetry_wifi_mutex, portMAX_DELAY);
        
        if(stamp_timer_free) {
          stamp_timer = millis();
          if(cardimetry::cardimetry_conn_wifi_selected_pass == "YEET69") {
            WiFi.begin(
              cardimetry::cardimetry_conn_wifi_scanned_ssid[cardimetry::cardimetry_conn_wifi_selected]
            );
          }
          else {
            WiFi.begin(
              cardimetry::cardimetry_conn_wifi_scanned_ssid[cardimetry::cardimetry_conn_wifi_selected], 
              cardimetry::cardimetry_conn_wifi_selected_pass
            );
          }
          stamp_timer_free = false;
        }

        if(WiFi.status() != WL_CONNECTED) {
          if(millis() - stamp_timer > CARDIMETRY_CONN_WIFI_CONNECT_TIMEOUT_MS) {
            /* Notify failed */
            out_req = CARDIMETRY_DISPLAY_REQ_WIFI_CONNECT_FAILED;
            xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
            task_state = CARDIMETRY_CONN_IDLE;
            stamp_timer_free = true;
          }
        }
        else {
          /* Save data to cmwifi.json */
          xSemaphoreTake(cardimetry::cardimetry_sd_mutex, portMAX_DELAY);
          cm_conn.saveConnWiFiTable(
            SD,
            cardimetry::cardimetry_conn_wifi_scanned_ssid[cardimetry::cardimetry_conn_wifi_selected], 
            cardimetry::cardimetry_conn_wifi_selected_pass
          );
          xSemaphoreGive(cardimetry::cardimetry_sd_mutex);

          /* Save SSID */
          cardimetry::cardimetry_conn_wifi_connected_ssid = cardimetry::cardimetry_conn_wifi_scanned_ssid[cardimetry::cardimetry_conn_wifi_selected];

          /* Notify success */
          out_req = CARDIMETRY_DISPLAY_REQ_WIFI_CONNECT_SUCCESS;
          xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
          task_state = CARDIMETRY_CONN_IDLE;
          stamp_timer_free = true;
        }

        xSemaphoreGive(cardimetry::cardimetry_wifi_mutex);
        break;




      case CARDIMETRY_CONN_PATIENT_SEARCH:

        /* Get patient data */
        if(cm_conn.getPatientData(
        &cardimetry::cardimetry_conn_patient_search_num,
        cardimetry::cardimetry_conn_patient_search_id,
        cardimetry::cardimetry_conn_patient_search_name,
        cardimetry::cardimetry_conn_patient_search_key)) {

          /* Send request to display patient list */
          out_req = CARDIMETRY_DISPLAY_REQ_PATIENT_SEARCH_SUCCESS;
          xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
        }

        else {
          out_req = CARDIMETRY_DISPLAY_REQ_PATIENT_SEARCH_FAILED;
          xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
        }

        /* Reset to idle */
        task_state = CARDIMETRY_CONN_IDLE;
        break;




      case CARDIMETRY_CONN_MQTT_PUBLISH:

        /* Check connection for each loop */
        cm_conn.mqtt_client.loop();

        /* Check any data that ready to be published */
        if(ecg_q1) {
          ecg_msg = String("$") + cardimetry::cardimetry_sensor_ecg_ts_q1 +
                    String("$") + cardimetry::cardimetry_sensor_ecg_lead1_q1 +
                    String("$") + cardimetry::cardimetry_sensor_ecg_lead2_q1 +
                    String("$") + cardimetry::cardimetry_sensor_ecg_lead3_q1;

          cm_conn.mqtt_client.publish((CARDIMETRY_CONN_MQTT_TOPIC_ECG + uid + String("/") + pid).c_str(), ecg_msg.c_str());

          cardimetry::cardimetry_sensor_ecg_ts_q1     = "";
          cardimetry::cardimetry_sensor_ecg_lead1_q1  = "";
          cardimetry::cardimetry_sensor_ecg_lead2_q1  = "";
          cardimetry::cardimetry_sensor_ecg_lead3_q1  = "";

          ecg_q1 = false;
        }

        if(ecg_q2) {
          ecg_msg = String("$") + cardimetry::cardimetry_sensor_ecg_ts_q2 +
                    String("$") + cardimetry::cardimetry_sensor_ecg_lead1_q2 +
                    String("$") + cardimetry::cardimetry_sensor_ecg_lead2_q2 +
                    String("$") + cardimetry::cardimetry_sensor_ecg_lead3_q2;

          cm_conn.mqtt_client.publish((CARDIMETRY_CONN_MQTT_TOPIC_ECG + uid + String("/") + pid).c_str(), ecg_msg.c_str());

          cardimetry::cardimetry_sensor_ecg_ts_q2     = "";
          cardimetry::cardimetry_sensor_ecg_lead1_q2  = "";
          cardimetry::cardimetry_sensor_ecg_lead2_q2  = "";
          cardimetry::cardimetry_sensor_ecg_lead3_q2  = "";

          ecg_q2 = false;
        }

        if(imu_q1) {
          imu_msg = String("$") + cardimetry::cardimetry_sensor_imu_ts_q1 + 
                    String("$") + cardimetry::cardimetry_sensor_imu_qw_q1 +
                    String("$") + cardimetry::cardimetry_sensor_imu_qx_q1 +
                    String("$") + cardimetry::cardimetry_sensor_imu_qy_q1 +
                    String("$") + cardimetry::cardimetry_sensor_imu_qz_q1;

          cm_conn.mqtt_client.publish((CARDIMETRY_CONN_MQTT_TOPIC_IMU + uid + String("/") + pid).c_str(), imu_msg.c_str());

          cardimetry::cardimetry_sensor_imu_ts_q1 = "";
          cardimetry::cardimetry_sensor_imu_qw_q1 = "";
          cardimetry::cardimetry_sensor_imu_qx_q1 = "";
          cardimetry::cardimetry_sensor_imu_qy_q1 = "";
          cardimetry::cardimetry_sensor_imu_qz_q1 = "";

          imu_q1 = false;
        }

        if(imu_q2) {
          imu_msg = String("$") + cardimetry::cardimetry_sensor_imu_ts_q2 + 
                    String("$") + cardimetry::cardimetry_sensor_imu_qw_q2 +
                    String("$") + cardimetry::cardimetry_sensor_imu_qx_q2 +
                    String("$") + cardimetry::cardimetry_sensor_imu_qy_q2 +
                    String("$") + cardimetry::cardimetry_sensor_imu_qz_q2;

          cm_conn.mqtt_client.publish((CARDIMETRY_CONN_MQTT_TOPIC_IMU + uid + String("/") + pid).c_str(), imu_msg.c_str());

          cardimetry::cardimetry_sensor_imu_ts_q2 = "";
          cardimetry::cardimetry_sensor_imu_qw_q2 = "";
          cardimetry::cardimetry_sensor_imu_qx_q2 = "";
          cardimetry::cardimetry_sensor_imu_qy_q2 = "";
          cardimetry::cardimetry_sensor_imu_qz_q2 = "";

          imu_q2 = false;
        }

        /* Reset to idle */
        task_state = CARDIMETRY_CONN_IDLE;
        break;
    }


    /* Task delay, room for another task to run */
    vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_CONN_TASK_DELAY));
  }
}