#include <ui.h>
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "gpio.h"
#include "screen.h"
#include "firebase.h"

data_firebase firebase;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
BH1750 lightMeter;
Adafruit_BMP280 bmp;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

void init_wifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
    ;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

String getFormattedTime()
{
  struct tm timeinfo;
  char timeStringBuff[50];
  if (getLocalTime(&timeinfo))
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

long getTimestamp()
{
  struct tm timeinfo;
  time_t now;
  if (getLocalTime(&timeinfo))
    time(&now);
  return now;
}

void read_sensor_ESP32()
{
  float ratio = (3.3 - analogRead(GAZ_SENSOR) * (3.3 / 4095.0)) / (analogRead(GAZ_SENSOR) * (3.3 / 4095.0));
  firebase.gas = pow(10, (log10(ratio * 10.0 / 10.0) - 2.7) / -0.77);
  firebase.brightness = lightMeter.readLightLevel();
  firebase.temperature = bmp.readTemperature();
  firebase.pressure = bmp.readPressure() / 100.0;
}

void init_sensor()
{
  lightMeter.begin();
  bmp.begin(0x76);
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void init_firebase()
{
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_MAIL;
  auth.user.password = USER_PASS;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;
  Firebase.begin(&config, &auth);
}

void write_recurrent_sensor_firebase(void)
{
  long timestamp = getTimestamp();
  String formattedTime = getFormattedTime();
  String uid = auth.token.uid.c_str();

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
  String uid = auth.token.uid.c_str();
  String base_path = "/utilisateurs/" + uid + "/instant_data/";
  Firebase.RTDB.setFloat(&fbdo, base_path + "temperature", firebase.temperature);
  Firebase.RTDB.setFloat(&fbdo, base_path + "brightness", firebase.brightness);
  Firebase.RTDB.setFloat(&fbdo, base_path + "pressure", firebase.pressure);
  Firebase.RTDB.setFloat(&fbdo, base_path + "gas", firebase.gas);
}

void write_tor_firebase(void)
{
  String uid = auth.token.uid.c_str();
  String path = "/utilisateurs/" + uid + "/instant_data/";
  Firebase.RTDB.setBool(&fbdo, path + "finder1", firebase.finder1);
  Firebase.RTDB.setBool(&fbdo, path + "finder2", firebase.finder2);
  Firebase.RTDB.setInt(&fbdo, path + "pwm", firebase.pwm);
}

void read_tor_firebase(void)
{
  String uid = auth.token.uid.c_str();
  String base_path = "/utilisateurs/" + uid + "/instant_data/";
  if (Firebase.RTDB.getBool(&fbdo, base_path + "finder1"))
    firebase.finder1 = fbdo.boolData();
  if (Firebase.RTDB.getBool(&fbdo, base_path + "finder2"))
    firebase.finder2 = fbdo.boolData();
  if (Firebase.RTDB.getInt(&fbdo, base_path + "pwm"))
    firebase.pwm = fbdo.intData();
}

void write_tor_ESP32(void)
{
  digitalWrite(FINDER1, firebase.finder1);
  digitalWrite(FINDER2, firebase.finder2);
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(6, 7);
  init_displays_tft();
  init_wifi();
  init_firebase();
  init_sensor();


  pinMode(FINDER1, OUTPUT);
  pinMode(FINDER2, OUTPUT);
}

void manageLED() {
  static long last = 0;
  static bool ledState;
  int delayTime = map(firebase.pwm, 0, 100, 1000, 1);

  if (last - millis() >= delayTime) {
    last = millis();
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(LED_BUILTIN, ledState);
  }
}

void loop()
{
  lv_timer_handler();
  static unsigned long last = 0;
  read_sensor_ESP32();
  write_instant_sensor_firebase();
  read_tor_firebase();
  write_tor_ESP32();

  if (millis() - last > 20000)
  {
    write_tor_firebase();
    write_recurrent_sensor_firebase();
    last = millis();
  }
  //manageLED();
}
