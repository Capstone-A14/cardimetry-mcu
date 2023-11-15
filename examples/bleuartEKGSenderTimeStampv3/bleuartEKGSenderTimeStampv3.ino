/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include "protocentral_ads1293.h"
#include <SPI.h>
#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>



#define DRDY_PIN                D2
#define CS_PIN                  D3

ads1293 ADS1293(DRDY_PIN, CS_PIN);

bool drdyIntFlag = false;
bool sensorflag = true;

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  uint8_t a[10];
  uint8_t b[10];
} struct_message;

// Create a struct_message called myData
struct_message myData;

uint32_t ch1=0,ch2=0;
uint8_t cnt=0;
unsigned long ts0,ts;
void setup()
{
  Serial.begin(115200);
  //pinMode( BLE_LED_PIN, OUTPUT );
  //SPI and ADS init
  SPI.begin();
 // initalize the  data ready and chip select pins:
  pinMode(DRDY_PIN, INPUT);
  pinMode(CS_PIN, OUTPUT);
  ADS1293.ads1293Begin3LeadECG();
  delay(5);
  //end of SPI and ADS init
#if CFG_DEBUG
  // Blocking wait for connection when debug mode is enabled via IDE
  while ( !Serial ) yield();
#endif
  
  Serial.println("Bluefruit52 BLEUART Example");
  Serial.println("---------------------------\n");

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behavior, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  startAdv();

  Serial.println("Please use Adafruit's Bluefruit LE app to connect in UART mode");
  Serial.println("Once connected, enter character(s) that you wish to send");

  //strcpy(myData.a, "abcdefghi");
  //strcpy(myData.b, "jklmnopqr");
  ts0=millis(); 
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void loop()
{
  if (digitalRead(ADS1293.drdyPin) == false){
    if(cnt==0){
      ch1=ADS1293.getECGdata(1);
      myData.a[0]=(uint8_t)(ch1 & 0xff);
      myData.a[1]=(uint8_t)((ch1>>8) & 0xff);
      myData.a[2]=(uint8_t)((ch1>>16) & 0xff);
      ch2=ADS1293.getECGdata(2);
      myData.b[0]=(uint8_t)(ch2 & 0xff);
      myData.b[1]=(uint8_t)((ch2>>8) & 0xff);
      myData.b[2]=(uint8_t)((ch2>>16) & 0xff); 
      //timestamp
      ts=millis()-ts0;
      myData.a[6]=(uint8_t)(ts & 0xff);
      myData.a[7]=(uint8_t)((ts>>8) & 0xff);
      myData.a[8]=(uint8_t)((ts>>16) & 0xff);
      cnt++; 
    } else if(cnt==1){
      ch1=ADS1293.getECGdata(1);
      myData.a[3]=(uint8_t)(ch1 & 0xff);
      myData.a[4]=(uint8_t)((ch1>>8) & 0xff);
      myData.a[5]=(uint8_t)((ch1>>16) & 0xff);
      ch2=ADS1293.getECGdata(2);
      myData.b[3]=(uint8_t)(ch2 & 0xff);
      myData.b[4]=(uint8_t)((ch2>>8) & 0xff);
      myData.b[5]=(uint8_t)((ch2>>16) & 0xff);  
      //timestamp
      ts=millis()-ts0;
      myData.b[6]=(uint8_t)(ts & 0xff);
      myData.b[7]=(uint8_t)((ts>>8) & 0xff);
      myData.b[8]=(uint8_t)((ts>>16) & 0xff);      
      cnt=0;  
      bleuart.write((uint8_t *) &myData, sizeof(myData));
    } 
  }
  
 
  //delay(10);
  
  /*ch1+=1000;
  ch2+=2000;
  if(ch1>=100000){
    ch1=0;
  }
  if(ch2>=100000){
    ch2=0;
  }*/
  
  //myData.b+=1;
  //myData.c+=0.01; 
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}