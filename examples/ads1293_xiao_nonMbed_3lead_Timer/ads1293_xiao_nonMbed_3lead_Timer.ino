// These define's must be placed at the beginning before #include "NRF52TimerInterrupt.h"
// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
// Don't define TIMER_INTERRUPT_DEBUG > 2. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     3

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
#include "NRF52TimerInterrupt.h"

#include "protocentral_ads1293.h"
#include <SPI.h>

#define DRDY_PIN                D2
#define CS_PIN                  D3

ads1293 ADS1293(DRDY_PIN, CS_PIN);

bool drdyIntFlag = false;

unsigned int SWPin = 11;

#define TIMER0_INTERVAL_MS        2
//#define TIMER0_DURATION_MS        5000

// Depending on the board, you can select NRF52 Hardware Timer from NRF_TIMER_1-NRF_TIMER_4 (1 to 4)
// If you select the already-used NRF_TIMER_0, it'll be auto modified to use NRF_TIMER_1

// Init NRF52 timer NRF_TIMER1
NRF52Timer ITimer0(NRF_TIMER_1);
volatile uint32_t preMillisTimer0 = 0;

void TimerHandler0()
{
    if (digitalRead(ADS1293.drdyPin) == false){

    //digitalWrite(4,HIGH);
    drdyIntFlag = false;
    int32_t ecg = ADS1293.getECGdata(1);
    int32_t ecg2 = ADS1293.getECGdata(2);
    int32_t ecg3 =ecg2-ecg;     
    Serial.print(ecg);Serial.print(',');Serial.print(ecg2);Serial.print(',');Serial.println(ecg3);
    //digitalWrite(4,LOW);
  }
  //preMillisTimer0 = millis();

  //timer interrupt toggles pin LED_BUILTIN
  //digitalWrite(LED_BUILTIN, toggle0);
  //toggle0 = !toggle0;
}



void setup() {

  Serial.begin(115200);
  // Interval in microsecs
  if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0))
  {
    preMillisTimer0 = millis();
    Serial.print(F("Starting  ITimer0 OK, millis() = "));
    Serial.println(preMillisTimer0);
  }
  else
    Serial.println(F("Can't set ITimer0. Select another freq. or timer"));
 // initalize the  data ready and chip select pins:
  //pinMode(DRDY_PIN, INPUT);
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

}
