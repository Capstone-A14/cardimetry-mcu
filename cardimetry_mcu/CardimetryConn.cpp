#include "CardimetryConn.h"




cardimetry::CardimetryConn::CardimetryConn() {}
cardimetry::CardimetryConn::~CardimetryConn() {}




void cardimetry::CardimetryConn::begin() {

  /* NTP config */
  sntp_servermode_dhcp(1);
  configTime(
    CARDIMETRY_CONN_NTP_GMT_OFFSET_S, 
    CARDIMETRY_CONN_NTP_DL_OFFSET_S, 
    CARDIMETRY_CONN_NTP_SERVER_1, 
    CARIDMETRY_CONN_NTP_SERVER_2
  );


  /* Start ESP32 WiFi as station */
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_CONN_WIFI_WAIT_TIME_MS));


  /* Start MQTT */
  // mqtt_client.enableDebuggingMessages();
  mqtt_client.setMaxPacketSize(CARDIMETRY_CONN_MQTT_BUFFER_SIZE);
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
    DynamicJsonDocument json_doc(2048);
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




bool cardimetry::CardimetryConn::linkDevice() {
  String uid = this->getUID(SD);
  HTTPClient http;
  http.begin(
    String("http://") + String(CARDIMETRY_CONN_BASE_SERVICE_IP) + String(":8000") + String(CARDIMETRY_CONN_API_LINK_DEVICE) + uid
  );

  /* POST */
  int http_code = http.POST("");
  if(http_code == HTTP_CODE_OK) return true;
  else return false;
}




bool cardimetry::CardimetryConn::linkPatient(uint16_t patient_id) {
  String uid = this->getUID(SD);
  HTTPClient http;
  http.begin(
    String("http://") + String(CARDIMETRY_CONN_BASE_SERVICE_IP) + String(":8000") + String(CARDIMETRY_CONN_API_LINK_DEVICE) + uid + String("/") + patient_id
  );

  /* POST */
  int http_code = http.POST("");
  if(http_code == HTTP_CODE_OK) return true;
  else return false;
}




bool cardimetry::CardimetryConn::getPatientData(uint16_t* num, uint16_t id_buf[], String name_buf[], String key) {
  
  /* Start HTTP client */
  HTTPClient http;
  http.begin(
    String("http://") + String(CARDIMETRY_CONN_BASE_SERVICE_IP) + String(":8000") + String(CARDIMETRY_CONN_API_PATIENT_SEARCH) + key
  );

  /* GET */
  int http_code = http.GET();
  
  if(http_code == HTTP_CODE_OK) {
    
    /* Get the data and parse the json */
    String payload = http.getString();
    DynamicJsonDocument json_doc(4096);
    deserializeJson(json_doc, payload);
    *num = json_doc.size();

    if(*num == 0) return false;

    for(uint8_t i = 0; i < (uint8_t)min((int)CARDIMETRY_CONN_MAX_PATIENT_SEARCH, (int)json_doc.size()); ++i) {
      id_buf[i]   = (uint8_t)json_doc[i]["patient_id"].as<int>();
      name_buf[i] = json_doc[i]["name"].as<String>();
    }

    /* Close and return true */
    http.end();
    return true;
  }

  else {
    return false;
  }

  return false;
}




String cardimetry::CardimetryConn::getUID(fs::FS &fs) {
  if(fs.exists(F("/cmconfig.json"))) {

    /* Open config file */
    File cmconfig = fs.open(F("/cmconfig.json"), FILE_READ);
    String json_str = cmconfig.readString();
    cmconfig.close();

    /* Deserialize Json */
    DynamicJsonDocument json_doc(1024);
    deserializeJson(json_doc, json_str);
    return json_doc["uid"].as<String>();
  }
  else return "cmxxx";
}




void onConnectionEstablished() {}