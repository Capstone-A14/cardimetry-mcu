# cardimetry-mcu

## 1. Prerequisites
Sebelum meng-*compile* dan mem-*flash* **cardimetry_mcu.ino**, pastikan *library* berikut telah terpasang pada Arduino IDE Anda.

1. [ArduinoJson](https://github.com/bblanchon/ArduinoJson) by bblanchon
2. [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) by Bodmer
3. [TJpg_Decoder](https://github.com/Bodmer/TJpg_Decoder) by Bodmer
4. [PubSubClient](https://github.com/knolleary/pubsubclient) by knolleary
5. [EspMQTTClient](https://github.com/plapointe6/EspMQTTClient) by plapointe6
6. [Adafruit_AHRS](https://github.com/adafruit/Adafruit_AHRS) by adafruit
7. [Adafruit_MPU6050](https://github.com/adafruit/Adafruit_MPU6050) by adafruit
8. [Adafruit_HMC5883_Unified](https://github.com/adafruit/Adafruit_HMC5883_Unified) by adafruit
9. [protocentral-ads1293-arduino](https://github.com/Protocentral/protocentral-ads1293-arduino) by Protocentral
10. [Adafruit_FT6206_Library](https://github.com/adafruit/Adafruit_FT6206_Library) by adafruit
11. [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library) by adafruit

Kemudian, *clone* repository [cardimetry-mcu-dlc](https://github.com/Capstone-A14/cardimetry-mcu-dlc) pada kartu SD yang akan dipasangkan pada perangkat Cardimetry. Pada kondisi ini, seharusnya alat sudah dapat digunakan.

## 2. Setup
Pada *file* **cardimetry-mcu/cardimetry_mcu/CardimetryConn.h** atur **CARDIMETRY_CONN_BASE_SERVICE_IP** sesuai dengan target IP atau IP *server* yang digunakan.