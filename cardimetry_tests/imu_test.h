#ifndef __IMU_TEST_H__
#define __IMU_TEST_H__

#ifdef __QWIIC__

  #include <Wire.h>
  #include "SparkFun_ISM330DHCX.h"
  #include "SparkFun_MMC5983MA_Arduino_Library.h"
  #include "Adafruit_AHRS.h"

  SparkFun_ISM330DHCX myISM;
  SFE_MMC5983MA myMag;
  Adafruit_Mahony filter;


  // Structs for X,Y,Z data
  sfe_ism_raw_data_t accelData; 
  sfe_ism_raw_data_t gyroData; 


  void imu_begin() {
    Wire.begin();
    Serial.begin(115200);

    if(!myISM.begin()){
      Serial.println("ISM330DHCX did not respond");
      while(1);
    }
    if(!myMag.begin()) {
      Serial.println("MMC5983MA did not respond");
      while (true);
    }

    // Reset ISM
    myISM.deviceReset();
    while(!myISM.getDeviceReset()){ 
      delay(1);
    }

    // Reset MMC
    myMag.softReset();

    Serial.println("Reset.");
    Serial.println("Applying settings.");
    delay(100);
    
    myISM.setDeviceConfig();
    myISM.setBlockDataUpdate();
    
    // Set the output data rate and precision of the accelerometer
    myISM.setAccelDataRate(ISM_XL_ODR_104Hz);
    myISM.setAccelFullScale(ISM_4g); 

    // Set the output data rate and precision of the gyroscope
    myISM.setGyroDataRate(ISM_GY_ODR_104Hz);
    myISM.setGyroFullScale(ISM_500dps); 

    // Turn on the accelerometer's filter and apply settings. 
    myISM.setAccelFilterLP2();
    myISM.setAccelSlopeFilter(ISM_LP_ODR_DIV_100);

    // Turn on the gyroscope's filter and apply settings. 
    myISM.setGyroFilterLP1();
    myISM.setGyroLP1Bandwidth(ISM_STRONG);

    filter.begin(50);
  }


  void imu_loop() {
    if( myISM.checkStatus() ){
    
      /* Get raw data */
      myISM.getRawAccel(&accelData);
      myISM.getRawGyro(&gyroData);


      /* Calculate sensor in SI unit (ms^(-2) and rad/s)
        * 1 equals to 0.122 mg in accelerometer (4g config)
        * 1 equals to 17.5 mdps in gyroscope (500dps config)
      */
      float acc_x = -((float)accelData.xData)*0.00119682,
            acc_y = -((float)accelData.yData)*0.00119682,
            acc_z = -((float)accelData.zData)*0.00119682,
            gyr_x = ((float)gyroData.xData)*0.0175,
            gyr_y = ((float)gyroData.yData)*0.0175,
            gyr_z = ((float)gyroData.zData)*0.0175,
            mag_x = (((float)myMag.getMeasurementX())/16384.0 - 8.0),
            mag_y = (((float)myMag.getMeasurementY())/16384.0 - 8.0),
            mag_z = (((float)myMag.getMeasurementZ())/16384.0 - 8.0);

      filter.update(
        gyr_x, gyr_y, gyr_z,
        acc_x, acc_y, acc_z,
        mag_x, mag_y, mag_z
      );


      float qw, qx, qy, qz;
      filter.getQuaternion(&qw, &qx, &qy, &qz);
      // Serial.print("qw:"); Serial.print(qw); Serial.print(',');
      // Serial.print("qx:"); Serial.print(qx); Serial.print(',');
      // Serial.print("qy:"); Serial.print(qy); Serial.print(',');
      // Serial.print("qz:"); Serial.print(qz); Serial.println();
      Serial.print(qw); Serial.print(',');
      Serial.print(qx); Serial.print(',');
      Serial.print(qy); Serial.print(',');
      Serial.print(qz); Serial.println();


      // Serial.print("Roll:"); Serial.print(filter.getRoll()); Serial.print(',');
      // Serial.print("Pitch:"); Serial.print(filter.getPitch()); Serial.print(',');
      // Serial.print("Yaw:"); Serial.print(filter.getYaw()); Serial.println();


      // Serial.print("aX:");
      // Serial.print(acc_x);
      // Serial.print(",");
      // Serial.print("aY:");
      // Serial.print(acc_y);
      // Serial.print(",");
      // Serial.print("aZ:");
      // Serial.print(acc_z);
      // Serial.print(",");
      // Serial.print("gX:");
      // Serial.print(gyr_x);
      // Serial.print(",");
      // Serial.print("gY:");
      // Serial.print(gyr_y);
      // Serial.print(",");
      // Serial.print("gZ:");
      // Serial.print(gyr_z);
      // Serial.print(",");
      // Serial.print("mX:");
      // Serial.print(mag_x);
      // Serial.print(",");
      // Serial.print("mY:");
      // Serial.print(mag_y);
      // Serial.print(",");
      // Serial.print("mZ:");
      // Serial.print(mag_z);
      // Serial.println("");
    }

    delay(20);
  }

#endif

#ifdef __MPU6050__

  #include <Wire.h>
  #include "Adafruit_Sensor.h"
  #include "Adafruit_MPU6050.h"
  #include "Adafruit_HMC5883_U.h"
  #include "Adafruit_AHRS.h"


  Adafruit_Mahony filter;
  Adafruit_MPU6050 mpu;
  Adafruit_HMC5883_Unified hmc = Adafruit_HMC5883_Unified(12345);


  void imu_begin() {
    
    /* Start MPU */
    mpu.begin();
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    /* Start HMC */
    hmc.begin();

    /* Start Filter */
    filter.begin(50);
  }


  void imu_loop() {
    sensors_event_t a, g, m, temp;
    mpu.getEvent(&a, &g, &temp);
    hmc.getEvent(&m);

    float ax  = a.acceleration.x,
          ay  = a.acceleration.y,
          az  = a.acceleration.z,
          gx  = g.gyro.x*57.29577793F,
          gy  = g.gyro.y*57.29577793F,
          gz  = g.gyro.z*57.29577793F,
          mx  = m.magnetic.x,
          my  = m.magnetic.y,
          mz  = m.magnetic.z;

    filter.update(
      gx, gy, gz,
      ax, ay, az,
      mx, my, mz
    );

    float qw, qx, qy, qz;
    filter.getQuaternion(&qw, &qx, &qy, &qz);
    Serial.print(qw); Serial.print(',');
    Serial.print(qx); Serial.print(',');
    Serial.print(qy); Serial.print(',');
    Serial.print(qz); Serial.println();


    /* Delay to sync the filter sampling freq. */
    delay(20);
  }


#endif

#endif