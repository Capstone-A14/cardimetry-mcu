#include "Cardimetry.h"




TaskHandle_t  cardimetry::cardimetry_display_task_handler,
              cardimetry::cardimetry_conn_task_handler,
              cardimetry::cardimetry_sensor_task_handler,
              cardimetry::cardimetry_uart_task_handler;

SemaphoreHandle_t cardimetry::cardimetry_sd_mutex,
                  cardimetry::cardimetry_wifi_mutex;

QueueHandle_t cardimetry::cardimetry_display_req_queue,
              cardimetry::cardimetry_conn_req_queue,
              cardimetry::cardimetry_sensor_req_queue,
              cardimetry::cardimetry_uart_req_queue,
              cardimetry::cardimetry_sensor_ecg_data_queue,
              cardimetry::cardimetry_sensor_imu_data_queue;

String  cardimetry::cardimetry_conn_wifi_scanned_ssid[CARDIMETRY_CONN_WIFI_SCAN_MAX],
        cardimetry::cardimetry_conn_wifi_scanned_enc[CARDIMETRY_CONN_WIFI_SCAN_MAX],
        cardimetry::cardimetry_conn_wifi_selected_pass;
int16_t cardimetry::cardimetry_conn_wifi_scanned_num,
        cardimetry::cardimetry_conn_wifi_scanned_rssi[CARDIMETRY_CONN_WIFI_SCAN_MAX];
uint8_t cardimetry::cardimetry_conn_wifi_selected;




void cardimetry::cardimetry_begin() {

  /* Create mutexes */
  cardimetry::cardimetry_sd_mutex   = xSemaphoreCreateMutex();
  cardimetry::cardimetry_wifi_mutex = xSemaphoreCreateMutex();


  /* Create queue handlers */
  cardimetry::cardimetry_display_req_queue      = xQueueCreate(CARDIMETRY_DISPLAY_TASK_REQ_QUEUE_LEN, sizeof(uint8_t));
  cardimetry::cardimetry_conn_req_queue         = xQueueCreate(CARDIMETRY_CONN_TASK_REQ_QUEUE_LEN, sizeof(uint8_t));
  cardimetry::cardimetry_sensor_req_queue       = xQueueCreate(CARDIMETRY_SENSOR_TASK_REQ_QUEUE_LEN, sizeof(uint8_t));
  cardimetry::cardimetry_uart_req_queue         = xQueueCreate(CARDIMETRY_UART_TASK_REQ_QUEUE_LEN, sizeof(uint8_t));
  cardimetry::cardimetry_sensor_ecg_data_queue  = xQueueCreate(CARDIMETRY_SENSOR_ECG_DATA_QUEUE_LEN, sizeof(double));
  cardimetry::cardimetry_sensor_imu_data_queue  = xQueueCreate(CARDIMETRY_SENSOR_IMU_DATA_QUEUE_LEN, sizeof(double));
  

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

  // xTaskCreatePinnedToCore(
  //   cardimetry::cardimetry_sensor_task,
  //   "Cardimetry Sensor",
  //   CARDIMETRY_SENSOR_TASK_STACK_SIZE,
  //   NULL,
  //   CARDIMETRY_SENSOR_TASK_PRIORITY,
  //   &cardimetry::cardimetry_sensor_task_handler,
  //   CARDIMETRY_SENSOR_TASK_CORE
  // );

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