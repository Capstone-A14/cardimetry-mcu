#ifndef __TRANSMIT_TEST_H__
#define __TRANSMIT_TEST_H__

#include <WiFi.h>
#include <SPI.h>
#include "protocentral_ads1293.h"
#include "EspMQTTClient.h"
#include "time.h"
#include "sntp.h"

#define WIFI_SSID "DhonanAP_HP"
#define WIFI_PASS "12345678"

#define MQTT_SERVER "192.168.42.102"
#define MQTT_PORT   1883
#define MQTT_TOPIC  "/dev/ecg/pub"
#define MQTT_TOPIC1 "/dev/time/pub"

#define NTP_SERVER_1  "pool.ntp.org"
#define NTP_SERVER_2  "time.nist.gov"
#define GMT_OFFSET_S  21600
#define DL_OFFSET_S   3600

#define ADS1293_DRDY_PIN  35
#define ADS1293_CS_PIN    32
#define ADS1293_READY     false

#define ECG_SAMPLING_TIME_MS          10
#define ECG_QUEUE_SIZE                100
#define ECG_MOCK_SIZE                 116
#define IMU_SAMPLING_COUNT_AFTER_ECG  2

#define TRANSMISSION_RATE_MS 800



EspMQTTClient client(
  WIFI_SSID,
  WIFI_PASS,
  MQTT_SERVER,
  "CardimetryTest",
  MQTT_PORT
);

ads1293 ADS1293(ADS1293_DRDY_PIN, ADS1293_CS_PIN);

SemaphoreHandle_t ecg1_mutex,
                  ecg2_mutex,
                  ecg_ready_mutex;

bool  ecg1_ready = false,
      ecg2_ready = false;

String  ecg1_lead1  = "",
        ecg1_lead2  = "",
        ecg1_lead3  = "",
        ecg1_ts     = "",
        ecg2_lead1  = "",
        ecg2_lead2  = "",
        ecg2_lead3  = "",
        ecg2_ts     = "";



void sensor_get_task(void* pvParameters) {
  /* Local variables */
  uint8_t imu_sampling_cnt_after_ecg = 0,
          ecg_queue_cnt = 0;
  bool  ecg_json_sw = true,
        ecg_semtake = false;

  /* Start SPI */
  SPI.begin();

  /* Create Mutex */
  ecg1_mutex      = xSemaphoreCreateMutex();
  ecg2_mutex      = xSemaphoreCreateMutex();
  ecg_ready_mutex = xSemaphoreCreateMutex();

  /* Start ADS1293 */
  uint8_t ecg_cnt = 0;
  int ecg_mock[116] = {
    2041,2043,2037,2047,2060,2062,2051,2023,2014,2027,2034,2033,2040,2047,
    2047,2053,2058,2064,2059,2063,2061,2052,2053,2038,1966,1885,1884,2009,
    2129,2166,2137,2102,2086,2077,2067,2067,2060,2059,2062,2062,2060,2057,
    2045,2047,2057,2054,2042,2029,2027,2018,2007,1995,2001,2012,2024,2039,
    2068,2092,2111,2125,2131,2148,2137,2138,2128,2128,2115,2099,2097,2096,
    2101,2101,2091,2073,2076,2077,2084,2081,2088,2092,2070,2069,2074,2077,
    2075,2068,2064,2060,2062,2074,2075,2074,2075,2063,2058,2058,2064,2064,
    2070,2074,2067,2060,2062,2063,2061,2059,2048,2052,2049,2048,2051,2059,
    2059,2066,2077,2073
  };
  ADS1293.ads1293Begin3LeadECG();
  vTaskDelay(pdMS_TO_TICKS(10));

  /* Loop */
  while(true) {

    /* ECG sampling */
    if(digitalRead(ADS1293_DRDY_PIN) == ADS1293_READY) {
      int32_t lead1 = ecg_mock[ecg_cnt],
              lead2 = ecg_mock[ecg_cnt],
              lead3 = ecg_mock[ecg_cnt];
      ecg_cnt = (++ecg_cnt) % ECG_MOCK_SIZE;
      
      if(ecg_json_sw) {
        xSemaphoreTake(ecg1_mutex, portMAX_DELAY);
        ecg1_lead1  += String(lead1) + String(',');
        ecg1_lead2  += String(lead2) + String(',');
        ecg1_lead3  += String(lead3) + String(',');
        ecg1_ts     += String(millis()) + String(',');
        xSemaphoreGive(ecg1_mutex);
      }

      else {
        xSemaphoreTake(ecg2_mutex, portMAX_DELAY);
        ecg2_lead1  += String(lead1) + String(',');
        ecg2_lead2  += String(lead2) + String(',');
        ecg2_lead3  += String(lead3) + String(',');
        ecg2_ts     += String(millis()) + String(',');
        xSemaphoreGive(ecg2_mutex);
      }

      ++ecg_queue_cnt;

      if(ecg_queue_cnt == ECG_QUEUE_SIZE) {

        if(ecg_json_sw) {
          xSemaphoreTake(ecg_ready_mutex, portMAX_DELAY);
          ecg1_ready = true;
          xSemaphoreGive(ecg_ready_mutex);
        }
        else {
          xSemaphoreTake(ecg_ready_mutex, portMAX_DELAY);
          ecg2_ready = true;
          xSemaphoreGive(ecg_ready_mutex);
        }
        ecg_json_sw   = !ecg_json_sw;
        ecg_queue_cnt = 0;
      }
    }

    /* IMU sampling */
    // ++imu_sampling_cnt_after_ecg;
    // if(imu_sampliing_cnt_after_ecg == IMU_SAMPLING_COUNT_AFTER_ECG) {
      
    //   /* IMU sampling */


    //   /* Reset */
    //   imu_sampling_cnt_after_ecg = 0;
    // }

    /* Delay */
    vTaskDelay(pdMS_TO_TICKS(ECG_SAMPLING_TIME_MS));
  }
}



void onConnectionEstablished() {
  return;
}



void data_transmit_task(void* pvParameters) {
  /* Local variables */
  String  cmml_data,
          unix_ts;
  bool  ecg1_rd       = false,
        ecg2_rd       = false,
        when_connect  = true;

  /* Set NTP */
  sntp_servermode_dhcp(1);
  configTime(GMT_OFFSET_S, DL_OFFSET_S, NTP_SERVER_1, NTP_SERVER_2);

  /* Additional */
  client.setMaxPacketSize(6400);
  client.enableDebuggingMessages(); 

  while(true) {

    client.loop();

    /* Check ECG ready */
    xSemaphoreTake(ecg_ready_mutex, portMAX_DELAY);
    ecg1_rd     = ecg1_ready;
    ecg1_ready  = false;

    ecg2_rd     = ecg2_ready;
    ecg2_ready  = false;
    xSemaphoreGive(ecg_ready_mutex);

    if(when_connect) {
      struct tm timeinfo;
      if(getLocalTime(&timeinfo)) {
        time_t unix_time = mktime(&timeinfo);
        unix_ts = String(unix_time);
      }
    }

    if(ecg1_rd) {
      xSemaphoreTake(ecg1_mutex, portMAX_DELAY);
      cmml_data   = String('$') + ecg1_ts + String('$') + ecg1_lead1 + String('$') + ecg1_lead2 + String('$') + ecg1_lead3;
      ecg1_lead1  = "";
      ecg1_lead2  = "";
      ecg1_lead3  = "";
      ecg1_ts     = "";
      xSemaphoreGive(ecg1_mutex);
      
      if(when_connect) {
        if(client.publish(MQTT_TOPIC1, unix_ts)) {
          Serial.println("Connection Established");
          when_connect = false;
        }
        else {
          Serial.println("Failed to establish connection");
        }
      }

      if(!when_connect) {
        if(client.publish(MQTT_TOPIC, cmml_data.c_str())) {
          Serial.println("Published!");
          cmml_data = "";
        }
        else {
          Serial.println("Failed to publish ECG1!");
          cmml_data = "";
        }
      }
    }

    else if(ecg2_rd) {
      xSemaphoreTake(ecg2_mutex, portMAX_DELAY);
      cmml_data   = String('$') + ecg2_ts + String('$') + ecg2_lead1 + String('$') + ecg2_lead2 + String('$') + ecg2_lead3;
      ecg2_lead1  = "";
      ecg2_lead2  = "";
      ecg2_lead3  = "";
      ecg2_ts     = "";
      xSemaphoreGive(ecg2_mutex);
      
      if(when_connect) {
        if(client.publish(MQTT_TOPIC1, unix_ts)) {
          Serial.println("Connection Established");
          when_connect = false;
        }
        else {
          Serial.println("Failed to establish connection");
        }
      }

      if(!when_connect) {
        if(client.publish(MQTT_TOPIC, cmml_data.c_str())) {
          Serial.println("Published!");
          cmml_data = "";
        }
        else {
          Serial.println("Failed to publish ECG2!");
          cmml_data = "";
        }
      }
    }

    /* Delay */
    vTaskDelay(pdMS_TO_TICKS(TRANSMISSION_RATE_MS));
  }
}



void transmit_begin() {

  /* Create tasks */
  xTaskCreatePinnedToCore(
    sensor_get_task,
    "Sensor Get",
    9600,
    NULL,
    10,
    NULL,
    1
  );

  xTaskCreatePinnedToCore(
    data_transmit_task,
    "Data Transmit",
    9600,
    NULL,
    10,
    NULL,
    0
  );
}



void transmit_loop() {

}

#endif