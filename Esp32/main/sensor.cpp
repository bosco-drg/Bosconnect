#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <BH1750.h>

#include "gpio.h"
#include "sensor.h"
#include "firebase.h"

BH1750 lightMeter;
Adafruit_BMP280 bmp;

void read_sensor_ESP32()
{
    firebase.gas = analogRead(GAZ_SENSOR);
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
