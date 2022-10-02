void loraSystemHardwareSend(struct diagnostics hardware)
{
  //struct sensorData env = {};
  LoRa.beginPacket();

  /*
     LoRa.setTxPower(txPower,RFOUT_pin);
     txPower -- 0 ~ 20
     RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
       - RF_PACONFIG_PASELECT_PABOOST -- LoRa single output via PABOOST, maximum output 20dBm
       - RF_PACONFIG_PASELECT_RFO     -- LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
  */
  LoRa.setTxPower(14, RF_PACONFIG_PASELECT_PABOOST);
  //LoRa.print("weather v4 ");
  MonPrintf("Packet size: %i\n", sizeof(diagnostics));
  LoRa.write((byte *)&hardware, sizeof(diagnostics));
  //LoRa.print(bootCount);
  LoRa.endPacket();
}

void loraSend(struct sensorData environment)
{
  struct sensorData env = {};
  LoRa.beginPacket();

  /*
     LoRa.setTxPower(txPower,RFOUT_pin);
     txPower -- 0 ~ 20
     RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
       - RF_PACONFIG_PASELECT_PABOOST -- LoRa single output via PABOOST, maximum output 20dBm
       - RF_PACONFIG_PASELECT_RFO     -- LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
  */
  LoRa.setTxPower(14, RF_PACONFIG_PASELECT_PABOOST);
  //LoRa.print("weather v4 ");
  MonPrintf("Packet size: %i\n", sizeof(sensorData));
  LoRa.write((byte *)&environment, sizeof(sensorData));
  //LoRa.print(bootCount);
  LoRa.endPacket();
}
/*
void loraTestSend( void)
{
  Serial.println("Sending LoRa packet");
  //LoRa.print("hello there my little LoRa friend!");
  LoRa.print("hello there my little LoRa friend! I hope you are doing quite well.");
  LoRa.print(bootCount);
  LoRa.endPacket();
  delay(60000);
}*/
