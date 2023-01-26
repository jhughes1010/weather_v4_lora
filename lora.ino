//===========================================
// loraSend: Send environment sensor data
//===========================================
void loraSend(void * packetStart, int packetSize) {

  LoRa.enableCrc();
  LoRa.beginPacket();
  MonPrintf("Packet size: %i\n", packetSize);
  LoRa.write((byte *)packetStart, packetSize);
  //should be blocking mode
  LoRa.endPacket();
  MonPrintf("Done TX\n");
}
