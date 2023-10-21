#ifndef __DOWNLOAD_TEST_H__
#define __DOWNLOAD_TEST_H__

#include <SD.h>
#include "WiFi.h"
// #include "WiFiClientSecure.h"
#include "HTTPClient.h"



const char* test_root_ca = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIFajCCBPGgAwIBAgIQDNCovsYyz+ZF7KCpsIT7HDAKBggqhkjOPQQDAzBWMQsw\n" \
  "CQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMTAwLgYDVQQDEydEaWdp\n" \
  "Q2VydCBUTFMgSHlicmlkIEVDQyBTSEEzODQgMjAyMCBDQTEwHhcNMjMwMjE0MDAw\n" \
  "MDAwWhcNMjQwMzE0MjM1OTU5WjBmMQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2Fs\n" \
  "aWZvcm5pYTEWMBQGA1UEBxMNU2FuIEZyYW5jaXNjbzEVMBMGA1UEChMMR2l0SHVi\n" \
  "LCBJbmMuMRMwEQYDVQQDEwpnaXRodWIuY29tMFkwEwYHKoZIzj0CAQYIKoZIzj0D\n" \
  "AQcDQgAEo6QDRgPfRlFWy8k5qyLN52xZlnqToPu5QByQMog2xgl2nFD1Vfd2Xmgg\n" \
  "nO4i7YMMFTAQQUReMqyQodWq8uVDs6OCA48wggOLMB8GA1UdIwQYMBaAFAq8CCkX\n" \
  "jKU5bXoOzjPHLrPt+8N6MB0GA1UdDgQWBBTHByd4hfKdM8lMXlZ9XNaOcmfr3jAl\n" \
  "BgNVHREEHjAcggpnaXRodWIuY29tgg53d3cuZ2l0aHViLmNvbTAOBgNVHQ8BAf8E\n" \
  "BAMCB4AwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMIGbBgNVHR8EgZMw\n" \
  "gZAwRqBEoEKGQGh0dHA6Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydFRMU0h5\n" \
  "YnJpZEVDQ1NIQTM4NDIwMjBDQTEtMS5jcmwwRqBEoEKGQGh0dHA6Ly9jcmw0LmRp\n" \
  "Z2ljZXJ0LmNvbS9EaWdpQ2VydFRMU0h5YnJpZEVDQ1NIQTM4NDIwMjBDQTEtMS5j\n" \
  "cmwwPgYDVR0gBDcwNTAzBgZngQwBAgIwKTAnBggrBgEFBQcCARYbaHR0cDovL3d3\n" \
  "dy5kaWdpY2VydC5jb20vQ1BTMIGFBggrBgEFBQcBAQR5MHcwJAYIKwYBBQUHMAGG\n" \
  "GGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBPBggrBgEFBQcwAoZDaHR0cDovL2Nh\n" \
  "Y2VydHMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0VExTSHlicmlkRUNDU0hBMzg0MjAy\n" \
  "MENBMS0xLmNydDAJBgNVHRMEAjAAMIIBgAYKKwYBBAHWeQIEAgSCAXAEggFsAWoA\n" \
  "dwDuzdBk1dsazsVct520zROiModGfLzs3sNRSFlGcR+1mwAAAYZQ3Rv6AAAEAwBI\n" \
  "MEYCIQDkFq7T4iy6gp+pefJLxpRS7U3gh8xQymmxtI8FdzqU6wIhALWfw/nLD63Q\n" \
  "YPIwG3EFchINvWUfB6mcU0t2lRIEpr8uAHYASLDja9qmRzQP5WoC+p0w6xxSActW\n" \
  "3SyB2bu/qznYhHMAAAGGUN0cKwAABAMARzBFAiAePGAyfiBR9dbhr31N9ZfESC5G\n" \
  "V2uGBTcyTyUENrH3twIhAPwJfsB8A4MmNr2nW+sdE1n2YiCObW+3DTHr2/UR7lvU\n" \
  "AHcAO1N3dT4tuYBOizBbBv5AO2fYT8P0x70ADS1yb+H61BcAAAGGUN0cOgAABAMA\n" \
  "SDBGAiEAzOBr9OZ0+6OSZyFTiywN64PysN0FLeLRyL5jmEsYrDYCIQDu0jtgWiMI\n" \
  "KU6CM0dKcqUWLkaFE23c2iWAhYAHqrFRRzAKBggqhkjOPQQDAwNnADBkAjAE3A3U\n" \
  "3jSZCpwfqOHBdlxi9ASgKTU+wg0qw3FqtfQ31OwLYFdxh0MlNk/HwkjRSWgCMFbQ\n" \
  "vMkXEPvNvv4t30K6xtpG26qmZ+6OiISBIIXMljWnsiYR1gyZnTzIg3AQSw4Vmw==\n" \
  "-----END CERTIFICATE-----\n";



// WiFiClientSecure client;

void download_begin() {
  WiFi.begin("DhonanAP", "epiepiepi");
  while(WiFi.status() != WL_CONNECTED);
  Serial.println("Connected to DhonanAP");

  if(SD.begin(16)) {
    Serial.println("SD card ready");
  }
  
  // Download and save the file
  HTTPClient http;
  // if(http.begin("https://github.com/Capstone-A14/cardimetry-mcu-dlc/raw/master/fonts/PoppinsLight10.vlw", test_root_ca)) {
  if(http.begin("https://techtutorialsx.com/2017/11/18/esp32-arduino-https-get-request/")) {
    
    if(http.GET() == HTTP_CODE_OK) {
      File file = SD.open("/PoppinsLight10.vlw", FILE_WRITE);

      if(file) {
        WiFiClient* stream = http.getStreamPtr();

        while(stream->connected()) {
          String line = stream->readStringUntil('\n');
          file.println(line);
        }

        file.close();
        Serial.println("File downloaded and saved successfully.");
      }

      else {
        Serial.println("Error opening file for writing.");
      }
    }

    else {
      Serial.println("HTTPS request failed.");
    }
  }

  http.end();
}

void download_loop() {

}

#endif