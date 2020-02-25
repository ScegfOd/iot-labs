//group 1
//Solmaz Hashemzadeh
//Jonathan Combs

#include <Arduino.h>
#include <Servo.h>
#include <Adafruit_BME680.h>
#include "Adafruit_APDS9960.h"
Adafruit_APDS9960 apds;

#define LED   25
#define PIR   5
#define RELAY 18
#define SERVO 19
#define SPEED 115200
#define SCL   22
#define SDA   21

#define BME_SCK 22
#define BME_MISO 21
#define BME_MOSI 21
#define BME_CS 21
#define BME_SDU 21

static const int servoPin = 19;

Adafruit_BME680 bme; //I2C

Servo servo1;

void setup() {   
  pinMode(LED,OUTPUT);
  pinMode(PIR,INPUT);
  pinMode(RELAY,OUTPUT);
  Serial.begin(SPEED);
  servo1.attach(servoPin);

  if(!bme.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  if(!apds.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  apds.enableColor(true);
}


void loop(){
  
  bool motion = digitalRead(PIR); //   Read the PIR
  if(motion){
    digitalWrite(LED,HIGH);    // Turns on the LED
    digitalWrite(RELAY,HIGH);
    delay(1000);
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees);
        delay(20);
    }
    for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        servo1.write(posDegrees);
        delay(20);
    }
  }else{
    digitalWrite(LED,LOW);
    digitalWrite(RELAY,LOW);
    delay(2000);
  }
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  
  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.println();


  //create some variables to store the color data in
  uint16_t r, g, b, c;

  //wait for color data to be ready
  while(!apds.colorDataReady()){
    delay(5);
  }

  //get the data and print the different channels
  apds.getColorData(&r, &g, &b, &c);
  Serial.print("red: ");
  Serial.print(r);
  
  Serial.print(" green: ");
  Serial.print(g);
  
  Serial.print(" blue: ");
  Serial.print(b);
  
  Serial.print(" clear: ");
  Serial.println(c);
  Serial.println();
  delay(1000);

} 
