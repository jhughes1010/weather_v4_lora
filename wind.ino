//=======================================================
// Variables used in calculating the windspeed (from ISR)
//=======================================================
volatile unsigned long timeSinceLastTick = 0;
volatile unsigned long lastTick = 0;
volatile unsigned long tickTime[20] = { 0 };
volatile int count = 0;

//========================================================================
//  readWindSpeed: Look at ISR data to see if we have wind data to average
//========================================================================
void readWindSpeed(struct sensorData *environment) {
  float windSpeed = 0;
  int position;
  long msTotal = 0;
  int samples = 0;

  //intentionally ignore the zeroth element
  //look at up to 5 ticks to get wind speed
  if (count) {
    MonPrintf("Count: %i\n", count);
    for (position = 1; position < 7; position++) {
      if (tickTime[position]) {
        msTotal += tickTime[position];
        samples++;
      }
    }
  } else {
    MonPrintf("No count values\n");
  }
  //Average samples
  if (msTotal > 0 && samples > 0) {
    windSpeed = 2.4 * 1000 / (msTotal / samples);
  } else {
    MonPrintf("No Wind data\n");
    windSpeed = 0;
  }
  //I see 2 ticks per revolution
  windSpeed = windSpeed / WIND_TICKS_PER_REVOLUTION;

  MonPrintf("WindSpeed: %f\n", windSpeed);
  environment->windSpeed = windSpeed;
  environment->windSpeedMax = maxWindSpeed;
  maxWindSpeed = 0;
}

//=======================================================
//  readWindDirection: Read ADC to find wind direction
//=======================================================
void readWindDirectionADC(struct sensorData *environment) {
  environment->windDirectionADC = analogRead(WIND_DIR_PIN);
  MonPrintf("WindDirADC: %i\n", environment->windDirectionADC);
}

//=======================================================
//  checkMaxWind: Update max windspeed, if needed
//=======================================================
void checkMaxWind(void) {
  //if current windspeed > MaxWindspeed
  //MaxWindspeed = current
}

//=======================================================
//  windTick: ISR to capture wind speed relay closure
//=======================================================
void IRAM_ATTR windTick(void) {
  timeSinceLastTick = millis() - lastTick;
  //software debounce attempt
  //record up to 10 ticks from anemometer
  if (timeSinceLastTick > 10 && count < 10) {
    lastTick = millis();
    tickTime[count] = timeSinceLastTick;
    count++;
    //digitalWrite(LED_BUILTIN, HIGH);
  }
}
