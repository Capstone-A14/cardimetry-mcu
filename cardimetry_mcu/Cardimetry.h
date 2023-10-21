#ifndef __CARDIMETRY_H__
#define __CARDIMETRY_H__

#include "CardimetryDisplay.h"
#include "CardimetryConn.h"
// #include "CardimetrySensor.h"

#define CARDIMETRY_DISPLAY_TASK_STACK_SIZE  6144
#define CARDIMETRY_CONN_TASK_STACK_SIZE     3072
#define CARDIMETRY_SENSOR_TASK_STACK_SIZE   2048
#define CARDIMETRY_UART_TASK_STACK_SIZE     2048

#define CARDIMETRY_DISPLAY_TASK_PRIORITY  1
#define CARDIMETRY_CONN_TASK_PRIORITY     1
#define CARDIMETRY_SENSOR_TASK_PRIORITY   2
#define CARDIMETRY_UART_TASK_PRIORIY      1

#define CARDIMETRY_DISPLAY_TASK_CORE  0
#define CARDIMETRY_CONN_TASK_CORE     0
#define CARDIMETRY_SENSOR_TASK_CORE   1
#define CARDIMETRY_UART_TASK_CORE     0

#define CARDIMETRY_DISPLAY_TASK_DELAY 40
#define CARDIMETRY_CONN_TASK_DELAY    500
#define CARDIMETRY_SENSOR_TASK_DELAY  10
#define CARDIMETRY_UART_TASK_DELAY    200

#define CARDIMETRY_DISPLAY_TASK_REQ_QUEUE_LEN 8
#define CARDIMETRY_CONN_TASK_REQ_QUEUE_LEN    8
#define CARDIMETRY_SENSOR_TASK_REQ_QUEUE_LEN  8
#define CARDIMETRY_UART_TASK_REQ_QUEUE_LEN    8
#define CARDIMETRY_TASK_REQ_WAIT_MS           10

#define CARDIMETRY_SENSOR_ECG_DATA_QUEUE_LEN  300
#define CARDIMETRY_SENSOR_IMU_DATA_QUEUE_LEN  100

#define CARDIMETRY_DISPLAY_REQ_NONE                 0
#define CARDIMETRY_DISPLAY_REQ_WIFI_SCAN_SUCCESS    1
#define CARDIMETRY_DISPLAY_REQ_WIFI_SCAN_FAILED     2
#define CARDIMETRY_DISPLAY_REQ_WIFI_CONNECT_SUCCESS 3
#define CARDIMETRY_DISPLAY_REQ_WIFI_CONNECT_FAILED  4

#define CARDIMETRY_CONN_REQ_NONE          0
#define CARDIMETRY_CONN_REQ_WIFI_SCAN     1
#define CARDIMETRY_CONN_REQ_WIFI_CONNECT  2

#define CARDIMETRY_SENSOR_REQ_NONE  0

#define CARDIMETRY_UART_REQ_NONE  0

#define CARDIMETRY_UART_BAUDRATE  115200


namespace cardimetry {

  extern TaskHandle_t cardimetry_display_task_handler,
                      cardimetry_conn_task_handler,
                      cardimetry_sensor_task_handler,
                      cardimetry_uart_task_handler;

  extern SemaphoreHandle_t  cardimetry_sd_mutex,
                            cardimetry_wifi_mutex,
                            cardimetry_time_mutex,
                            cardimetry_bat_mutex;

  extern QueueHandle_t  cardimetry_display_req_queue,
                        cardimetry_conn_req_queue,
                        cardimetry_sensor_req_queue,
                        cardimetry_uart_req_queue,
                        cardimetry_sensor_ecg_data_queue,
                        cardimetry_sensor_imu_data_queue;

  extern String   cardimetry_conn_wifi_scanned_ssid[CARDIMETRY_CONN_WIFI_SCAN_MAX],
                  cardimetry_conn_wifi_scanned_enc[CARDIMETRY_CONN_WIFI_SCAN_MAX],
                  cardimetry_conn_wifi_selected_pass;
  extern int16_t  cardimetry_conn_wifi_scanned_num,
                  cardimetry_conn_wifi_scanned_rssi[CARDIMETRY_CONN_WIFI_SCAN_MAX];
  extern uint8_t  cardimetry_conn_wifi_selected;

  extern int16_t  cardimetry_conn_bat_perc,
                  cardimetry_conn_signal;
  extern uint16_t cardimetry_conn_time_m,
                  cardimetry_conn_time_h,
                  cardimetry_conn_time_d,
                  cardimetry_conn_time_mn,
                  cardimetry_conn_time_y;


  void cardimetry_display_task(void* pvParameters);
  void cardimetry_conn_task(void* pvParameters);
  void cardimetry_sensor_task(void* pvParameters);
  void cardimetry_uart_task(void* pvParameters);
  void cardimetry_begin();
}


#endif