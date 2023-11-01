#ifndef __CARDIMETRY_SENSOR_H__
#define __CARDIMETRY_SENSOR_H__

#include <Wire.h>
#include <SPI.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_MPU6050.h"
#include "Adafruit_HMC5883_U.h"
// #include "SparkFun_ISM330DHCX.h"
// #include "SparkFun_MMC5983MA_Arduino_Library.h"
#include "Adafruit_AHRS.h"
#include "protocentral_ads1293.h"

#define CARDIMETRY_SENSOR_ECG_DRDY_PIN  35
#define CARDIMETRY_SENSOR_ECG_CS_PIN    32

#define CARDIMETRY_SENSOR_ECG_RATE_MS 10
#define CARDIMETRY_SENSOR_IMU_RATE_MS 20
#define CARDIMETRY_SENSOR_IMU_FREQ    50

#define CARDIMETRY_SENSOR_ECG_TRANSMISSION_MAX_QUEUE  100
#define CARDIMETRY_SENSOR_IMU_TRANSMISSION_MAX_QUEUE  50
#define CARDIMETRY_SENSOR_ECG_TEST_MAX_QUEUE          50
#define CARDIMETRY_SENSOR_IMU_TEST_MAX_QUEUE          25

#define CARDIMETRY_SENSOR_ECG_QUEUE1  false
#define CARDIMETRY_SENSOR_ECG_QUEUE2  true
#define CARDIMETRY_SENSOR_IMU_QUEUE1  false
#define CARDIMETRY_SENSOR_IMU_QUEUE2  true

#define CARDIMETRY_SENSOR_INIT_DELAY_MS     400
#define CARDIMETRY_SENSOR_ECG_SYNC_DELAY_MS 100

#define CARDIMETRY_SENSOR_HALT  0
#define CARDIMETRY_SENSOR_READ  1

#define CARDIMETRY_SENSOR_IDLE              0
#define CARDIMETRY_SENSOR_INIT              1
#define CARDIMETRY_SENSOR_RUN_TRANSMISSION  2
#define CARDIMETRY_SENSOR_RUN_ECG_TEST      3
#define CARDIMETRY_SENSOR_RUN_IMU_TEST      4
#define CARDIMETRY_SENSOR_ECG_SYNC          5

#define CARDIMETRY_SENSOR_ECG_LVL2MUV   15435
#define CARDIMETRY_SENSOR_IMU_RADS2DPS  57.29577793F
#define CARDIMETRY_SENSOR_PREP_DELAY_MS 700
// #define CARDIMETRY_SENSOR_ACC_COEF      0.00119682
// #define CARDIMETRY_SENSOR_GYR_COEF      0.0175
// #define CARDIMETRY_SENSOR_MAG_DIV       16384.0
// #define CARDIMETRY_SENSOR_MAG_OFFSET    8.0


namespace cardimetry {


  class CardimetrySensor {
    
    
    private:
      // sfe_ism_raw_data_t  acc_data,
      //                     gyro_data;
    
    public:
      // SparkFun_ISM330DHCX ism;
      // SFE_MMC5983MA       mmc;
      Adafruit_MPU6050          mpu;
      Adafruit_HMC5883_Unified  hmc = Adafruit_HMC5883_Unified(12345);;
      Adafruit_Mahony           filter;
      ads1293                   ads = ads1293(CARDIMETRY_SENSOR_ECG_DRDY_PIN, CARDIMETRY_SENSOR_ECG_CS_PIN);

      CardimetrySensor();
      ~CardimetrySensor();
      bool begin();
      bool ecgSync();
  };
}


#endif