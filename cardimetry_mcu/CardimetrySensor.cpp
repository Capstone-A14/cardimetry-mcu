#include "CardimetrySensor.h"




cardimetry::CardimetrySensor::CardimetrySensor() {}
cardimetry::CardimetrySensor::~CardimetrySensor() {}




bool cardimetry::CardimetrySensor::begin() {
  Wire.begin();


  /* Start and configure ISM */
  if(!this->mpu.begin()) return false;
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  // this->ism.deviceReset();
  // this->ism.setDeviceConfig();
  // this->ism.setBlockDataUpdate();
  // this->ism.setAccelDataRate(ISM_XL_ODR_104Hz);
	// this->ism.setAccelFullScale(ISM_4g);
  // this->ism.setGyroDataRate(ISM_GY_ODR_104Hz);
	// this->ism.setGyroFullScale(ISM_500dps);
  // this->ism.setAccelFilterLP2();
	// this->ism.setAccelSlopeFilter(ISM_LP_ODR_DIV_100);
  // this->ism.setGyroFilterLP1();
	// this->ism.setGyroLP1Bandwidth(ISM_STRONG);


  /* Start and configure MMC */
  if(!this->hmc.begin()) return false;
  // this->mmc.softReset();


  /* Start filter */
  this->filter.begin(CARDIMETRY_SENSOR_IMU_FREQ);
  

  vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_SENSOR_INIT_DELAY_MS));
  return true;
}




bool cardimetry::CardimetrySensor::ecgSync() {
  int32_t lead1[30],
          lead2[30],
          lead3[30];
  uint8_t zero_cnt = 0;

  /* Begin ADS1293 */
  // pinMode(CARDIMETRY_SENSOR_ECG_CS_PIN, OUTPUT);
  // this->ads.ads1293Begin3LeadECG();
  // vTaskDelay(pdMS_TO_TICKS(CARDIMETRY_SENSOR_ECG_SYNC_DELAY_MS));

  /* Run sync test */
  for(int i = 0; i < 30; ++i) {
    if(digitalRead(CARDIMETRY_SENSOR_ECG_DRDY_PIN) == false) {
      lead1[i] = this->ads.getECGdata(1);
      lead2[i] = this->ads.getECGdata(2);
      lead3[i] = lead1[i] - lead2[i];
    }
    else {
      --i;
      continue;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  for(uint8_t i = 0; i < 30; ++i) {
    if(lead1[i] == 0 || lead2[i] == 0 || lead3[i] == 0) ++zero_cnt;
  }

  if(zero_cnt > 5) {
    return false;
  }

  return true;
}