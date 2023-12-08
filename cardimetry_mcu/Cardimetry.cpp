#include "Cardimetry.h"




TaskHandle_t  cardimetry::cardimetry_display_task_handler,
              cardimetry::cardimetry_conn_task_handler,
              cardimetry::cardimetry_sensor_task_handler,
              cardimetry::cardimetry_uart_task_handler;

SemaphoreHandle_t cardimetry::cardimetry_sd_mutex,
                  cardimetry::cardimetry_wifi_mutex,
                  cardimetry::cardimetry_info_mutex,
                  cardimetry::cardimetry_spi_mutex;

QueueHandle_t cardimetry::cardimetry_display_req_queue,
              cardimetry::cardimetry_conn_req_queue,
              cardimetry::cardimetry_sensor_req_queue,
              cardimetry::cardimetry_uart_req_queue;

String  cardimetry::cardimetry_conn_wifi_scanned_ssid[CARDIMETRY_CONN_WIFI_SCAN_MAX],
        cardimetry::cardimetry_conn_wifi_scanned_enc[CARDIMETRY_CONN_WIFI_SCAN_MAX],
        cardimetry::cardimetry_conn_wifi_selected_pass,
        cardimetry::cardimetry_conn_wifi_connected_ssid;
int16_t cardimetry::cardimetry_conn_wifi_scanned_num,
        cardimetry::cardimetry_conn_wifi_scanned_rssi[CARDIMETRY_CONN_WIFI_SCAN_MAX];
uint8_t cardimetry::cardimetry_conn_wifi_selected;

int16_t   cardimetry::cardimetry_conn_bat_perc,
          cardimetry::cardimetry_conn_signal;
uint16_t  cardimetry::cardimetry_conn_time_sec,
          cardimetry::cardimetry_conn_time_mnt,
          cardimetry::cardimetry_conn_time_hr,
          cardimetry::cardimetry_conn_time_wd,
          cardimetry::cardimetry_conn_time_md,
          cardimetry::cardimetry_conn_time_mth,
          cardimetry::cardimetry_conn_time_yr;

uint8_t   cardimetry::cardimetry_conn_patient_selected;
uint16_t  cardimetry::cardimetry_conn_patient_search_id[CARDIMETRY_CONN_MAX_PATIENT_SEARCH],
          cardimetry::cardimetry_conn_patient_search_num;
String    cardimetry::cardimetry_conn_patient_search_key,
          cardimetry::cardimetry_conn_patient_search_name[CARDIMETRY_CONN_MAX_PATIENT_SEARCH];

String  cardimetry::cardimetry_sensor_ecg_ts_q1     = "",
        cardimetry::cardimetry_sensor_ecg_lead1_q1  = "",
        cardimetry::cardimetry_sensor_ecg_lead2_q1  = "",
        cardimetry::cardimetry_sensor_ecg_lead3_q1  = "",
        cardimetry::cardimetry_sensor_imu_ts_q1     = "",
        cardimetry::cardimetry_sensor_imu_qw_q1     = "",
        cardimetry::cardimetry_sensor_imu_qx_q1     = "",
        cardimetry::cardimetry_sensor_imu_qy_q1     = "",
        cardimetry::cardimetry_sensor_imu_qz_q1     = "",

        cardimetry::cardimetry_sensor_ecg_ts_q2     = "",
        cardimetry::cardimetry_sensor_ecg_lead1_q2  = "",
        cardimetry::cardimetry_sensor_ecg_lead2_q2  = "",
        cardimetry::cardimetry_sensor_ecg_lead3_q2  = "",
        cardimetry::cardimetry_sensor_imu_ts_q2     = "",
        cardimetry::cardimetry_sensor_imu_qw_q2     = "",
        cardimetry::cardimetry_sensor_imu_qx_q2     = "",
        cardimetry::cardimetry_sensor_imu_qy_q2     = "",
        cardimetry::cardimetry_sensor_imu_qz_q2     = "";

uint64_t  cardimetry::cardimetry_sensor_ecg_ts_qt1[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_ecg_ts_qt2[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE];
int32_t   cardimetry::cardimetry_sensor_ecg_lead1_qt1[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_ecg_lead2_qt1[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_ecg_lead3_qt1[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_ecg_lead1_qt2[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_ecg_lead2_qt2[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_ecg_lead3_qt2[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE];

uint64_t  cardimetry::cardimetry_sensor_imu_ts_qt1[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_imu_ts_qt2[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE];
float     cardimetry::cardimetry_sensor_imu_qw_qt1[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_imu_qx_qt1[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_imu_qy_qt1[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_imu_qz_qt1[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_imu_qw_qt2[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_imu_qx_qt2[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_imu_qy_qt2[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
          cardimetry::cardimetry_sensor_imu_qz_qt2[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE];




void cardimetry::cardimetry_ecg_sync() {
  SPI.begin();
  ads1293 ADSSYNC = ads1293(CARDIMETRY_SENSOR_ECG_DRDY_PIN, CARDIMETRY_SENSOR_ECG_CS_PIN);
  pinMode(CARDIMETRY_SENSOR_ECG_CS_PIN, OUTPUT);
  ADSSYNC.ads1293Begin3LeadECG();
  vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_SENSOR_ECG_SYNC_DELAY_MS));
  for(int i = 0; i < 30; ++i) {
    if(digitalRead(CARDIMETRY_SENSOR_ECG_DRDY_PIN) == false) {
      int32_t lead1 = ADSSYNC.getECGdata(1),
              lead2 = ADSSYNC.getECGdata(2),
              lead3 = lead1 - lead2;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}




void cardimetry::cardimetry_begin() {

  /* Do the ECG synchronization first */
  cardimetry::cardimetry_ecg_sync();


  /* Create mutexes */
  cardimetry::cardimetry_sd_mutex   = xSemaphoreCreateMutex();
  cardimetry::cardimetry_wifi_mutex = xSemaphoreCreateMutex();
  cardimetry::cardimetry_info_mutex = xSemaphoreCreateMutex();
  cardimetry::cardimetry_spi_mutex  = xSemaphoreCreateMutex();


  /* Create queue handlers */
  cardimetry::cardimetry_display_req_queue      = xQueueCreate(CARDIMETRY_DISPLAY_TASK_REQ_QUEUE_LEN, sizeof(uint8_t));
  cardimetry::cardimetry_conn_req_queue         = xQueueCreate(CARDIMETRY_CONN_TASK_REQ_QUEUE_LEN, sizeof(uint8_t));
  cardimetry::cardimetry_sensor_req_queue       = xQueueCreate(CARDIMETRY_SENSOR_TASK_REQ_QUEUE_LEN, sizeof(uint8_t));
  // cardimetry::cardimetry_uart_req_queue         = xQueueCreate(CARDIMETRY_UART_TASK_REQ_QUEUE_LEN, sizeof(uint8_t));
  
  // Serial.begin(115200);

  /* Execute tasks */
  xTaskCreatePinnedToCore(
    cardimetry::cardimetry_display_task,
    "Cardimetry Display",
    CARDIMETRY_DISPLAY_TASK_STACK_SIZE,
    NULL,
    CARDIMETRY_DISPLAY_TASK_PRIORITY,
    &cardimetry::cardimetry_display_task_handler,
    CARDIMETRY_DISPLAY_TASK_CORE
  );

  xTaskCreatePinnedToCore(
    cardimetry::cardimetry_conn_task,
    "Cardimetry Connection",
    CARDIMETRY_CONN_TASK_STACK_SIZE,
    NULL,
    CARDIMETRY_CONN_TASK_PRIORITY,
    &cardimetry::cardimetry_conn_task_handler,
    CARDIMETRY_CONN_TASK_CORE
  );

  xTaskCreatePinnedToCore(
    cardimetry::cardimetry_sensor_task,
    "Cardimetry Sensor",
    CARDIMETRY_SENSOR_TASK_STACK_SIZE,
    NULL,
    CARDIMETRY_SENSOR_TASK_PRIORITY,
    &cardimetry::cardimetry_sensor_task_handler,
    CARDIMETRY_SENSOR_TASK_CORE
  );

  // xTaskCreatePinnedToCore(
  //   cardimetry::cardimetry_uart_task,
  //   "Cardimetry UART",
  //   CARDIMETRY_UART_TASK_STACK_SIZE,
  //   NULL,
  //   CARDIMETRY_UART_TASK_PRIORITY,
  //   &cardimetry::cardimetry_uart_task_handler,
  //   CARDIMETRY_UART_TASK_CORE
  // );
}