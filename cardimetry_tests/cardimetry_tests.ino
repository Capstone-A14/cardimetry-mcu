// #include "touch_test.h"
// #include "wifiscan_test.h"
#include "download_test.h"

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
}