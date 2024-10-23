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

float last_temperature = 0;
float last_brightness = 0;
float last_pressure = 0;
float last_gas = 0;

const float TEMPERATURE_THRESHOLD = 0.5;
const float BRIGHTNESS_THRESHOLD = 5.0;
const float PRESSURE_THRESHOLD = 1.0;
const float GAS_THRESHOLD = 0.1;

void init_wifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED);
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

void write_sensor_firebase(void) {
  long timestamp = getTimestamp();
  String formattedTime = getFormattedTime();
  String uid = auth.token.uid.c_str();
  String base_path = "/utilisateurs/" + uid + "/instant_data/";

  if (abs(firebase.temperature - last_temperature) > TEMPERATURE_THRESHOLD) {
    Firebase.RTDB.setFloat(&fbdo, base_path + "temperature", String(firebase.temperature, 1).toFloat());
    lv_label_set_text_fmt(ui_temperatureTFT, "%.1f", firebase.temperature);
    String path = "/utilisateurs/" + uid + "/data/temperature/" + String(timestamp);
    Firebase.RTDB.setString(&fbdo, path + "/time", formattedTime);
    Firebase.RTDB.setFloat(&fbdo, path + "/valeur", firebase.temperature);
    last_temperature = firebase.temperature;
  }

  if (abs(firebase.brightness - last_brightness) > BRIGHTNESS_THRESHOLD) {
    Firebase.RTDB.setFloat(&fbdo, base_path + "brightness", String(firebase.brightness, 1).toFloat());
    lv_label_set_text_fmt(ui_brightnessTFT, "%.1f", firebase.brightness);
    String path = "/utilisateurs/" + uid + "/data/brightness/" + String(timestamp);
    Firebase.RTDB.setString(&fbdo, path + "/time", formattedTime);
    Firebase.RTDB.setFloat(&fbdo, path + "/valeur", firebase.brightness);
    last_brightness = firebase.brightness;
  }

  if (abs(firebase.pressure - last_pressure) > PRESSURE_THRESHOLD) {
    Firebase.RTDB.setFloat(&fbdo, base_path + "pressure", String(firebase.pressure, 1).toFloat());
    lv_label_set_text_fmt(ui_pressureTFT, "%.1f", firebase.pressure);
    String path = "/utilisateurs/" + uid + "/data/pressure/" + String(timestamp);
    Firebase.RTDB.setString(&fbdo, path + "/time", formattedTime);
    Firebase.RTDB.setFloat(&fbdo, path + "/valeur", firebase.pressure);
    last_pressure = firebase.pressure;
  }

  if (abs(firebase.gas - last_gas) > GAS_THRESHOLD) {
    Firebase.RTDB.setFloat(&fbdo, base_path + "gas", String(firebase.gas, 1).toFloat());
    lv_label_set_text_fmt(ui_gasTFT, "%.1f", firebase.gas);
    String path = "/utilisateurs/" + uid + "/data/gas/" + String(timestamp);
    Firebase.RTDB.setString(&fbdo, path + "/time", formattedTime);
    Firebase.RTDB.setFloat(&fbdo, path + "/valeur", firebase.gas);
    last_gas = firebase.gas;
  }
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

void loop()
{
  lv_timer_handler();
  read_sensor_ESP32();
  write_tor_ESP32();
  write_sensor_firebase();
}
