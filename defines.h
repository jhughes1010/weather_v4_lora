//===========================================
// Target build defines
//===========================================
//#define heltec


//===========================================
// Pin Defines
//===========================================
#define WIND_SPD_PIN 14  //reed switch based anemometer count
#define RAIN_PIN     25  //reed switch based tick counter on tip bucket
#define WIND_DIR_PIN 35  //variable voltage divider output based on varying R network with reed switches
#define PR_PIN       15  //photoresistor pin 
#define VBAT_PIN     39  //voltage divider for battery monitor
#define VSOLAR_PIN   36  //voltage divider for solar voltage monitor
#define TEMP_PIN      4  // DS18B20 hooked up to GPIO pin 4
#define LED_BUILTIN   2  //Diagnostics using built-in LED, may be set to 12 for newer boards that do not use devkit sockets
#define LORA_PWR     16
#define SENSOR_PWR   26


#define SEC 1E6          //Multiplier for uS based math
#define WDT_TIMEOUT 120   //watchdog timer

#define BAND 915E6

#define SerialMonitor

//===========================================
//Set how often to wake and read sensors
//===========================================
//const int UpdateIntervalSeconds = 3 * 60;  //Sleep timer (900s) for my normal operation
const int UpdateIntervalSeconds = .25 * 60; //Sleep timer (300s) testing


//===========================================
//Metric or Imperial measurements
//===========================================
//#define METRIC

//===========================================
//Use optional NVM for backup
//This is a failsafe for RESET events out of
//system control
//===========================================
//#define USE_EEPROM

//===========================================
//BME280 altitude offsets (set by user)
//===========================================
//#define ALTITUDE_OFFSET_IMPERIAL 5.58
//#define ALTITUDE_OFFSET_METRIC 142.6
//Paul J
#define ALTITUDE_OFFSET_IMPERIAL 6.66
#define ALTITUDE_OFFSET_METRIC 170.2

//===========================================
//BH1750 Enable
//===========================================
#define BH1750Enable

//===========================================
//Anemometer Calibration
//===========================================
//I see 2 switch pulls to GND per revolation. Not sure what others see
#define WIND_TICKS_PER_REVOLUTION 2

//===========================================
//Battery calibration
//===========================================
//batteryCalFactor = measured battery voltage/ADC reading
#define batteryCalFactor .00270
#define batteryLowVoltage 3.3
