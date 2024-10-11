#include <WiFi.h>

#include "gpio.h"
#include "wifi.h"
#include "firebase.h"
#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

void init_wifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
