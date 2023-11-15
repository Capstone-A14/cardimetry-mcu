#ifndef __TIME_TEST_H__
#define __TIME_TEST_H__

#include <WiFi.h>
#include "time.h"
#include "sntp.h"

#define WIFI_SSID     "DhonanAP_HP"
#define WIFI_PASS     "12345678"
#define NTP_SERVER_1  "pool.ntp.org"
#define NTP_SERVER_2  "time.nist.gov"
#define GMT_OFFSET_S  21600
#define DL_OFFSET_S   3600





void time_begin() {
  /* Time configuration */
  sntp_servermode_dhcp(1);
  configTime(GMT_OFFSET_S, DL_OFFSET_S, NTP_SERVER_1, NTP_SERVER_2);

  /* Start WiFi */
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("WiFi Connected!");
}



void time_loop() {
  delay(1000);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}



#endif