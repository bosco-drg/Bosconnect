#include <Firebase_ESP_Client.h>
#include <WiFi.h>

#include "gpio.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include "firebase.h"
#include "wifi_esp.h"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
volatile data_firebase firebase;

void init_firebase(void)
{
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = ADMIN_EMAIL;
  auth.user.password = ADMIN_PASSWORD;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;
  Firebase.begin(&config, &auth);
}


void write_recurrent_sensor_firebase(void) {
  String uid = IUD_USER;
  long timestamp = getTimestamp();
  String formattedTime = getFormattedTime();

  String path = "/utilisateurs/" + uid + "/data/temperature/" + String(timestamp);
  Firebase.RTDB.setString(&fbdo, path + "/time", formattedTime);
  Firebase.RTDB.setFloat(&fbdo, path + "/valeur", firebase.temperature);

  path = "/utilisateurs/" + uid + "/data/brightness/" + String(timestamp);
  Firebase.RTDB.setString(&fbdo, path + "/time", formattedTime);
  Firebase.RTDB.setFloat(&fbdo, path + "/valeur", firebase.brightness);

  path = "/utilisateurs/" + uid + "/data/pressure/" + String(timestamp);
  Firebase.RTDB.setString(&fbdo, path + "/time", formattedTime);
  Firebase.RTDB.setFloat(&fbdo, path + "/valeur", firebase.pressure);

  path = "/utilisateurs/" + uid + "/data/gas/" + String(timestamp);
  Firebase.RTDB.setString(&fbdo, path + "/time", formattedTime);
  Firebase.RTDB.setFloat(&fbdo, path + "/valeur", firebase.gas);
}

void write_instant_sensor_firebase(void)
{
  String uid = IUD_USER;
  Firebase.RTDB.setInt(&fbdo, "/utilisateurs/" + uid + "/instant_data/temperature", (int)firebase.temperature);
  Firebase.RTDB.setInt(&fbdo, "/utilisateurs/" + uid + "/instant_data/brightness", (int)firebase.brightness);
  Firebase.RTDB.setInt(&fbdo, "/utilisateurs/" + uid + "/instant_data/pressure", (int)firebase.pressure);
  Firebase.RTDB.setInt(&fbdo, "/utilisateurs/" + uid + "/instant_data/gas", (int)firebase.gas);
}

void write_tor_firebase(void)
{
  String uid = IUD_USER;
  String path = "/utilisateurs/" + uid + "/instant_data/" + "/finder1";
  Firebase.RTDB.setInt(&fbdo, path, firebase.finder1);
  path = "/utilisateurs/" + uid + "/instant_data/" + "/finder2";
  Firebase.RTDB.setInt(&fbdo, path, firebase.finder2);
  path = "/utilisateurs/" + uid + "/instant_data/" + "/finder3";
  Firebase.RTDB.setInt(&fbdo, path, firebase.finder3);
  path = "/utilisateurs/" + uid + "/instant_data/" + "/pwm";
  Firebase.RTDB.setInt(&fbdo, path, firebase.pwm);
}

void read_tor_firebase(void)
{
  String uid = IUD_USER;
  String path;
  path = "/utilisateurs/" + uid + "/instant_data/" + "/finder1";
  if (Firebase.RTDB.getInt(&fbdo, path))
    firebase.finder1 = fbdo.intData();
  path = "/utilisateurs/" + uid + "/instant_data/" + "/finder2";
  if (Firebase.RTDB.getInt(&fbdo, path))
    firebase.finder2 = fbdo.intData();
  path = "/utilisateurs/" + uid + "/instant_data/" + "/finder3";
  if (Firebase.RTDB.getInt(&fbdo, path))
    firebase.finder3 = fbdo.intData();
  path = "/utilisateurs/" + uid + "/instant_data/" + "/pwm";
  if (Firebase.RTDB.getInt(&fbdo, path))
    firebase.pwm = fbdo.intData();
}

/*
  void read_tor_ESP32(void)
  {
  }

  void write_tor_ESP32(void)
  {
  digitalWrite(FINDER1, firebase.finder1);
  digitalWrite(FINDER2, firebase.finder2);
  digitalWrite(FINDER3, firebase.finder3);
  //analogWrite(PWM,firebase.pwm);
  }
*/
