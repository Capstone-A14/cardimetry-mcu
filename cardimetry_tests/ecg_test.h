#ifndef __ECG_TEST_H__
#define __ECG_TEST_H__

#include <SPI.h>
#include "protocentral_ads1293.h"

#define DRDY_PIN  35
#define CS_PIN    32
#define LED_PIN   33

ads1293 ADS1293(DRDY_PIN, CS_PIN);

void ecg_begin() {
  SPI.begin();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  
  pinMode(CS_PIN, OUTPUT);
  ADS1293.ads1293Begin3LeadECG();
  // ADS1293.readErrorStatus()
  delay(100);
}

void ecg_loop() {
  uint16_t state = digitalRead(DRDY_PIN);
  // Serial.printf("DRDY PIN: %d\n", state);

  // uint8_t err = ADS1293.ads1293ReadRegister(0x18);
  // Serial.printf("ERR_LOD: %d\n", err);
  // err = ADS1293.ads1293ReadRegister(0x19);
  // Serial.printf("ERR_STS: %d\n", err);
  // err = ADS1293.ads1293ReadRegister(0x1A);
  // Serial.printf("ERR_RG1: %d\n", err);
  // err = ADS1293.ads1293ReadRegister(0x1B);
  // Serial.printf("ERR_RG2: %d\n", err);
  // err = ADS1293.ads1293ReadRegister(0x1C);
  // Serial.printf("ERR_RG3: %d\n", err);
  // err = ADS1293.ads1293ReadRegister(0x1D);
  // Serial.printf("ERR_SYN: %d\n", err);
  // err = ADS1293.ads1293ReadRegister(0x1E);
  // Serial.printf("ERR_MIS: %d\n", err);

  if (state == false){

    int32_t ecg1  = ADS1293.getECGdata(1);
    int32_t ecg2  = ADS1293.getECGdata(2);
    int32_t ecg3  = ecg1 - ecg2;     
    Serial.print(ecg1); Serial.print(','); Serial.print(ecg2); Serial.print(','); Serial.println(ecg3);
  }
  delay(10);
}

#endif