#include "Cardimetry.h"



void cardimetry::cardimetry_sensor_task(void* pvParameters) {

  /* Local variables */
  uint8_t   task_state  = CARDIMETRY_SENSOR_IDLE,
            task_req    = CARDIMETRY_SENSOR_REQ_NONE,
            out_req     = 0,
            imu_state   = CARDIMETRY_SENSOR_HALT,
            ecg_state   = CARDIMETRY_SENSOR_HALT;


  /* Cardimetry sensor instance */
  cardimetry::CardimetrySensor cm_sensor = cardimetry::CardimetrySensor();
  

  /* Loop */
  while(true) {

    /* Read incoming request from other tasks */
    xQueueReceive(cardimetry::cardimetry_sensor_req_queue, &task_req, pdMS_TO_TICKS(CARDIMETRY_SENSOR_TASK_REQ_WAIT_MS));
    switch(task_req) {
      
      case CARDIMETRY_SENSOR_REQ_NONE:
        break;


      case CARDIMETRY_SENSOR_REQ_INIT:
        task_state = CARDIMETRY_SENSOR_INIT;
        break;


      case CARDIMETRY_SENSOR_REQ_ECG_HALT:
        ecg_state   = CARDIMETRY_SENSOR_HALT;
        task_state  = (imu_state == CARDIMETRY_SENSOR_HALT) ? CARDIMETRY_SENSOR_IDLE : CARDIMETRY_SENSOR_RUN;
        break;


      case CARDIMETRY_SENSOR_REQ_ECG_READ:
        ecg_state   = CARDIMETRY_SENSOR_READ;
        task_state  = CARDIMETRY_SENSOR_RUN;
        break;


      case CARDIMETRY_SENSOR_REQ_IMU_HALT:
        imu_state   = CARDIMETRY_SENSOR_HALT;
        task_state  = (ecg_state == CARDIMETRY_SENSOR_HALT) ? CARDIMETRY_SENSOR_IDLE : CARDIMETRY_SENSOR_RUN;
        break;


      case CARDIMETRY_SENSOR_REQ_IMU_READ:
        imu_state   = CARDIMETRY_SENSOR_READ;
        task_state  = CARDIMETRY_SENSOR_RUN;
        break;
    }
    task_req = CARDIMETRY_SENSOR_REQ_NONE;



    
    /* Task states */
    switch(task_state) {

      case CARDIMETRY_SENSOR_IDLE:
        break;




      case CARDIMETRY_SENSOR_INIT:
        
        if(cm_sensor.begin()) {
          out_req = CARDIMETRY_DISPLAY_REQ_SENSOR_INIT_SUCCESS;
          xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
        }

        else {
          out_req = CARDIMETRY_DISPLAY_REQ_SENSOR_INIT_FAILED;
          xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
        }

        task_state = CARDIMETRY_SENSOR_IDLE;
        break;




      case CARDIMETRY_SENSOR_RUN:
        break;
    }

    vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_SENSOR_TASK_DELAY));
  }
}