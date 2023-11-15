#ifndef __TOUCH_TEST_H__
#define __TOUCH_TEST_H__

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_FT6206.h>

Adafruit_FT6206 ctp = Adafruit_FT6206();

void touch_begin() {
  ctp.begin(40, &Wire);
}

void touch_print() {
  if(!ctp.touched()) {
    return;
  }
  else {
    TS_Point p = ctp.getPoint();
    Serial.print("[TOUCH_TEST] (x,y) = ("); Serial.print(p.y); Serial.print(','); Serial.print(320 - p.x); Serial.println(')');
  }
}

#endif