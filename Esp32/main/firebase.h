#ifndef FIREBASE_H
#define FIREBASE_H

#define WIFI_SSID "Reseau Gloglo"
#define WIFI_PASSWORD "GloGlo317682"

#define API_KEY "AIzaSyBluSTtDBKsxPEBhWRs-42WyvwxFodY8RQ"
#define DATABASE_URL "https://bosco-nnect-default-rtdb.europe-west1.firebasedatabase.app/"

#define ADMIN_EMAIL "bosco.adresseprojet@gmail.com"
#define ADMIN_PASSWORD "GEII1234!"

#define IUD_USER "-O8HoWxypcSDv8EvYaTi"

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

extern volatile data_firebase firebase;


void init_firebase(void);
void write_recurrent_sensor_firebase(void);
void write_instant_sensor_firebase(void);
void write_tor_firebase(void);
void read_tor_firebase(void);
// void read_tor_ESP32(void);
// void write_tor_ESP32(void);


#endif
