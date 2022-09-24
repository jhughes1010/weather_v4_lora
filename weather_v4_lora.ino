//LoRa baseed weather station
//Hardware design by Debasish Dutta - opengreenenergy@gmail.com
//Software design by James Hughes - jhughes1010@gmail.com

//Hardware build target: ESP32
#define VERSION "1.0.0"

#include "defines.h"
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <driver/rtc_io.h>

#include <time.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>
#include <BH1750.h>
#include <BME280I2C.h>
#include <Adafruit_SI1145.h>
#include <stdarg.h>
#include <PubSubClient.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <driver/rtc_io.h>
//OLED diagnostics board
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4

//===========================================
// RTC Memory storage
//===========================================
RTC_DATA_ATTR volatile int rainTicks = 0;
//RTC_DATA_ATTR int lastHour = 0;
//RTC_DATA_ATTR time_t nextUpdate;
//RTC_DATA_ATTR struct rainfallData rainfall;
RTC_DATA_ATTR int bootCount = 0;
//RTC_DATA_ATTR unsigned int elapsedTime = 0;

//===========================================
// Weather-environment structure
//===========================================
struct sensorData
{
  float temperatureC;
  float temperatureF;
  float windSpeed;
  float windDirection;
  char windCardinalDirection[5];
  float barometricPressure;
  float BMEtemperature;
  float humidity;
  float UVIndex;
  float lux;
  int photoresistor;
  float batteryVoltage;
  int batteryADC;
  unsigned int coreF;
  unsigned int coreC;
};

struct sensorStatus
{
  int uv;
  int bme;
  int lightMeter;
  int temperature;
};

//===========================================
// ISR Prototypes
//===========================================
void IRAM_ATTR rainTick(void);
void IRAM_ATTR windTick(void);

//===========================================
// Global instantiation
//===========================================
BH1750 lightMeter(0x23);
BME280I2C bme;
Adafruit_SI1145 uv = Adafruit_SI1145();
bool lowBattery = false;
struct sensorStatus status;
//long rssi = 0;

void setup()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  struct sensorData environment;

  Serial.begin(115200);
  printTitle();

  //get wake up reason
  wakeup_reason = esp_sleep_get_wakeup_cause();
  MonPrintf("Wakeup reason: %d\n", wakeup_reason);
  switch (wakeup_reason)
  {
    //Rain Tip Gauge
    case ESP_SLEEP_WAKEUP_EXT0 :
      MonPrintf("Wakeup caused by external signal using RTC_IO\n");
      //WiFiEnable = false;
      rainTicks++;
      break;


    //Timer
    case ESP_SLEEP_WAKEUP_TIMER :
      MonPrintf("Wakeup caused by timer\n");
      //WiFiEnable = true;
      //Rainfall interrupt pin set up
      delay(100); //possible settling time on pin to charge
      attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainTick, FALLING);
      attachInterrupt(digitalPinToInterrupt(WIND_SPD_PIN), windTick, RISING);

      //initialize GPIOs
      //power up peripherals
      //set TOD on interval
      //read sensors
      sensorEnable();
      sensorStatusToConsole();
      readSensors(&environment);
      //send LoRa data structure
      //power off peripherals
      break;
  }

  //sleep
  sleepyTime(UpdateIntervalSeconds);
}

//===================================================
// loop: these are not the droids you are looking for
//===================================================
void loop()
{
  //no loop code
}

//===================================================
// printTitle
//===================================================
void printTitle(void)
{
  Serial.printf("Weather station v4\n");
  Serial.printf("Version %s\n\n", VERSION);
}

//===========================================
// sleepyTime: prepare for sleep and set
// timer and EXT0 WAKE events
//===========================================
void sleepyTime(long UpdateInterval)
{
  Serial.println("\n\n\nGoing to sleep now...");
  Serial.printf("Waking in %i seconds\n\n\n\n\n\n\n\n\n\n", UpdateInterval);
  Serial.flush();

  rtc_gpio_set_level(GPIO_NUM_12, 0);
  esp_sleep_enable_timer_wakeup(UpdateInterval * SEC);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 0);
  //elapsedTime = (int)millis() / 1000;
  esp_deep_sleep_start();
}

//===========================================
// MonPrintf: diagnostic printf to terminal
//===========================================
void MonPrintf( const char* format, ... ) {
  char buffer[200];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end( args );
#ifdef SerialMonitor
  Serial.printf("%s", buffer);
#endif
}

//===========================================
// BlinkLED: Blink BUILTIN x times
//===========================================
void BlinkLED(int count)
{
  int x;
  //if reason code =0, then set count =1 (just so I can see something)
  if (!count)
  {
    count = 1;
  }
  for (x = 0; x < count; x++)
  {
    //LED ON
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
    //LED OFF
    digitalWrite(LED_BUILTIN, LOW);
    delay(350);
  }
}
