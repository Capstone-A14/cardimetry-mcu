// #include "touch_test.h"
// #include "wifiscan_test.h"
// #include "download_test.h"
// #include "transmit_test.h"
// #include "time_test.h"
// #include "png_test.h"
#include "imu_test.h"

void setup() {
  
  Serial.begin(115200);

  #ifdef __TOUCH_TEST_H__
    touch_begin();
  #endif

  #ifdef __WIFISCAN_TEST_H__
    wifi_begin();
  #endif

  #ifdef __DOWNLOAD_TEST_H__
    download_begin();
  #endif

  #ifdef __TRANSMIT_TEST_H__
    transmit_begin();
  #endif

  #ifdef __TIME_TEST_H__
    time_begin();
  #endif

  #ifdef __PNG_TEST_H__
    png_begin();
  #endif

  #ifdef __IMU_TEST_H__
    imu_begin();
  #endif
}

void loop() {

  #ifdef __TOUCH_TEST_H__
    touch_print();
  #endif

  #ifdef __WIFISCAN_TEST_H__
    wifi_print();
  #endif

  #ifdef __DOWNLOAD_TEST_H__
    download_loop();
  #endif

  #ifdef __TRANSMIT_TEST_H__
    transmit_loop();
  #endif

  #ifdef __TIME_TEST_H__
    time_loop();
  #endif

  #ifdef __PNG_TEST_H__
    png_loop();
  #endif

  #ifdef __IMU_TEST_H__
    imu_loop();
  #endif
}