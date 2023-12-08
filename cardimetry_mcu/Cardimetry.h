#ifndef __CARDIMETRY_H__
#define __CARDIMETRY_H__

#include "CardimetryDisplay.h"
#include "CardimetryConn.h"
#include "CardimetrySensor.h"

#define CARDIMETRY_DISPLAY_TASK_STACK_SIZE  7200
#define CARDIMETRY_CONN_TASK_STACK_SIZE     12800
#define CARDIMETRY_SENSOR_TASK_STACK_SIZE   12800
#define CARDIMETRY_UART_TASK_STACK_SIZE     2048

#define CARDIMETRY_DISPLAY_TASK_PRIORITY  1
#define CARDIMETRY_CONN_TASK_PRIORITY     1
#define CARDIMETRY_SENSOR_TASK_PRIORITY   10
#define CARDIMETRY_UART_TASK_PRIORIY      1

#define CARDIMETRY_DISPLAY_TASK_CORE  0
#define CARDIMETRY_CONN_TASK_CORE     0
#define CARDIMETRY_SENSOR_TASK_CORE   1
#define CARDIMETRY_UART_TASK_CORE     0

#define CARDIMETRY_DISPLAY_TASK_DELAY 40
#define CARDIMETRY_CONN_TASK_DELAY    200
#define CARDIMETRY_SENSOR_TASK_DELAY  10
#define CARDIMETRY_UART_TASK_DELAY    200

#define CARDIMETRY_DISPLAY_TASK_REQ_QUEUE_LEN 8
#define CARDIMETRY_CONN_TASK_REQ_QUEUE_LEN    8
#define CARDIMETRY_SENSOR_TASK_REQ_QUEUE_LEN  8
#define CARDIMETRY_UART_TASK_REQ_QUEUE_LEN    8
#define CARDIMETRY_TASK_REQ_WAIT_MS           10
#define CARDIMETRY_SENSOR_TASK_REQ_WAIT_MS    0

#define CARDIMETRY_SENSOR_ECG_DATA_QUEUE_LEN  300
#define CARDIMETRY_SENSOR_IMU_DATA_QUEUE_LEN  100

#define CARDIMETRY_DISPLAY_REQ_NONE                   0
#define CARDIMETRY_DISPLAY_REQ_WIFI_SCAN_SUCCESS      1
#define CARDIMETRY_DISPLAY_REQ_WIFI_SCAN_FAILED       2
#define CARDIMETRY_DISPLAY_REQ_WIFI_CONNECT_SUCCESS   3
#define CARDIMETRY_DISPLAY_REQ_WIFI_CONNECT_FAILED    4
#define CARDIMETRY_DISPLAY_REQ_SENSOR_INIT_SUCCESS    5
#define CARDIMETRY_DISPLAY_REQ_SENSOR_INIT_FAILED     6
#define CARDIMETRY_DISPLAY_REQ_PLOT_ECG1              7
#define CARDIMETRY_DISPLAY_REQ_PLOT_ECG2              8
#define CARDIMETRY_DISPLAY_REQ_PLOT_IMU1              9
#define CARDIMETRY_DISPLAY_REQ_PLOT_IMU2              10
#define CARDIMETRY_DISPLAY_REQ_PATIENT_SEARCH_SUCCESS 11
#define CARDIMETRY_DISPLAY_REQ_PATIENT_SEARCH_FAILED  12
#define CARDIMETRY_DISPLAY_REQ_ECG_SYNC_SUCCESS       13
#define CARDIMETRY_DISPLAY_REQ_ECG_SYNC_FAILED        14
#define CARDIMETRY_DISPLAY_REQ_LINK_DEVICE_SUCCESS    15
#define CARDIMETRY_DISPLAY_REQ_LINK_DEVICE_FAILED     16
#define CARDIMETRY_DISPLAY_REQ_LINK_PATIENT_SUCCESS   17
#define CARDIMETRY_DISPLAY_REQ_LINK_PATIENT_FAILED    18

#define CARDIMETRY_CONN_REQ_NONE            0
#define CARDIMETRY_CONN_REQ_WIFI_SCAN       1
#define CARDIMETRY_CONN_REQ_WIFI_CONNECT    2
#define CARDIMETRY_CONN_REQ_PUB_ECG1        3
#define CARDIMETRY_CONN_REQ_PUB_ECG2        4
#define CARDIMETRY_CONN_REQ_PUB_IMU1        5
#define CARDIMETRY_CONN_REQ_PUB_IMU2        6
#define CARDIMETRY_CONN_REQ_PATIENT_SEARCH  7
#define CARDIMETRY_CONN_REQ_MQTT_PUBLISH    8
#define CARDIMETRY_CONN_REQ_LINK_DEVICE     9
#define CARDIMETRY_CONN_REQ_LINK_PATIENT    10

#define CARDIMETRY_SENSOR_REQ_NONE              0
#define CARDIMETRY_SENSOR_REQ_INIT              1
#define CARDIMETRY_SENSOR_REQ_ECG_HALT          2
#define CARDIMETRY_SENSOR_REQ_ECG_READ          3
#define CARDIMETRY_SENSOR_REQ_IMU_HALT          4
#define CARDIMETRY_SENSOR_REQ_IMU_READ          5
#define CARDIMETRY_SENSOR_REQ_ECG_TEST          6
#define CARDIMETRY_SENSOR_REQ_IMU_TEST          7
#define CARDIMETRY_SENSOR_REQ_ECG_SYNC          8
#define CARDIMETRY_SENSOR_REQ_RUN_TRANSMISSION  9
#define CARDIMETRY_SENSOR_REQ_STOP_TRANSMISSION 10

#define CARDIMETRY_UART_REQ_NONE  0

#define CARDIMETRY_UART_BAUDRATE  115200


namespace cardimetry {

  extern TaskHandle_t cardimetry_display_task_handler,
                      cardimetry_conn_task_handler,
                      cardimetry_sensor_task_handler,
                      cardimetry_uart_task_handler;

  extern SemaphoreHandle_t  cardimetry_sd_mutex,
                            cardimetry_wifi_mutex,
                            cardimetry_info_mutex,
                            cardimetry_spi_mutex;

  extern QueueHandle_t  cardimetry_display_req_queue,
                        cardimetry_conn_req_queue,
                        cardimetry_sensor_req_queue,
                        cardimetry_uart_req_queue;

  extern String   cardimetry_conn_wifi_scanned_ssid[CARDIMETRY_CONN_WIFI_SCAN_MAX],
                  cardimetry_conn_wifi_scanned_enc[CARDIMETRY_CONN_WIFI_SCAN_MAX],
                  cardimetry_conn_wifi_selected_pass,
                  cardimetry_conn_wifi_connected_ssid;
  extern int16_t  cardimetry_conn_wifi_scanned_num,
                  cardimetry_conn_wifi_scanned_rssi[CARDIMETRY_CONN_WIFI_SCAN_MAX];
  extern uint8_t  cardimetry_conn_wifi_selected;

  extern int16_t  cardimetry_conn_bat_perc,
                  cardimetry_conn_signal;
  extern uint16_t cardimetry_conn_time_sec,
                  cardimetry_conn_time_mnt,
                  cardimetry_conn_time_hr,
                  cardimetry_conn_time_wd,
                  cardimetry_conn_time_md,
                  cardimetry_conn_time_mth,
                  cardimetry_conn_time_yr;

  extern uint8_t  cardimetry_conn_patient_selected;
  extern uint16_t cardimetry_conn_patient_search_id[CARDIMETRY_CONN_MAX_PATIENT_SEARCH],
                  cardimetry_conn_patient_search_num;
  extern String   cardimetry_conn_patient_search_key,
                  cardimetry_conn_patient_search_name[CARDIMETRY_CONN_MAX_PATIENT_SEARCH];

  extern String   cardimetry_sensor_ecg_ts_q1,
                  cardimetry_sensor_ecg_lead1_q1,
                  cardimetry_sensor_ecg_lead2_q1,
                  cardimetry_sensor_ecg_lead3_q1,
                  cardimetry_sensor_imu_ts_q1,
                  cardimetry_sensor_imu_qw_q1,
                  cardimetry_sensor_imu_qx_q1,
                  cardimetry_sensor_imu_qy_q1,
                  cardimetry_sensor_imu_qz_q1,

                  cardimetry_sensor_ecg_ts_q2,
                  cardimetry_sensor_ecg_lead1_q2,
                  cardimetry_sensor_ecg_lead2_q2,
                  cardimetry_sensor_ecg_lead3_q2,
                  cardimetry_sensor_imu_ts_q2,
                  cardimetry_sensor_imu_qw_q2,
                  cardimetry_sensor_imu_qx_q2,
                  cardimetry_sensor_imu_qy_q2,
                  cardimetry_sensor_imu_qz_q2;

  extern uint64_t cardimetry_sensor_ecg_ts_qt1[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
                  cardimetry_sensor_ecg_ts_qt2[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE];
  extern int32_t  cardimetry_sensor_ecg_lead1_qt1[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
                  cardimetry_sensor_ecg_lead2_qt1[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
                  cardimetry_sensor_ecg_lead3_qt1[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
                  cardimetry_sensor_ecg_lead1_qt2[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
                  cardimetry_sensor_ecg_lead2_qt2[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE],
                  cardimetry_sensor_ecg_lead3_qt2[CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE];

  extern uint64_t cardimetry_sensor_imu_ts_qt1[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
                  cardimetry_sensor_imu_ts_qt2[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE];
  extern float    cardimetry_sensor_imu_qw_qt1[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
                  cardimetry_sensor_imu_qx_qt1[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
                  cardimetry_sensor_imu_qy_qt1[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
                  cardimetry_sensor_imu_qz_qt1[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
                  cardimetry_sensor_imu_qw_qt2[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
                  cardimetry_sensor_imu_qx_qt2[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
                  cardimetry_sensor_imu_qy_qt2[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE],
                  cardimetry_sensor_imu_qz_qt2[CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE];


  void cardimetry_display_task(void* pvParameters);
  void cardimetry_conn_task(void* pvParameters);
  void cardimetry_sensor_task(void* pvParameters);
  // void cardimetry_uart_task(void* pvParameters);

  void cardimetry_ecg_sync();
  void cardimetry_begin();
}


#endif