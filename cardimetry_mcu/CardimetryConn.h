#ifndef __CARDIMETRY_CONN_H__
#define __CARDIMETRY_CONN_H__

#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <WiFi.h>
#include "time.h"
#include "sntp.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"

#define CARDIMETRY_CONN_NTP_SERVER_1        "pool.ntp.org"
#define CARIDMETRY_CONN_NTP_SERVER_2        "time.nist.gov"
#define CARDIMETRY_CONN_NTP_GMT_OFFSET_S    21600
#define CARDIMETRY_CONN_NTP_DL_OFFSET_S     3600
#define CARDIMETRY_CONN_BASE_SERVICE_URL    "http://192.168.100.176:8000"
#define CARDIMETRY_CONN_API_PATIENT_SEARCH  "/api/v1/patient/search?name="
#define CARDIMETRY_CONN_MQTT_PORT           1883
#define CARDIMETRY_CONN_HTTP_PORT           80

#define CARDIMETRY_CONN_IDLE            0
#define CARDIMETRY_CONN_WIFI_SCAN       1
#define CARDIMETRY_CONN_WIFI_CONNECT    2
#define CARDIMETRY_CONN_PATIENT_SEARCH  3

#define CARDIMETRY_CONN_WIFI_SCAN_UNDONE  -69
#define CARDIMETRY_CONN_WIFI_SCAN_MAX     7

#define CARDIMETRY_CONN_WIFI_WAIT_TIME_MS       100
#define CARDIMETRY_CONN_WIFI_CONNECT_TIMEOUT_MS 5000

#define CARDIMETRY_CONN_MAX_PATIENT_SEARCH 50

#define CARDIMETRY_CONN_BAT_PIN       36
#define CARDIMETRY_CONN_BAT_FULL      1890.0
#define CARDIMETRY_CONN_BAT_LOW       1600.0
#define CARDIMETRY_CONN_BAT_TOLERANCE 1.4
#define CARDIMETRY_CONN_BAT_COMPFILT  0.075

#define CARDIMETRY_CONN_RSSI_3  -50
#define CARDIMETRY_CONN_RSSI_2  -70
#define CARDIMETRY_CONN_RSSI_1  -90


namespace cardimetry{

  class CardimetryConn{

    public:
      CardimetryConn();
      ~CardimetryConn();
      void begin();
      int16_t scanWiFi();
      void saveConnWiFiTable(fs::FS &fs, String ssid, String pass);
      bool getPatientData(uint16_t id[], String name[], String key);
  };
}


#endif