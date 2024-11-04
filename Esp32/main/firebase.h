#ifndef FIREBASE_H
#define FIREBASE_H

#include <Arduino.h>

#define WIFI_SSID "bosco"
#define WIFI_PASS "1234567890"

#define USER_MAIL "bosco.adresseprojet@gmail.com"
#define USER_PASS "GEII1234!"

#define API_KEY "AIzaSyBluSTtDBKsxPEBhWRs-42WyvwxFodY8RQ"
#define DATABASE_URL "https://bosco-nnect-default-rtdb.europe-west1.firebasedatabase.app/"

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

} data_firebase;

extern data_firebase firebase;


#endif
