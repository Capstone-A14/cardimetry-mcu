#ifndef __TRANSMIT_TEST_H__
#define __TRANSMIT_TEST_H__

#include <WiFi.h>
#include <SPI.h>
#include "protocentral_ads1293.h"
#include "EspMQTTClient.h"

#define WIFI_SSID "Kost Jakal"
#define WIFI_PASS "kucinglisa16"

#define MQTT_SERVER "192.168.1.12"
#define MQTT_PORT   1883
#define MQTT_TOPIC  "/dev/ecg/pub"

#define ADS1293_DRDY_PIN  35
#define ADS1293_CS_PIN    32
#define ADS1293_READY     false

#define ECG_SAMPLING_TIME_MS          10
#define ECG_QUEUE_SIZE                100
#define IMU_SAMPLING_COUNT_AFTER_ECG  2

#define TRANSMISSION_RATE_MS 800



EspMQTTClient client(
  WIFI_SSID,
  WIFI_PASS,
  MQTT_SERVER,
  "Cardimetry",
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
  ecg1_mutex = xSemaphoreCreateMutex();
  ecg2_mutex = xSemaphoreCreateMutex();
  ecg_ready_mutex = xSemaphoreCreateMutex();

  /* Start ADS1293 */
  ADS1293.ads1293Begin3LeadECG();
  vTaskDelay(pdMS_TO_TICKS(10));

  /* Loop */
  while(true) {

    /* ECG sampling */
    if(digitalRead(ADS1293_DRDY_PIN) == ADS1293_READY) {
      int32_t lead1 = ADS1293.getECGdata(1),
              lead2 = ADS1293.getECGdata(2),
              lead3 = lead2 - lead1;
      
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
  String        cmml_data;
  bool  ecg1_rd = false,
        ecg2_rd = false;

  /* Additional */
  client.setMaxPacketSize(4096);
  // client.enableDebuggingMessages(); 

  while(true) {

    client.loop();

    /* Check ECG ready */
    xSemaphoreTake(ecg_ready_mutex, portMAX_DELAY);
    ecg1_rd     = ecg1_ready;
    ecg1_ready  = false;

    ecg2_rd     = ecg2_ready;
    ecg2_ready  = false;
    xSemaphoreGive(ecg_ready_mutex);

    if(ecg1_rd) {
      xSemaphoreTake(ecg1_mutex, portMAX_DELAY);
      cmml_data   = String('$') + ecg1_ts + String('$') + ecg1_lead1 + String('$') + ecg1_lead2 + String('$') + ecg1_lead3;
      ecg1_lead1  = "";
      ecg1_lead2  = "";
      ecg1_lead3  = "";
      ecg1_ts     = "";
      xSemaphoreGive(ecg1_mutex);
      
      if(client.publish(MQTT_TOPIC, cmml_data.c_str())) {
        Serial.println("Published!");
        cmml_data = "";
      }
      else {
        Serial.println("Failed to publish ECG1!");
        cmml_data = "";
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
      
      if(client.publish(MQTT_TOPIC, cmml_data.c_str())) {
        Serial.println("Published!");
        Serial.println(cmml_data);
        cmml_data = "";
      }
      else {
        Serial.println("Failed to publish ECG2!");
        Serial.println(cmml_data);
        cmml_data = "";
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