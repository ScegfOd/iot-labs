// 
// Lab5: BLE
// Group 1:
// Apoorva Sharma
// Jonathan Combs
// Solmaz Hashemzadeh
// 
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <stdio.h> 
#include <string>  
 
using namespace std;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
 
//#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
//#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define BMETempService BLEUUID((uint16_t)0x181A)
#define BME_SCK 22
float tempReading;
char tempVal[5];

BLECharacteristic BMETempCharacteristic(BLEUUID((uint16_t)0x2A1F), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor BMETempDescriptor(BLEUUID((uint16_t)0x2901));
Adafruit_BME680 bme; // I2C

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  while (!Serial);
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
 
  BLEDevice::init("Group 1 BLE Server");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pBMETempService = pServer->createService(BMETempService);
  pBMETempService->addCharacteristic(&BMETempCharacteristic);
  BMETempCharacteristic.setValue(tempReading);
  BMETempDescriptor.setValue("in degree Celsius");
  BMETempCharacteristic.addDescriptor(&BMETempDescriptor);
  BMETempCharacteristic.addDescriptor(new BLEDescriptor(BLEUUID((uint16_t)0x2902)));
 
  //pCharacteristic->setValue("Hello World says Neil");
  pBMETempService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BMETempService);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  tempReading = bme.temperature;

  sprintf(tempVal,"%.2f",tempReading);

  BMETempCharacteristic.setValue(tempVal);
  BMETempCharacteristic.notify();

  Serial.print("Temperature = ");
  Serial.print(tempReading);
  Serial.println(" *C");

  delay(3000);
}