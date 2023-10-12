#include "CardimetryConn.h"




cardimetry::CardimetryConn::CardimetryConn() {}
cardimetry::CardimetryConn::~CardimetryConn() {}




void cardimetry::CardimetryConn::begin() {

  /* Start ESP32 WiFi as station */
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_CONN_WIFI_WAIT_TIME_MS));
}




int16_t cardimetry::CardimetryConn::scanWiFi() {
  return WiFi.scanNetworks();
}