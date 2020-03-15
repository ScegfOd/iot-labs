// Group 1
// Solmaz Hashemzadeh
// Jonathan Combs

#include <Arduino.h>
#include <Adafruit_GPS.h>
#include <SPI.h>
#include <LoRa.h>
#include "Muh_LoRa.h"

#define CSMS 36
#define DRY 3700

// the name of the hardware serial port?
#define GPSSerial Serial2

// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false

//uint32_t timer = millis();

/////////////////// Timer stuff
volatile int interruptCounter;
int totalInterruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


int counter = 0;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
 
}

void sayHello(){
  Serial.print("Sending hello #");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();
}

void useGPS(){
  static char c;
  if (GPSSerial.available()) {
    Serial.println("GPS serial available :D");
    while(c=GPS.read()){};
    String raw = String(GPS.lastNMEA());
    String fixed = "UTC Time: ";
    fixed += raw.substring(7,9);
    fixed += ':';
    fixed += raw.substring(9,11);
    fixed += ':';
    fixed += raw.substring(11,17);
    Serial.println(fixed);
    
    
    LoRa.beginPacket();
    LoRa.println(fixed);
    LoRa.endPacket();
  /*
    if (GPS.newNMEAreceived()) {
      GPS.parse(GPS.lastNMEA());
      LoRa.beginPacket();
      
      LoRa.println(GPS.lastNMEA());
      LoRa.print("\nTime: ");
      LoRa.print(GPS.hour, DEC); LoRa.print(':');
      LoRa.print(GPS.minute, DEC); LoRa.print(':');
      LoRa.print(GPS.seconds, DEC); LoRa.print('.');
      LoRa.println(GPS.milliseconds);

      LoRa.print("Date: ");
      LoRa.print(GPS.day, DEC); LoRa.print('/');
      LoRa.print(GPS.month, DEC); LoRa.print("/20");
      LoRa.println(GPS.year, DEC);
      LoRa.print("Fix: "); LoRa.print((int)GPS.fix);
      LoRa.print(" quality: "); LoRa.println((int)GPS.fixquality);
      if (GPS.fix) {
        LoRa.print("Location: ");
        LoRa.print(GPS.latitude, 4); LoRa.print(GPS.lat);
        LoRa.print(", ");
        LoRa.print(GPS.longitude, 4); LoRa.println(GPS.lon);
        LoRa.print("Speed (knots): "); LoRa.println(GPS.speed);
        LoRa.print("Angle: "); LoRa.println(GPS.angle);
        LoRa.print("Altitude: "); LoRa.println(GPS.altitude);
        LoRa.print("Satellites: "); LoRa.println((int)GPS.satellites);
      }
      LoRa.endPacket();
      
    }*/
  }else{
    Serial.println("GPS serial not available D:");
  }
}

void checkSensor(){
  static String message;
  static unsigned int x;
  x = analogRead(CSMS);
  message = "Dryness: ";
  message += x;
  message += "; sensor is ";
  if (x > DRY)
    message += "dry!";
  else // eventually maybe test for wet soil?
    message += "wet!";
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  Serial.println(message);
}

void setup() {

  SPI.begin(LORA_SCK,LORA_MISO,LORA_MOSI,LORA_CS);
  LoRa.setSPI(SPI);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_IRQ);
  
  Serial.begin(9600);
  
  Serial.println("Adafruit GPS library basic test!");
  
  Serial.println("LoRa Sender");

  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
  }else{
    Serial.println("Starting LoRa succeeded!");
  }
  
  Serial.println("Adafruit GPS library basic test!");

  // 9600 baud is the default rate for the Adafruit Ultimate GPS module
  GPS.begin(9600);

  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz


  Serial.println("Init done...");

  Serial.println("Initializing Timer");
  // For additional information on timers see the following link:
  // https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/

  // Add your timer interrupt here

  
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 5000000, true);
  timerAlarmEnable(timer);

   // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);

  }


void loop() {
  if(counter < interruptCounter){
    counter++;
    sayHello();
    useGPS();
    checkSensor();
  }
}
