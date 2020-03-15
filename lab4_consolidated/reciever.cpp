// Group 1
// Solmaz Hashemzadeh
// Jonathan Combs

#include <SPI.h>
#include <LoRa.h>
#include "Muh_LoRa.h"

void setup() {
  SPI.begin(LORA_SCK,LORA_MISO,LORA_MOSI,LORA_CS);
  LoRa.setSPI(SPI);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
  Serial.begin(9600);

  //delay(10000);//so we can start the serial window for testing...
  Serial.println("LoRa Receiver");

  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
  }else{
    Serial.println("Starting LoRa succeeded!");
  }
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
