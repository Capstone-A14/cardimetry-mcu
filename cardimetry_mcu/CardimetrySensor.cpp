#include "CardimetrySensor.h"




cardimetry::CardimetrySensor::CardimetrySensor() {}
cardimetry::CardimetrySensor::~CardimetrySensor() {}




bool cardimetry::CardimetrySensor::begin() {
  Wire.begin();
  SPI.begin();
  pinMode(CARDIMETRY_SENSOR_ECG_CS_PIN, OUTPUT);


  /* Start and configure ISM */
  if(!this->ism.begin()) return false;
  this->ism.deviceReset();
  this->ism.setDeviceConfig();
  this->ism.setBlockDataUpdate();
  this->ism.setAccelDataRate(ISM_XL_ODR_104Hz);
	this->ism.setAccelFullScale(ISM_4g);
  this->ism.setGyroDataRate(ISM_GY_ODR_104Hz);
	this->ism.setGyroFullScale(ISM_500dps);
  this->ism.setAccelFilterLP2();
	this->ism.setAccelSlopeFilter(ISM_LP_ODR_DIV_100);
  this->ism.setGyroFilterLP1();
	this->ism.setGyroLP1Bandwidth(ISM_STRONG);


  /* Start and configure MMC */
  if(!this->mmc.begin()) return false;
  this->mmc.softReset();


  /* Start filter */
  this->filter.begin(CARDIMETRY_SENSOR_IMU_FREQ);


  /* Start ECG */
  this->ads.ads1293Begin3LeadECG();


  vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_SENSOR_INIT_DELAY_MS));
  return true;
}