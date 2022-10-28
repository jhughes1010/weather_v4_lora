//===========================================
// loraSystemHardwareSend: Send hardware system data
//===========================================
void loraSystemHardwareSend(struct diagnostics hardware) {

  LoRa.beginPacket();

  /*
     LoRa.setTxPower(txPower,RFOUT_pin);
     txPower -- 0 ~ 20
     RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
       - RF_PACONFIG_PASELECT_PABOOST -- LoRa single output via PABOOST, maximum output 20dBm
       - RF_PACONFIG_PASELECT_RFO     -- LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
  */
#ifdef heltec
  LoRa.setTxPower(14, RF_PACONFIG_PASELECT_RFO);
#endif
  MonPrintf("Packet size: %i\n", sizeof(diagnostics));
  LoRa.write((byte *)&hardware, sizeof(diagnostics));
  //should be blocking mode
  LoRa.endPacket();
}

//===========================================
// loraSend: Send environment sensor data
//===========================================
void loraSend(struct sensorData environment) {

  LoRa.beginPacket();

/*
     LoRa.setTxPower(txPower,RFOUT_pin);
     txPower -- 0 ~ 20
     RFOUT_pin could be RF_PACONFIG_PASELECT_PABOOST or RF_PACONFIG_PASELECT_RFO
       - RF_PACONFIG_PASELECT_PABOOST -- LoRa single output via PABOOST, maximum output 20dBm
       - RF_PACONFIG_PASELECT_RFO     -- LoRa single output via RFO_HF / RFO_LF, maximum output 14dBm
  */
#ifdef heltec
  LoRa.setTxPower(14, RF_PACONFIG_PASELECT_RFO);
#endif
  MonPrintf("Packet size: %i\n", sizeof(sensorData));
  LoRa.write((byte *)&environment, sizeof(sensorData));
  //should be blocking mode
  LoRa.endPacket();
}