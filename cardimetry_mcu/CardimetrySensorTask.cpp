#include "Cardimetry.h"



void cardimetry::cardimetry_sensor_task(void* pvParameters) {

  /* Local variables */
  uint8_t   task_state      = CARDIMETRY_SENSOR_IDLE,
            task_req        = CARDIMETRY_SENSOR_REQ_NONE,
            out_req         = 0,
            ecg_state       = CARDIMETRY_SENSOR_HALT,
            imu_state       = CARDIMETRY_SENSOR_HALT,
            ecg_cnt         = 0,
            imu_cnt         = 0;
  bool      ecg_qs          = CARDIMETRY_SENSOR_ECG_QUEUE1,
            imu_qs          = CARDIMETRY_SENSOR_IMU_QUEUE1,
            imu_rel_cnt     = true;
  int32_t   lead1,
            lead2,
            lead3;
  uint64_t  start_trans_time;

  // sfe_ism_raw_data_t accel_data; 
  // sfe_ism_raw_data_t gyro_data;
  sensors_event_t a, g, m, temp;
  float acc_x, acc_y, acc_z,
        gyr_x, gyr_y, gyr_z,
        mag_x, mag_y, mag_z,
        qw, qx, qy, qz;


  /* Cardimetry sensor instance */
  cardimetry::CardimetrySensor cm_sensor = cardimetry::CardimetrySensor();
  

  /* Loop */
  while(true) {

    /* Read incoming request from other tasks */
    xQueueReceive(cardimetry::cardimetry_sensor_req_queue, &task_req, 0);
    switch(task_req) {
      
      case CARDIMETRY_SENSOR_REQ_NONE:
        break;


      case CARDIMETRY_SENSOR_REQ_INIT:
        task_state = CARDIMETRY_SENSOR_INIT;
        break;


      case CARDIMETRY_SENSOR_REQ_ECG_HALT:
        ecg_state = CARDIMETRY_SENSOR_HALT;
        ecg_cnt   = 0;
        ecg_qs    = CARDIMETRY_SENSOR_ECG_QUEUE1;

        if(task_state == CARDIMETRY_SENSOR_RUN_TRANSMISSION) {
          cardimetry::cardimetry_sensor_ecg_ts_q1     = "";
          cardimetry::cardimetry_sensor_ecg_lead1_q1  = "";
          cardimetry::cardimetry_sensor_ecg_lead2_q1  = "";
          cardimetry::cardimetry_sensor_ecg_lead3_q1  = "";
          cardimetry::cardimetry_sensor_ecg_ts_q2     = "";
          cardimetry::cardimetry_sensor_ecg_lead1_q2  = "";
          cardimetry::cardimetry_sensor_ecg_lead2_q2  = "";
          cardimetry::cardimetry_sensor_ecg_lead3_q2  = "";
        }

        task_state  = (imu_state == CARDIMETRY_SENSOR_HALT) ? CARDIMETRY_SENSOR_IDLE : CARDIMETRY_SENSOR_RUN_TRANSMISSION;
        break;


      case CARDIMETRY_SENSOR_REQ_ECG_READ:
        ecg_state   = CARDIMETRY_SENSOR_READ;
        task_state  = CARDIMETRY_SENSOR_RUN_TRANSMISSION;
        break;


      case CARDIMETRY_SENSOR_REQ_IMU_HALT:
        imu_state = CARDIMETRY_SENSOR_HALT;
        imu_cnt   = 0;
        imu_qs    = CARDIMETRY_SENSOR_IMU_QUEUE1;

        if(task_state == CARDIMETRY_SENSOR_RUN_TRANSMISSION) {
          cardimetry::cardimetry_sensor_imu_ts_q1 = "";
          cardimetry::cardimetry_sensor_imu_qw_q1 = "";
          cardimetry::cardimetry_sensor_imu_qx_q1 = "";
          cardimetry::cardimetry_sensor_imu_qy_q1 = "";
          cardimetry::cardimetry_sensor_imu_qz_q1 = "";
          cardimetry::cardimetry_sensor_imu_ts_q2 = "";
          cardimetry::cardimetry_sensor_imu_qw_q2 = "";
          cardimetry::cardimetry_sensor_imu_qx_q2 = "";
          cardimetry::cardimetry_sensor_imu_qy_q2 = "";
          cardimetry::cardimetry_sensor_imu_qz_q2 = "";
        }

        task_state  = (ecg_state == CARDIMETRY_SENSOR_HALT) ? CARDIMETRY_SENSOR_IDLE : CARDIMETRY_SENSOR_RUN_TRANSMISSION;
        break;


      case CARDIMETRY_SENSOR_REQ_IMU_READ:
        imu_state   = CARDIMETRY_SENSOR_READ;
        task_state  = CARDIMETRY_SENSOR_RUN_TRANSMISSION;
        break;


      case CARDIMETRY_SENSOR_REQ_ECG_TEST:
        task_state  = CARDIMETRY_SENSOR_RUN_ECG_TEST;
        vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_SENSOR_PREP_DELAY_MS));
        break;


      case CARDIMETRY_SENSOR_REQ_IMU_TEST:
        task_state  = CARDIMETRY_SENSOR_RUN_IMU_TEST;
        vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_SENSOR_PREP_DELAY_MS));
        break;


      case CARDIMETRY_SENSOR_REQ_ECG_SYNC:
        task_state  = CARDIMETRY_SENSOR_ECG_SYNC;
        break;
    

      case CARDIMETRY_SENSOR_REQ_RUN_TRANSMISSION:
        ecg_state         = CARDIMETRY_SENSOR_READ;
        imu_state         = CARDIMETRY_SENSOR_READ;
        task_state        = CARDIMETRY_SENSOR_RUN_TRANSMISSION;
        start_trans_time  = millis(); 
        break;


      case CARDIMETRY_SENSOR_REQ_STOP_TRANSMISSION:
        /* Stop ECG*/
        ecg_state = CARDIMETRY_SENSOR_HALT;
        ecg_cnt   = 0;
        ecg_qs    = CARDIMETRY_SENSOR_ECG_QUEUE1;

        if(task_state == CARDIMETRY_SENSOR_RUN_TRANSMISSION) {
          cardimetry::cardimetry_sensor_ecg_ts_q1     = "";
          cardimetry::cardimetry_sensor_ecg_lead1_q1  = "";
          cardimetry::cardimetry_sensor_ecg_lead2_q1  = "";
          cardimetry::cardimetry_sensor_ecg_lead3_q1  = "";
          cardimetry::cardimetry_sensor_ecg_ts_q2     = "";
          cardimetry::cardimetry_sensor_ecg_lead1_q2  = "";
          cardimetry::cardimetry_sensor_ecg_lead2_q2  = "";
          cardimetry::cardimetry_sensor_ecg_lead3_q2  = "";
        }

        /* Stop IMU */
        imu_state = CARDIMETRY_SENSOR_HALT;
        imu_cnt   = 0;
        imu_qs    = CARDIMETRY_SENSOR_IMU_QUEUE1;

        if(task_state == CARDIMETRY_SENSOR_RUN_TRANSMISSION) {
          cardimetry::cardimetry_sensor_imu_ts_q1 = "";
          cardimetry::cardimetry_sensor_imu_qw_q1 = "";
          cardimetry::cardimetry_sensor_imu_qx_q1 = "";
          cardimetry::cardimetry_sensor_imu_qy_q1 = "";
          cardimetry::cardimetry_sensor_imu_qz_q1 = "";
          cardimetry::cardimetry_sensor_imu_ts_q2 = "";
          cardimetry::cardimetry_sensor_imu_qw_q2 = "";
          cardimetry::cardimetry_sensor_imu_qx_q2 = "";
          cardimetry::cardimetry_sensor_imu_qy_q2 = "";
          cardimetry::cardimetry_sensor_imu_qz_q2 = "";
        }

        task_state = CARDIMETRY_SENSOR_IDLE;
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

        task_state      = CARDIMETRY_SENSOR_IDLE;
        break;




      case CARDIMETRY_SENSOR_RUN_TRANSMISSION:
        
        /* Check for ecg */
        if(ecg_state == CARDIMETRY_SENSOR_READ) {

          /* Read ecg sensor */
          if(!digitalRead(CARDIMETRY_SENSOR_ECG_DRDY_PIN)) {
            xSemaphoreTake(cardimetry::cardimetry_spi_mutex, portMAX_DELAY);
            lead1 = cm_sensor.ads.getECGdata(1);
            lead2 = cm_sensor.ads.getECGdata(2);
            xSemaphoreGive(cardimetry::cardimetry_spi_mutex);
            lead3 = lead1 - lead2;

            /* Insert to queue */
            if(ecg_qs == CARDIMETRY_SENSOR_ECG_QUEUE1) {
              cardimetry::cardimetry_sensor_ecg_ts_q1     += String(millis() - start_trans_time) + String(",");
              cardimetry::cardimetry_sensor_ecg_lead1_q1  += String(lead1) + String(",");
              cardimetry::cardimetry_sensor_ecg_lead2_q1  += String(lead2) + String(",");
              cardimetry::cardimetry_sensor_ecg_lead3_q1  += String(lead3) + String(",");
            }
            else if(ecg_qs == CARDIMETRY_SENSOR_ECG_QUEUE2) {
              cardimetry::cardimetry_sensor_ecg_ts_q2     += String(millis() - start_trans_time) + String(",");
              cardimetry::cardimetry_sensor_ecg_lead1_q2  += String(lead1) + String(",");
              cardimetry::cardimetry_sensor_ecg_lead2_q2  += String(lead2) + String(",");
              cardimetry::cardimetry_sensor_ecg_lead3_q2  += String(lead3) + String(",");
            }
            ++ecg_cnt;
          }

          /* Transmit queue if full */
          if(ecg_cnt == CARDIMETRY_SENSOR_ECG_TRANSMISSION_MAX_QUEUE) {

            /* Call req for transmit */
            if(ecg_qs == CARDIMETRY_SENSOR_ECG_QUEUE1)      out_req = CARDIMETRY_CONN_REQ_PUB_ECG1;
            else if(ecg_qs == CARDIMETRY_SENSOR_ECG_QUEUE2) out_req = CARDIMETRY_CONN_REQ_PUB_ECG2;
            xQueueSend(cardimetry::cardimetry_conn_req_queue, &out_req, portMAX_DELAY);

            /* Reset and switch queue */
            ecg_cnt = 0;
            ecg_qs  = !ecg_qs;
          }
        }


        /* Check for imu */
        if(imu_state == CARDIMETRY_SENSOR_READ) {

          /* IMU is sampled once after 2 ECG sampling, hence we use relative count */
          imu_rel_cnt = !imu_rel_cnt;
          if(imu_rel_cnt) {

            cm_sensor.mpu.getEvent(&a, &g, &temp);
            cm_sensor.hmc.getEvent(&m);

            /* Transform to desired form */
            acc_x = a.acceleration.x;
            acc_y = a.acceleration.y;
            acc_z = a.acceleration.z;
            gyr_x = g.gyro.x*CARDIMETRY_SENSOR_IMU_RADS2DPS;
            gyr_y = g.gyro.y*CARDIMETRY_SENSOR_IMU_RADS2DPS;
            gyr_z = g.gyro.z*CARDIMETRY_SENSOR_IMU_RADS2DPS;
            mag_x = m.magnetic.x;
            mag_y = m.magnetic.y;
            mag_z = m.magnetic.z;

            /* Update filter */
            cm_sensor.filter.update(
              gyr_x, gyr_y, gyr_z,
              acc_x, acc_y, acc_z,
              mag_x, mag_y, mag_z
            );
            cm_sensor.filter.getQuaternion(&qw, &qx, &qy, &qz);
            ++imu_cnt;

            /* Insert to queue */
            if(imu_qs == CARDIMETRY_SENSOR_IMU_QUEUE1) {
              cardimetry::cardimetry_sensor_imu_ts_q1 += String(millis() - start_trans_time) + String(",");
              cardimetry::cardimetry_sensor_imu_qw_q1 += String(qw) + String(",");
              cardimetry::cardimetry_sensor_imu_qx_q1 += String(qx) + String(",");
              cardimetry::cardimetry_sensor_imu_qy_q1 += String(qy) + String(",");
              cardimetry::cardimetry_sensor_imu_qz_q1 += String(qz) + String(",");
            }

            else if(imu_qs == CARDIMETRY_SENSOR_IMU_QUEUE2) {
              cardimetry::cardimetry_sensor_imu_ts_q2 += String(millis() - start_trans_time) + String(",");
              cardimetry::cardimetry_sensor_imu_qw_q2 += String(qw) + String(",");
              cardimetry::cardimetry_sensor_imu_qx_q2 += String(qx) + String(",");
              cardimetry::cardimetry_sensor_imu_qy_q2 += String(qy) + String(",");
              cardimetry::cardimetry_sensor_imu_qz_q2 += String(qz) + String(",");
            }

          
            /* Transmit and change queue */
            if(imu_cnt == CARDIMETRY_SENSOR_IMU_TRANSMISSION_MAX_QUEUE) {
              
              if(imu_qs == CARDIMETRY_SENSOR_IMU_QUEUE1) {
                out_req = CARDIMETRY_CONN_REQ_PUB_IMU1;
              }

              else if(imu_qs == CARDIMETRY_SENSOR_IMU_QUEUE2) {
                out_req = CARDIMETRY_CONN_REQ_PUB_IMU2;
              }

              xQueueSend(cardimetry::cardimetry_conn_req_queue, &out_req, portMAX_DELAY);

              imu_cnt = 0;
              imu_qs  = !imu_qs;
            }
          }
        }

        break;




      case CARDIMETRY_SENSOR_RUN_ECG_TEST:

        /* Read ECG sensor */
        if(digitalRead(CARDIMETRY_SENSOR_ECG_DRDY_PIN) == false) {
          xSemaphoreTake(cardimetry::cardimetry_spi_mutex, portMAX_DELAY);
          lead1 = cm_sensor.ads.getECGdata(1);
          lead2 = cm_sensor.ads.getECGdata(2);
          xSemaphoreGive(cardimetry::cardimetry_spi_mutex);
          lead3 = lead1 - lead2;

          /* Insert to queue */
          if(ecg_qs == CARDIMETRY_SENSOR_ECG_QUEUE1) {
            cardimetry::cardimetry_sensor_ecg_ts_qt1[ecg_cnt]     = millis();
            cardimetry::cardimetry_sensor_ecg_lead1_qt1[ecg_cnt]  = lead1/CARDIMETRY_SENSOR_ECG_LVL2MUV;
            cardimetry::cardimetry_sensor_ecg_lead2_qt1[ecg_cnt]  = lead2/CARDIMETRY_SENSOR_ECG_LVL2MUV;
            cardimetry::cardimetry_sensor_ecg_lead3_qt1[ecg_cnt]  = lead3/CARDIMETRY_SENSOR_ECG_LVL2MUV;
          }
          else if(ecg_qs == CARDIMETRY_SENSOR_ECG_QUEUE2) {
            cardimetry::cardimetry_sensor_ecg_ts_qt2[ecg_cnt]     = millis();
            cardimetry::cardimetry_sensor_ecg_lead1_qt2[ecg_cnt]  = lead1/CARDIMETRY_SENSOR_ECG_LVL2MUV;
            cardimetry::cardimetry_sensor_ecg_lead2_qt2[ecg_cnt]  = lead2/CARDIMETRY_SENSOR_ECG_LVL2MUV;
            cardimetry::cardimetry_sensor_ecg_lead3_qt2[ecg_cnt]  = lead3/CARDIMETRY_SENSOR_ECG_LVL2MUV;
          }
          ++ecg_cnt;
        }

        /* Transmit queue if full */
        if(ecg_cnt == CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE) {

          /* Call req for transmit */
          if(ecg_qs == CARDIMETRY_SENSOR_ECG_QUEUE1)      out_req = CARDIMETRY_DISPLAY_REQ_PLOT_ECG1;
          else if(ecg_qs == CARDIMETRY_SENSOR_ECG_QUEUE2) out_req = CARDIMETRY_DISPLAY_REQ_PLOT_ECG2;
          xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);

          /* Reset and switch queue */
          ecg_cnt = 0;
          ecg_qs  = !ecg_qs;
        }
        break;




      case CARDIMETRY_SENSOR_RUN_IMU_TEST:

        /* Relative count 2 from Ecg sampling */
        imu_rel_cnt = !imu_rel_cnt;
        if(imu_rel_cnt) {

          cm_sensor.mpu.getEvent(&a, &g, &temp);
          cm_sensor.hmc.getEvent(&m);

          /* Transform to desired form */
          acc_x = a.acceleration.x;
          acc_y = a.acceleration.y;
          acc_z = a.acceleration.z;
          gyr_x = g.gyro.x*CARDIMETRY_SENSOR_IMU_RADS2DPS;
          gyr_y = g.gyro.y*CARDIMETRY_SENSOR_IMU_RADS2DPS;
          gyr_z = g.gyro.z*CARDIMETRY_SENSOR_IMU_RADS2DPS;
          mag_x = m.magnetic.x;
          mag_y = m.magnetic.y;
          mag_z = m.magnetic.z;

          /* Update filter */
          cm_sensor.filter.update(
            gyr_x, gyr_y, gyr_z,
            acc_x, acc_y, acc_z,
            mag_x, mag_y, mag_z
          );

          /* Insert to queue */
          if(imu_qs == CARDIMETRY_SENSOR_IMU_QUEUE1) {
            cardimetry::cardimetry_sensor_imu_ts_qt1[imu_cnt] = millis();
            cm_sensor.filter.getQuaternion(
              (cardimetry::cardimetry_sensor_imu_qw_qt1 + imu_cnt),
              (cardimetry::cardimetry_sensor_imu_qx_qt1 + imu_cnt),
              (cardimetry::cardimetry_sensor_imu_qy_qt1 + imu_cnt),
              (cardimetry::cardimetry_sensor_imu_qz_qt1 + imu_cnt)
            );
          }
          else if(imu_qs == CARDIMETRY_SENSOR_IMU_QUEUE2) {
            cardimetry::cardimetry_sensor_imu_ts_qt2[imu_cnt] = millis();
            cm_sensor.filter.getQuaternion(
              (cardimetry::cardimetry_sensor_imu_qw_qt2 + imu_cnt),
              (cardimetry::cardimetry_sensor_imu_qx_qt2 + imu_cnt),
              (cardimetry::cardimetry_sensor_imu_qy_qt2 + imu_cnt),
              (cardimetry::cardimetry_sensor_imu_qz_qt2 + imu_cnt)
            );
          }
          ++imu_cnt;
        }

        /* Transmit queue if full */
        if(imu_cnt == CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE) {

          /* Call req for transmit */
          if(imu_qs == CARDIMETRY_SENSOR_IMU_QUEUE1)      out_req = CARDIMETRY_DISPLAY_REQ_PLOT_IMU1;
          else if(imu_qs == CARDIMETRY_SENSOR_IMU_QUEUE2) out_req = CARDIMETRY_DISPLAY_REQ_PLOT_IMU2;
          xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);

          /* Reset and switch queue */
          imu_cnt = 0;
          imu_qs  = !imu_qs;
        } 
        break;




      case CARDIMETRY_SENSOR_ECG_SYNC:

        /* Run ecg sync */
        if(cm_sensor.ecgSync()) {
          out_req = CARDIMETRY_DISPLAY_REQ_ECG_SYNC_SUCCESS;
          xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
        }
        else {
          out_req = CARDIMETRY_DISPLAY_REQ_ECG_SYNC_FAILED;
          xQueueSend(cardimetry::cardimetry_display_req_queue, &out_req, portMAX_DELAY);
        }

        task_state = CARDIMETRY_SENSOR_IDLE;
        break;
    }

    vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_SENSOR_TASK_DELAY));
  }
}