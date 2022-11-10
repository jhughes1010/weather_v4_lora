void LoRaPowerUp(void)
{
  //Turn on LoRa
  //let power stabilize before turning on LoRa
  delay(500);
  digitalWrite(LORA_PWR, HIGH);  //TODO: Need these as RTC_IO pins to stay enabled all the time
  delay(500);

#ifdef heltec
  Wire.begin(4, 15);
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
#else
  LoRa.setSPIFrequency(1000000);
  LoRa.setPins(15, 17, 13);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
#endif
  title("LoRa radio online");

  //End LoRa turn on
}


void powerDownAll(void)
{
  digitalWrite(SENSOR_PWR, LOW);
  digitalWrite(LORA_PWR, LOW);
}

void powerUpSensors(void)
{
  delay(500);
  digitalWrite(SENSOR_PWR, HIGH);
  delay(500);
}
