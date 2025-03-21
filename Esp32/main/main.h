#ifndef MAIN_H
#define MAIN_H

#define GAZ_SENSOR 5
#define FINDER1 15
#define FINDER2 23
#define RFID_FINDER 22

#define CS_TFT 18
#define CS_TOUCH 2
#define CS_RFID 4

#define IRQ_RFID 12
#define IRQ_TFT 13 
#define RST_RFID -1

#define SCK 21
#define MISO 20
#define MOSI 19

#define TEMPERATURE_THRESHOLD  1
#define BRIGHTNESS_THRESHOLD 10
#define PRESSURE_THRESHOLD 50
#define GAS_THRESHOLD 50

#define SENSOR_INTERVAL 1000
#define DOOR_OPEN_DURATION 2500
#define TIME_READ_CARD 5000
#define SETTING_INTERVAL 10000

extern volatile bool new_card;
extern volatile bool wifi_connect;
extern volatile bool touch_detect;
extern volatile bool screen_data_detect;

void init_wifi(void);
void init_firebase(void);
void write_finder1_firebase(void);
void write_finder2_firebase(void);
void write_slider_firebase(void);
void reset_firebase(void);

#endif
