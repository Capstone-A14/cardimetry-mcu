#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_MPU6050.h"
#include "Adafruit_HMC5883_U.h"
#include "Adafruit_AHRS.h"
#include "Adafruit_GFX.h"
#include "Adafruit_FT6206.h"
#include "TFT_eSPI.h"



Adafruit_Mahony           filter;
Adafruit_MPU6050          mpu;
Adafruit_HMC5883_Unified  hmc = Adafruit_HMC5883_Unified(12345);
TFT_eSPI                  tft = TFT_eSPI();
Adafruit_FT6206           ctp = Adafruit_FT6206();


String    file_name = "/data/har_dataset";
int16_t   tc_x, tc_y;
int32_t   idx = 0;
bool      is_recording  = false,
          is_init       = true;


File file_data;



void setup() {
  SPI.begin();
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(0xFFFF);
  ctp.begin(40, &Wire);

  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);

  /* Start SD card */
  if(!SD.begin(16)) {
    while(1);
  }
  if(SD.exists("/data")) {
    Serial.println("Data folder exists!");
  }
  while(true) {
    if(!SD.exists(file_name + String(idx) + String(".csv"))) {
      file_name = file_name + String(idx) + String(".csv");
      break;
    }
    ++idx;
  }
  tft.fillRoundRect(190, 190, 100, 30, 10, 0x0000);
  tft.setTextSize(2);
  tft.setTextColor(0x0000, 0xFFFF);
  tft.drawCentreString(String("Recording on ") + file_name, 240, 150, 1);
  tft.setTextColor(0xFFFF, 0x0000);
  tft.drawCentreString("START", 240, 198, 1);

  /* Start MPU */
  mpu.begin();
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  /* Start HMC */
  hmc.begin();

  /* Start Filter */
  filter.begin(50);

  delay(100);
}



void loop() {

  if(ctp.touched()) {
    TS_Point tp = ctp.getPoint();
    tc_x  = tp.y;
    tc_y  = 320 - tp.x;

    if(190 <= tc_x && tc_x <= 290 && 190 <= tc_y && tc_y <= 220) {
      is_recording = !is_recording;

      if(is_recording) {
        tft.fillRoundRect(190, 190, 100, 30, 10, 0xA000);
        tft.setTextSize(2);
        tft.setTextColor(0xFFFF, 0xA000);
        tft.drawCentreString("STOP", 240, 198, 1);

        file_data = SD.open(file_name, FILE_WRITE);
      }

      else {
        tft.fillScreen(0xFFFF);
        tft.fillRoundRect(190, 190, 100, 30, 10, 0x0000);
        tft.setTextSize(2);
        tft.setTextColor(0xFFFF, 0x0000);
        tft.drawCentreString("START", 240, 198, 1);

        ++idx;
        file_name = String("/data/har_dataset") + String(idx) + String(".csv");
        tft.setTextColor(0x0000, 0xFFFF);
        tft.drawCentreString(String("Recording on ") + file_name, 240, 150, 1);

        file_data.close();
      }

      delay(300);
    }
  }


  if(is_recording) {
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
    file_data.print(millis()); file_data.print(',');
    file_data.print(qw); file_data.print(',');
    file_data.print(qx); file_data.print(',');
    file_data.print(qy); file_data.print(',');
    file_data.print(qz); file_data.println();


    /* Delay to sync the filter sampling freq. */
    delay(20);
  }
}

