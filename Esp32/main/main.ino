#include <Wire.h>
#include <WiFi.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>

#include "gpio.h"
#include "screen.h"
#include "firebase.h"
#include "sensor.h"
#include "wifi_esp.h"


void setup()
{
  Serial.begin(115200);
  init_wifi();
  init_firebase();
  init_displays_tft();
  init_sensor();
  Wire.begin(6, 7);
  
}

void loop()
{
  lv_timer_handler();
  static int last = 0;
  if (millis() - last > 20000)
  {
    read_tor_firebase();
    read_sensor_ESP32();
    write_tor_firebase();
    write_instant_sensor_firebase();
    write_recurrent_sensor_firebase();
    last = millis();
  }

}
