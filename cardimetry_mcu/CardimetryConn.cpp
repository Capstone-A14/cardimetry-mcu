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




void cardimetry::CardimetryConn::saveConnWiFiTable(fs::FS &fs, String ssid, String pass) {
  if(!fs.exists(F("/cmwifitable.json"))) {

    /* Create new file */
    File cmwifitable = fs.open(F("/cmwifitable.json"), FILE_WRITE);

    /* Create Json document */
    DynamicJsonDocument json_doc(128);
    json_doc[ssid] = pass;
    String json_str;
    serializeJson(json_doc, json_str);

    /* Write the json */
    cmwifitable.println(json_str);
    cmwifitable.close();
  }

  else {

    /* Open the existing file */
    File cmwifitable = fs.open(F("/cmwifitable.json"), FILE_READ);
    String json_data = cmwifitable.readString();
    cmwifitable.close();

    /* Recreate the json document */
    DynamicJsonDocument json_doc(1024);
    deserializeJson(json_doc, json_data);
    json_doc[ssid] = pass;

    /* Serialize */
    String new_json_data;
    serializeJson(json_doc, new_json_data);

    /* Overwrite the file */
    File new_cmwifitable = fs.open(F("/cmwifitable.json"), FILE_WRITE);
    new_cmwifitable.println(new_json_data);
    new_cmwifitable.close();
  }
}