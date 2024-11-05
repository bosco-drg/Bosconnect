#include <ui.h>
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <MFRC522.h>
#include <BH1750.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "main.h"
#include "screen.h"
#include "firebase.h"

MFRC522 mfrc522(CS_RFID, -1);
MFRC522::MIFARE_Key key;

data_firebase firebase;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
BH1750 lightMeter;
Adafruit_BMP280 bmp;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

volatile bool new_card = false;
volatile bool card_detect = false;
volatile bool screen_data_detect = false;

byte regVal = 0x7F;

void activateRec(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
}

void clearInt(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
}

void init_wifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

String getFormattedTime() {
  struct tm timeinfo;
  char timeStringBuff[50];
  if (getLocalTime(&timeinfo))
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

long getTimestamp() {
  struct tm timeinfo;
  time_t now;
  if (getLocalTime(&timeinfo))
    time(&now);
  return now;
}

void read_sensor_ESP32() {
  float ratio = (3.3 - analogRead(GAZ_SENSOR) * (3.3 / 4095.0)) / (analogRead(GAZ_SENSOR) * (3.3 / 4095.0));
  firebase.gas = pow(10, (log10(ratio * 10.0 / 10.0) - 2.7) / -0.77);
  firebase.brightness = lightMeter.readLightLevel();
  firebase.temperature = bmp.readTemperature();
  firebase.pressure = bmp.readPressure() / 100.0;
}

void init_sensor() {
  lightMeter.begin();
  bmp.begin(0x76);
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
}

void init_firebase() {
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

void reset_firebase() {
  String uid = auth.token.uid.c_str();
  String path = "/utilisateurs/" + uid + "/data";
  Firebase.RTDB.deleteNode(&fbdo, path);
}

void write_sensor_firebase(void) {
  static int last_temperature = 0;
  static int last_brightness = 0;
  static int last_pressure = 0;
  static int last_gas = 0;

  String uid = auth.token.uid.c_str();
  String base_path = "/utilisateurs/" + uid + "/instant_data/";

  int temperatureInt = int(firebase.temperature);
  if (abs(temperatureInt - last_temperature) > TEMPERATURE_THRESHOLD) {
    Firebase.RTDB.setInt(&fbdo, base_path + "temperature", temperatureInt);
    lv_label_set_text_fmt(ui_temperatureTFT, "%d", temperatureInt);
    last_temperature = temperatureInt;
  }

  int brightnessInt = int(firebase.brightness);
  if (abs(brightnessInt - last_brightness) > BRIGHTNESS_THRESHOLD) {
    Firebase.RTDB.setInt(&fbdo, base_path + "brightness", brightnessInt);
    lv_label_set_text_fmt(ui_brightnessTFT, "%d", brightnessInt);
    last_brightness = brightnessInt;
  }

  int pressureInt = int(firebase.pressure);
  if (abs(pressureInt - last_pressure) > PRESSURE_THRESHOLD) {
    Firebase.RTDB.setInt(&fbdo, base_path + "pressure", pressureInt);
    lv_label_set_text_fmt(ui_pressureTFT, "%d", pressureInt);
    last_pressure = pressureInt;
  }

  int gasInt = int(firebase.gas);
  if (abs(gasInt - last_gas) > GAS_THRESHOLD) {
    Firebase.RTDB.setInt(&fbdo, base_path + "gas", gasInt);
    lv_label_set_text_fmt(ui_gasTFT, "%d", gasInt);
    last_gas = gasInt;
  }
}

void write_sensor_tft(void) {
  static int last_temperature = 0;
  static int last_brightness = 0;
  static int last_pressure = 0;
  static int last_gas = 0;

  String uid = auth.token.uid.c_str();
  String base_path = "/utilisateurs/" + uid + "/instant_data/";

  int temperatureInt = int(firebase.temperature);
  if (abs(temperatureInt - last_temperature) > TEMPERATURE_THRESHOLD) {
    lv_label_set_text_fmt(ui_temperatureTFT, "%d", temperatureInt);
    last_temperature = temperatureInt;
  }

  int brightnessInt = int(firebase.brightness);
  if (abs(brightnessInt - last_brightness) > BRIGHTNESS_THRESHOLD) {
    lv_label_set_text_fmt(ui_brightnessTFT, "%d", brightnessInt);
    last_brightness = brightnessInt;
  }

  int pressureInt = int(firebase.pressure);
  if (abs(pressureInt - last_pressure) > PRESSURE_THRESHOLD) {
    lv_label_set_text_fmt(ui_pressureTFT, "%d", pressureInt);
    last_pressure = pressureInt;
  }

  int gasInt = int(firebase.gas);
  if (abs(gasInt - last_gas) > GAS_THRESHOLD) {
    lv_label_set_text_fmt(ui_gasTFT, "%d", gasInt);
    last_gas = gasInt;
  }
}

void write_chart_firebase(void) {
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

void write_finder1_firebase(void) {
  String uid = auth.token.uid.c_str();
  String path = "/utilisateurs/" + uid + "/instant_data/";
  Firebase.RTDB.setBool(&fbdo, path + "finder1", firebase.finder1);
}

void write_finder2_firebase(void) {
  String uid = auth.token.uid.c_str();
  String path = "/utilisateurs/" + uid + "/instant_data/";
  Firebase.RTDB.setBool(&fbdo, path + "finder2", firebase.finder2);
}

void write_slider_firebase(void) {
  String uid = auth.token.uid.c_str();
  String path = "/utilisateurs/" + uid + "/instant_data/";
  Firebase.RTDB.setInt(&fbdo, path + "pwm", firebase.pwm);
}

void read_tor_firebase(void) {
  String uid = auth.token.uid.c_str();
  String base_path = "/utilisateurs/" + uid + "/instant_data/";

  if (Firebase.RTDB.getBool(&fbdo, base_path + "finder1")) {
    if (fbdo.boolData() != firebase.finder1)
      firebase.finder1 = fbdo.boolData();
  }

  if (Firebase.RTDB.getBool(&fbdo, base_path + "finder2")) {
    if (fbdo.boolData() != firebase.finder2)
      firebase.finder2 = fbdo.boolData();
  }

  if (Firebase.RTDB.getInt(&fbdo, base_path + "pwm")) {
    if (fbdo.intData() != firebase.pwm)
      firebase.pwm = fbdo.intData();
  }
}

void readCard() {
  card_detect = true;
}

void write_tor_ESP32(void) {
  digitalWrite(FINDER1, firebase.finder1);
  digitalWrite(FINDER2, firebase.finder2);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(6, 7);
  SPI.begin(SCK, MISO, MOSI, CS_RFID);

  pinMode(CS_TFT, OUTPUT);
  pinMode(CS_RFID, OUTPUT);
  pinMode(CS_TOUCH, OUTPUT);
  pinMode(FINDER1, OUTPUT);
  pinMode(FINDER2, OUTPUT);

  digitalWrite(CS_RFID, LOW);
  digitalWrite(CS_TFT, HIGH);
  digitalWrite(CS_TOUCH, HIGH);

  mfrc522.PCD_Init();
  byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  pinMode(IRQ_RFID, INPUT_PULLUP);
  regVal = 0xA0;
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, regVal);
  card_detect = false;
  attachInterrupt(digitalPinToInterrupt(IRQ_RFID), readCard, FALLING);
  card_detect = true;

  init_wifi();
  init_displays_tft();
  init_firebase();
  init_sensor();
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void loop() {
  static long timer1 = 0;
  static long timer2 = 0;
  static long cardPresent = 0;
  const long ledDuration = 2000;

  lv_timer_handler();
  activateRec(mfrc522);

  if (!card_detect) {

    read_sensor_ESP32();
    write_tor_ESP32();

    if (screen_data_detect)
    {
      write_sensor_tft();
    }
    else
    {
      read_tor_firebase();
      
      if (millis() - timer2 > 500) {
        write_sensor_firebase();
        timer2 = millis();
      }
      if (millis() - timer1 > 20000) {
        write_chart_firebase();
        timer1 = millis();
      }
    }

    if (millis() - cardPresent > ledDuration) {
      digitalWrite(LED_BUILTIN, LOW);
    }

  } else {
    digitalWrite(CS_TFT, HIGH);
    digitalWrite(CS_TOUCH, HIGH);
    digitalWrite(CS_RFID, LOW);

    mfrc522.PICC_ReadCardSerial();
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    clearInt(mfrc522);
    mfrc522.PICC_HaltA();
    digitalWrite(LED_BUILTIN, HIGH);
    cardPresent = millis();
    card_detect = false;

    digitalWrite(CS_RFID, HIGH);
    digitalWrite(CS_TFT, LOW);
    digitalWrite(CS_TOUCH, LOW);
  }
}
