#ifndef FIREBASE_H
#define FIREBASE_H

#include <Arduino.h>

typedef struct data_firebase
{
  float brightness;
  float temperature;
  float pressure;
  float gas;
  bool finder1;
  bool finder2;
  bool finder3;
  int pwm;
  int interval;

  String ssid;
  String pass;
  
  bool autoFinder1;
  bool autoFinder2;

  int startDate1;
  int endDate1;
  int startDate2;
  int endDate2;

} data_firebase;

extern data_firebase firebase;


#endif
