#ifndef __CARDIMETRY_SENSOR_H__
#define __CARDIMETRY_SENSOR_H__

#include <Wire.h>
#include <SPI.h>
#include "SparkFun_ISM330DHCX.h"
#include "SparkFun_MMC5983MA_Arduino_Library.h"
#include "Adafruit_AHRS.h"
#include "protocentral_ads1293.h"

#define CARDIMETRY_SENSOR_ECG_DRDY_PIN  35
#define CARDIMETRY_SENSOR_ECG_CS_PIN    32

#define CARDIMETRY_SENSOR_ECG_RATE_MS 10
#define CARDIMETRY_SENSOR_IMU_RATE_MS 20
#define CARDIMETRY_SENSOR_IMU_FREQ    50

#define CARDIMETRY_SENSOR_INIT_DELAY_MS  1000

#define CARDIMETRY_SENSOR_HALT  0
#define CARDIMETRY_SENSOR_READ  1

#define CARDIMETRY_SENSOR_IDLE    0
#define CARDIMETRY_SENSOR_INIT    1
#define CARDIMETRY_SENSOR_RUN     2


namespace cardimetry {


  class CardimetrySensor {
    
    
    private:
      sfe_ism_raw_data_t  acc_data,
                          gyro_data;
    
    public:
      SparkFun_ISM330DHCX ism;
      SFE_MMC5983MA       mmc;
      Adafruit_Mahony     filter;
      ads1293             ads = ads1293(CARDIMETRY_SENSOR_ECG_DRDY_PIN, CARDIMETRY_SENSOR_ECG_CS_PIN);

      CardimetrySensor();
      ~CardimetrySensor();
      bool begin();
  };
}


#endif