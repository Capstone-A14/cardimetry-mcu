#include "protocentral_ads1293.h"
#include <SPI.h>

#define DRDY_PIN                35
#define CS_PIN                  32

ads1293 ADS1293(DRDY_PIN, CS_PIN);

bool drdyIntFlag = false;

void setup() {

  Serial.begin(115200);
 // initalize the  data ready and chip select pins:
  // pinMode(DRDY_PIN, INPUT);
  pinMode(CS_PIN, OUTPUT);  
  SPI.begin();

  //pinMode(4,OUTPUT);
  //digitalWrite(4,LOW);
  ADS1293.ads1293Begin3LeadECG();
  //enableInterruptPin();
  delay(5);
}

void loop() {

  //if (drdyIntFlag) {
  if (digitalRead(ADS1293.drdyPin) == false){

    //digitalWrite(4,HIGH);
    drdyIntFlag = false;
    int32_t ecg = ADS1293.getECGdata(1);
    int32_t ecg2 = ADS1293.getECGdata(2);
    int32_t ecg3 =ecg2-ecg;     
    Serial.print(ecg);Serial.print(',');Serial.print(ecg2);Serial.print(',');Serial.println(ecg3);
    //digitalWrite(4,LOW);
  }
  delay(5);
}
