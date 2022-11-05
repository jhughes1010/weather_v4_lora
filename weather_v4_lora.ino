//LoRa baseed weather station
//Hardware design by Debasish Dutta - opengreenenergy@gmail.com
//Software design by James Hughes - jhughes1010@gmail.com

/* History
   0.9.0 10-2-22 Initial development for Heltec ESP32 LoRa v2 devkit

   1.0.0 11-02-22 First release
                  Much fine tuning to do, but interested in getting feedback from users
*/

//Hardware build target: ESP32
#define VERSION "1.0.0 beta"

#ifdef heltec
#include "heltec.h"
#else
#include <LoRa.h>
#include <spi.h>
#endif

#include "defines.h"
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <driver/rtc_io.h>
#include <sys/time.h>

#include <time.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>
#include <BH1750.h>
#include <BME280I2C.h>
#include <Adafruit_SI1145.h>
//#include <stdarg.h>
#include <soc/soc.h>
#include <soc/rtc_cntl_reg.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <driver/rtc_io.h>
//OLED diagnostics board
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>

#define OLED_RESET 4


//===========================================
// Weather-environment structure
//===========================================
struct sensorData {
  int windDirectionADC;
  int rainTicks24h;
  int rainTicks60m;
  float temperatureC;
  float windSpeed;
  float barometricPressure;
  float humidity;
  float UVIndex;
  float lux;
};

//===========================================
// Station hardware structure
//===========================================
struct diagnostics {
  float BMEtemperature;
  float batteryVoltage;
  int batteryADC;
  int solarADC;
  int coreC;
  int bootCount;
};

//===========================================
// Sensor initilization structure
//===========================================
struct sensorStatus {
  int uv;
  int bme;
  int lightMeter;
  int temperature;
};

//===========================================
// rainfallData structure
//===========================================
struct rainfallData
{
  unsigned int intervalRainfall;
  unsigned int hourlyRainfall[24];
  unsigned int current60MinRainfall[5];
  unsigned int hourlyCarryover;
  unsigned int priorHour;
  unsigned int minuteCarryover;
  unsigned int priorMinute;
};

//===========================================
// RTC Memory storage
//===========================================
RTC_DATA_ATTR volatile int rainTicks = 0;
//RTC_DATA_ATTR int lastHour = 0;
//RTC_DATA_ATTR time_t nextUpdate;
RTC_DATA_ATTR struct rainfallData rainfall;
RTC_DATA_ATTR int bootCount = 0;
//RTC_DATA_ATTR unsigned int elapsedTime = 0;

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
time_t now;
time_t nextUpdate;
struct tm timeinfo;
//long rssi = 0;

//===========================================
// Setup
//===========================================
void setup() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  struct sensorData environment = {};
  struct diagnostics hardware = {};
  struct timeval tv;


  Serial.begin(115200);
  printTitle();

  //Enable WDT for any lock-up events
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);

  //time testing


  time(&now);
  localtime_r(&now, &timeinfo);
  updateWake();

  Serial.print("The current date/time is ");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  //-- end time testing

  //set hardware pins
  pinMode(WIND_SPD_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR_PWR, OUTPUT);
  pinMode(LORA_PWR, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);



  BlinkLED(1);





  //get wake up reason
  /*
    POR - First boot
    TIMER - Periodic send of sensor data on LoRa
    Interrupt - Count tick in rain gauge
  */
  wakeup_reason = esp_sleep_get_wakeup_cause();
  MonPrintf("\n\nWakeup reason: %d\n", wakeup_reason);
  //MonPrintf("rainTicks: %i\n", rainTicks);
  switch (wakeup_reason) {
    //Power on reset
    case 0:
      // set current day/time

      tv.tv_sec =   1667301066;  // enter UTC UNIX time (get it from https://www.unixtimestamp.com )
      settimeofday(&tv, NULL);
      //default to wake 5 sec after POR
      nextUpdate = 5;
      break;

    //Rain Tip Gauge
    case ESP_SLEEP_WAKEUP_EXT0:
      MonPrintf("Wakeup caused by external signal using RTC_IO\n");
      //updateWake();
      rainTicks++;
      break;

    //Timer
    case ESP_SLEEP_WAKEUP_TIMER:
      title("Wakeup caused by timer");
      //updateWake();
      bootCount++;

      //Turn on LoRa
      //let power stabilize before turning on LoRa
      delay(500);
      digitalWrite(SENSOR_PWR, HIGH);
      delay(500);
      digitalWrite(LORA_PWR, HIGH);  //TODO: Need these as RTC_IO pins to stay enabled all the time
      delay(500);

#ifdef heltec
      Wire.begin(4, 15);
      Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
#else
      Wire.begin();
      LoRa.setSPIFrequency(1000000);
      LoRa.setPins(15, 17, 13);
      if (!LoRa.begin(BAND)) {
        Serial.println("Starting LoRa failed!");
        while (1);
      }
#endif
      title("LoRa radio online");

      //End LoRa turn on


      //Rainfall interrupt pin set up
      delay(100);  //possible settling time on pin to charge
      //attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainTick, FALLING);
      attachInterrupt(digitalPinToInterrupt(WIND_SPD_PIN), windTick, RISING);


      //TODO: set TOD on interval

      //read sensors
      sensorEnable();
      sensorStatusToConsole();
      if (bootCount % 2 == 1) {
        title("Sending sensor data");
        //give 5 seconds to aquire wind speed data
        delay(5000);

        //update rainfall
        addTipsToMinute(rainTicks);
        printMinuteArray();
        clearRainfallMinute(timeinfo.tm_min + 5);
        printMinuteArray();

        addTipsToHour(rainTicks);
        clearRainfallHour(timeinfo.tm_hour + 1);
        //printHourlyArray();
        rainTicks = 0;

        //environmental sensor data send
        readSensors(&environment);

        //send LoRa data structure
        loraSend(environment);
        PrintEnvironment(environment);
      } else {
        title("Sending hardware data");
        //system (battery levels, ESP32 core temp, case temp, etc) send
        readSystemSensors(&hardware);
        hardware.bootCount = bootCount;
        loraSystemHardwareSend(hardware);
      }


      //Power down peripherals
      LoRa.end();
      digitalWrite(SENSOR_PWR, LOW);
      digitalWrite(LORA_PWR, LOW);
      break;
  }

  //preparing for sleep
  BlinkLED(1);
  sleepyTime(nextUpdate);
}

//===================================================
// loop: these are not the droids you are looking for
//===================================================
void loop() {
  //no loop code
}

//===================================================
// printTitle
//===================================================
void printTitle(void) {
  char buffer[32];
  Serial.printf("\n\nWeather station v4\n");
  Serial.printf("Version %s\n\n", VERSION);

#ifdef heltec
  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->clear();
  sprintf(buffer, "Weather v4 LoRa: %s", VERSION);
  Heltec.display->drawString(0, 0, buffer);
  sprintf(buffer, "Boot: %i", bootCount);
  Heltec.display->drawString(0, 16, buffer);
  Heltec.display->display();
  delay(3000);
#endif
}

//===========================================
// sleepyTime: prepare for sleep and set
// timer and EXT0 WAKE events
//===========================================
void sleepyTime(long nextUpdate) {
  int elapsedTime;
  Serial.println("Going to sleep now...");


  //rtc_gpio_set_level(GPIO_NUM_16, HIGH);
  //rtc_gpio_set_level(GPIO_NUM_26, HIGH);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 0);
  elapsedTime = (int)millis() / 1000;

  //subtract elapsed time to try to maintain interval
  nextUpdate -= elapsedTime;
  if (nextUpdate < 3)
  {
    nextUpdate = 3;
  }
  Serial.printf("Elapsed time: %i seconds\n", elapsedTime);
  Serial.printf("Waking in %i seconds\n", nextUpdate);
  Serial.flush();
  esp_sleep_enable_timer_wakeup(nextUpdate * SEC);
  esp_deep_sleep_start();
}

//===========================================
// MonPrintf: diagnostic printf to terminal
//===========================================
void MonPrintf(const char *format, ...) {
  char buffer[200];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);
#ifdef SerialMonitor
  Serial.printf("%s", buffer);
#endif
}

//===========================================
// BlinkLED: Blink BUILTIN x times
//===========================================
void BlinkLED(int count) {
  int x;
  //if reason code =0, then set count =1 (just so I can see something)
  if (!count) {
    count = 1;
  }
  for (x = 0; x < count; x++) {
    //LED ON
    digitalWrite(LED_BUILTIN, HIGH);
    delay(150);
    //LED OFF
    digitalWrite(LED_BUILTIN, LOW);
    delay(350);
  }
}

//===========================================
// HexDump: display environment structure bytes
//===========================================
void HexDump(struct sensorData environment) {
  int size = 28;
  int x;
  char ch;
  char *p = (char *)&environment;
  //memset(&environment,0,28);

  for (x = 0; x < size; x++) {
    MonPrintf("%02X ", p[x]);
    if (x % 8 == 7) {
      MonPrintf("\n");
    }
  }
  MonPrintf("\n");
}

//===========================================
// FillEnvironment: Fill environment struct with test data (no sensors)
//===========================================
void FillEnvironment(struct sensorData *environment) {
  environment->temperatureC = 20;
  environment->windSpeed = 05;
  //TODO environment->windDirection = 90;
  environment->barometricPressure = 30;
  environment->humidity = 15.0;
  environment->UVIndex = 3.0;
  environment->lux = 58;
}

//===========================================
// PrintEnvironment:
//===========================================
void PrintEnvironment(struct sensorData environment) {
  Serial.printf("Temperature: %f\n", environment.temperatureC);
  Serial.printf("Wind speed: %f\n", environment.windSpeed);
  //TODO:  Serial.printf("Wind direction: %f\n", environment->windDirection);
  Serial.printf("barometer: %f\n", environment.barometricPressure);
  Serial.printf("Humidity: %f\n", environment.humidity);
  Serial.printf("UV Index: %f\n", environment.UVIndex);
  Serial.printf("Lux: %f\n", environment.lux);
}

//===========================================
// Title: banner to terminal
//===========================================
void title(const char* format, ... )
{ char buffer[200];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end( args );
#ifdef SerialMonitor
  Serial.printf("==============================================\n");
  Serial.printf("%s\n", buffer);
  Serial.printf("==============================================\n");
#endif
}
