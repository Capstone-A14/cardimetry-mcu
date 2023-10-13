#ifndef __CARDIMETRY_CONN_H__
#define __CARDIMETRY_CONN_H__

#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <WiFi.h>
#include "ArduinoJson.h"

#define CARDIMETRY_CONN_IDLE          0
#define CARDIMETRY_CONN_WIFI_SCAN     1
#define CARDIMETRY_CONN_WIFI_CONNECT  2

#define CARDIMETRY_CONN_WIFI_SCAN_UNDONE  -69
#define CARDIMETRY_CONN_WIFI_SCAN_MAX     7

#define CARDIMETRY_CONN_WIFI_WAIT_TIME_MS       100
#define CARDIMETRY_CONN_WIFI_CONNECT_TIMEOUT_MS 5000


namespace cardimetry{

  class CardimetryConn{

    public:
      CardimetryConn();
      ~CardimetryConn();
      void begin();
      int16_t scanWiFi();
      void saveConnWiFiTable(fs::FS &fs, String ssid, String pass);
  };
}


#endif