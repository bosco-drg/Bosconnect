//   _____  ______  _____  _____   _______   ____   _    _  _        ____   _   _
//  / ____||  ____||_   _||_   _| |__   __| / __ \ | |  | || |      / __ \ | \ | |
// | |  __ | |__     | |    | |      | |   | |  | || |  | || |     | |  | ||  \| |
// | | |_ ||  __|    | |    | |      | |   | |  | || |  | || |     | |  | ||   ` |
// | |__| || |____  _| |_  _| |_     | |   | |__| || |__| || |____ | |__| || |\  |
//  \_____||______||_____||_____|    |_|    \____/  \____/ |______| \____/ |_| \_|
//
//
//  ____    ____    _____   _____   ____   _   _  _   _  ______   _____  _______
// |  _ \  / __ \  / ____| / ____| / __ \ | \ | || \ | ||  ____| / ____||__   __|
// | |_) || |  | || (___  | |     | |  | ||  \| ||  \| || |__   | |        | |
// |  _ < | |  | | \___ \ | |     | |  | ||   ` ||   ` ||  __|  | |        | |
// | |_) || |__| | ____) || |____ | |__| || |\  || |\  || |____ | |____    | |
// |____/  \____/ |_____/  \_____| \____/ |_| \_||_| \_||______| \_____|   |_|



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

MFRC522 mfrc522(CS_RFID, RST_RFID);
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
volatile bool wifi_connect = false;
volatile bool touch_detect = false;
bool lastTouchDetect = false;

String uid = "";
String last_uid = "";
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
  WiFi.begin(firebase.ssid, firebase.pass);
  unsigned long startAttemptTime = millis();
  const unsigned long wifiTimeout = 10000;
  while (WiFi.status() != WL_CONNECTED && (millis() - startAttemptTime) < wifiTimeout);

  if (WiFi.status() == WL_CONNECTED) {
    init_firebase();
    wifi_connect = true;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  } else {
    wifi_connect = false;
  }
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
  firebase.brightness = 1;//lightMeter.readLightLevel();
  firebase.temperature = 1;//bmp.readTemperature();
  firebase.pressure = 1;//bmp.readPressure() / 100.0;
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
  String uid = UID_USER;
  String path = "/utilisateurs/" + uid + "/data";
  Firebase.RTDB.deleteNode(&fbdo, path);
  path = "/utilisateurs/" + uid + "/RFID";
  Firebase.RTDB.deleteNode(&fbdo, path);
}

void write_sensor_firebase(void) {
  static int last_temperature = 0;
  static int last_brightness = 0;
  static int last_pressure = 0;
  static int last_gas = 0;

  String uid = UID_USER;
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

  String uid = UID_USER;
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
  String uid = UID_USER;

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
  String uid = UID_USER;
  String path = "/utilisateurs/" + uid + "/instant_data/";
  Firebase.RTDB.setBool(&fbdo, path + "finder1", firebase.finder1);
}

void write_finder2_firebase(void) {
  String uid = UID_USER;
  String path = "/utilisateurs/" + uid + "/instant_data/";
  Firebase.RTDB.setBool(&fbdo, path + "finder2", firebase.finder2);
}

void write_slider_firebase(void) {
  String uid = UID_USER;
  String path = "/utilisateurs/" + uid + "/instant_data/";
  Firebase.RTDB.setInt(&fbdo, path + "pwm", firebase.pwm);
}

void read_setting_firebase(void) {
  String uid = UID_USER;
  String base_path = "/utilisateurs/" + uid + "/interval/";
  if (Firebase.RTDB.getInt(&fbdo, base_path + "interval")) {
    if (fbdo.intData() != firebase.interval)
      firebase.pwm = fbdo.intData();
  }

  base_path = "/utilisateurs/" + uid + "/auto/Device 1/";
  if (Firebase.RTDB.getBool(&fbdo, base_path + "autoMode")) {
    if (fbdo.intData() != firebase.autoFinder1)
      firebase.autoFinder1 = fbdo.intData();
  }

  if (firebase.autoFinder1)
  {
    if (Firebase.RTDB.getInt(&fbdo, base_path + "startDate")) {
      if (fbdo.intData() != firebase.startDate1)
        firebase.startDate1 = fbdo.intData();
    }

    if (Firebase.RTDB.getInt(&fbdo, base_path + "endDate")) {
      if (fbdo.intData() != firebase.endDate1)
        firebase.endDate1 = fbdo.intData();
    }
  }

  base_path = "/utilisateurs/" + uid + "/auto/Device 2/";
  if (Firebase.RTDB.getBool(&fbdo, base_path + "autoMode")) {
    if (fbdo.intData() != firebase.autoFinder2)
      firebase.autoFinder2 = fbdo.intData();
  }

  if (firebase.autoFinder2)
  {
    if (Firebase.RTDB.getInt(&fbdo, base_path + "startDate")) {
      if (fbdo.intData() != firebase.startDate2)
        firebase.startDate2 = fbdo.intData();
    }

    if (Firebase.RTDB.getInt(&fbdo, base_path + "endDate")) {
      if (fbdo.intData() != firebase.endDate2)
        firebase.endDate2 = fbdo.intData();
    }
  }
}

void read_tor_firebase(void) {
  String uid = UID_USER;
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
    lv_slider_set_value(ui_SliderPWM, firebase.pwm , LV_ANIM_OFF);
  }
}

bool isCardInFirebase(String cardID) {
  String uid = UID_USER;
  String path = "/utilisateurs/" + uid + "/RFID/" + cardID;
  if (Firebase.RTDB.getString(&fbdo, path)) {
    return fbdo.stringData() != "";
  } else {
    return false;
  }
}

void write_card_firebase(String cardID) {
  String uid = UID_USER;
  String path = "/utilisateurs/" + uid + "/RFID/" + cardID;
  Firebase.RTDB.setString(&fbdo, path, "valid");
  lv_obj_clear_flag(ui_Labelgoodmessage, LV_OBJ_FLAG_HIDDEN);
}

void readCard() {
  card_detect = true;
}

void write_tor_ESP32(void) {
  digitalWrite(FINDER1, firebase.finder1);
  digitalWrite(FINDER2, firebase.finder2);
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  uid = "";
  for (byte i = 0; i < bufferSize; i++) {
    uid += buffer[i];
  }
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
  pinMode(RFID_FINDER, OUTPUT);

  digitalWrite(CS_RFID, LOW);
  digitalWrite(CS_TFT, HIGH);
  digitalWrite(CS_TOUCH, HIGH);
  pinMode(IRQ_TFT, INPUT);

  mfrc522.PCD_Init();
  byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  pinMode(IRQ_RFID, INPUT_PULLUP);
  regVal = 0xA0;
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, regVal);
  card_detect = false;
  attachInterrupt(digitalPinToInterrupt(IRQ_RFID), readCard, FALLING);
  card_detect = true;

  init_displays_tft();
  //init_sensor();

  firebase.ssid = "Reseau Gloglo";
  firebase.pass = "GloGlo317682";
  init_wifi();

  firebase.interval = 60000;
}

void loop() {
  static long timer0 = 0;
  static long timer1 = 0;
  static long timer2 = 0;
  static long timer3 = 0;
  static long cardPresent = 0;
  static long readcard = 0;

  lv_timer_handler();

  if (WiFi.status() != WL_CONNECTED)
    wifi_connect = false;

  if (!card_detect) {
    read_sensor_ESP32();
    write_tor_ESP32();

    if (screen_data_detect) {
      write_sensor_tft();
    }
    else if (wifi_connect && touch_detect) {
      read_tor_firebase();

      if (millis() - timer2 > SENSOR_INTERVAL) {
        write_sensor_firebase();
        timer2 = millis();
      }

      if (millis() - timer1 > firebase.interval) {
        write_chart_firebase();
        timer1 = millis();
      }

      read_setting_firebase();
      long time_stamp = getTimestamp();

      if (firebase.autoFinder1) {
        if (time_stamp >= firebase.startDate1 && time_stamp <= firebase.endDate1)
          firebase.finder1 = true;
        else
          firebase.finder1 = false;
      }

      if (firebase.autoFinder2) {
        if (time_stamp >= firebase.startDate2 && time_stamp <= firebase.endDate2)
          firebase.finder2 = true;
        else
          firebase.finder2 = false;
      }
    }

    if (millis() - cardPresent > DOOR_OPEN_DURATION) {
      digitalWrite(RFID_FINDER, LOW);
    }

  } else {
    digitalWrite(CS_TFT, HIGH);
    digitalWrite(CS_TOUCH, HIGH);
    digitalWrite(CS_RFID, LOW);

    if (millis() - readcard >= TIME_READ_CARD) {

      mfrc522.PICC_ReadCardSerial();
      dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
      mfrc522.PICC_HaltA();

      if (isCardInFirebase(uid) && wifi_connect)
      {
        cardPresent = millis();
        digitalWrite(RFID_FINDER, HIGH);
        last_uid = uid;
      }

      readcard = millis();
    }

    if (new_card && wifi_connect)
    {
      write_card_firebase(uid);
    }

    clearInt(mfrc522);
    card_detect = false;

    digitalWrite(CS_RFID, HIGH);
    digitalWrite(CS_TFT, LOW);
    digitalWrite(CS_TOUCH, LOW);
  }
  activateRec(mfrc522);
}


//  ____                                        _                _____                        _                    _
// |  _ \                                      | |              |  __ \                      | |                  | |
// | |_) |  ___   ___   ___   ___            __| |  ___         | |__) |  __ _  _   _   __ _ | |  __ _  _   _   __| | _ __   ___
// |  _ <  / _ \ / __| / __| / _ \          / _` | / _ \        |  _  /  / _` || | | | / _` || | / _` || | | | / _` || '__| / _ \
// | |_) || (_) |\__ \| (__ | (_) |        | (_| ||  __/        | | \ \ | (_| || |_| || (_| || || (_| || |_| || (_| || |   |  __/
// |____/  \___/ |___/ \___| \___/          \__,_| \___|        |_|  \_\ \__,_| \__,_| \__, ||_| \__,_| \__,_| \__,_||_|    \___|
//                                  ______               ______                         __/ |
//                                 |______|             |______|                       |___/
