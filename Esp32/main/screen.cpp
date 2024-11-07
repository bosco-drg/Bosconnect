#include <lvgl.h>
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include <stdio.h>

#include "firebase.h"
#include "main.h"

const uint16_t screenWidth = 240;
const uint16_t screenHeight = 320;
lv_disp_draw_buf_t draw_buf;
lv_color_t buf[screenWidth * screenHeight / 10];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight);

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX = 0, touchY = 0;
  bool touched = tft.getTouch(&touchX, &touchY, 600);
  if (!touched)
    data->state = LV_INDEV_STATE_REL;
  else
  {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
  }
}

void init_displays_tft()
{
  uint16_t calData[5] = { 425, 3243, 320, 3579, 1 };
  tft.setTouch(calData);
  lv_init();
  tft.begin();
  tft.setRotation(0);

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  ui_init();
}

void set_device1_on(lv_event_t *e)
{
  bool lastfinder = firebase.finder1;
  firebase.finder1 = true;
  if (lastfinder != firebase.finder1)
    write_finder1_firebase();
}

void set_device1_off(lv_event_t *e)
{
  bool lastfinder = firebase.finder1;
  firebase.finder1 = false;
  if (lastfinder != firebase.finder1)
    write_finder1_firebase();
}

void set_device2_on(lv_event_t *e)
{
  bool lastfinder = firebase.finder2;
  firebase.finder2 = true;
  if (lastfinder != firebase.finder2)
    write_finder2_firebase();
}

void set_device2_off(lv_event_t *e)
{
  bool lastfinder = firebase.finder2;
  firebase.finder2 = false;
  if (lastfinder != firebase.finder2)
    write_finder2_firebase();
}

void sliderdimmer(lv_event_t *e)
{
  lv_obj_t *slider = lv_event_get_target(e);
  firebase.pwm = lv_slider_get_value(slider);
  write_slider_firebase();
}

void resetfirebase(lv_event_t * e)
{
  reset_firebase();
}

void rfidnewpass(lv_event_t *e)
{
  new_card = true;
}

void return_rfid_pass(lv_event_t * e)
{
  new_card = false;
}

void change_donnees_on(lv_event_t * e)
{
  screen_data_detect = true;
}

void change_donnees_off(lv_event_t * e)
{
  screen_data_detect = false;
}

void ssid_wifi(lv_event_t * e)
{
  wifi_connect = false;
  const char *text = lv_textarea_get_text(ui_TextArea1);
  firebase.ssid = String(text);  
}

void pass_wifi(lv_event_t * e)
{
  const char *text = lv_textarea_get_text(ui_TextArea2);
  firebase.pass = String(text); 
  init_wifi();
}
