// Group 1
// Solmaz Hashemzadeh
// Jonathan Combs
#include <Arduino.h>
#include <M5Stack.h>
#include <Adafruit_NeoPixel.h>

// LED stuff
#define M5STACK_FIRE_NEO_NUM_LEDS 10
#define M5STACK_FIRE_NEO_DATA_PIN 15

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(M5STACK_FIRE_NEO_NUM_LEDS, M5STACK_FIRE_NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);

//Sound stuff
#define DACPIN 25 // speaker DAC, only 8 Bit

#define SAMPLINGFREQUENCY 44100
#define NUMBEROFSAMPLES   SAMPLINGFREQUENCY * 1 // play 1 second

#define DAC_MAX_AMPLITUDE 127/20 // max value is 127, but it is too loud

#define AUDIOBUFFERLENGTH NUMBEROFSAMPLES

uint8_t AudioBuffer[AUDIOBUFFERLENGTH];


void setup() {
  // sound stuff
  const float   frequency = 440;
  const float   amplitude = DAC_MAX_AMPLITUDE;

  // store sine wave in buffer
  for (int n = 0; n < NUMBEROFSAMPLES; n++)
  {
    int16_t sineWaveSignal = ( sin( 2 * PI * frequency / SAMPLINGFREQUENCY * n )) * amplitude;
    AudioBuffer[n] = sineWaveSignal+135;
  }

  // m5 and LED setup
  Serial.begin(115200); // Sets the serial speed
  M5.begin();
  pixels.begin();
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.println("                 Hello world!");
  Serial.println("Setup is now done!");
}

int i = 10;
void loop() {
  // put your main code here, to run repeatedly:
  if(i == 1){
    uint32_t start = micros();
    for (int n = 0; n < NUMBEROFSAMPLES; n++){
      // wait for next sample
      while (start + ( 1000000UL / SAMPLINGFREQUENCY) > micros() );
      start = micros();

      dacWrite(DACPIN, AudioBuffer[n]);
    }
  }
  dacWrite(DACPIN, 0);

  if (i%10 == 0){
    if(i > 99){
      i = 0;
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 30);
    }
    M5.Lcd.println("                 Hello world ...again!");
    Serial.print(i);
    Serial.println(" is i's value now, and we just printed hello world again...");
  }
  i++;

  
  
  

  static int pixelNumber=0;// = random(0, M5STACK_FIRE_NEO_NUM_LEDS - 1);
  pixelNumber++;
  if(pixelNumber>9)pixelNumber=0;
  int r = 1<<random(0, 7);
  int g = 1<<random(0, 7);
  int b = 1<<random(0, 7);

  pixels.setPixelColor(pixelNumber, pixels.Color(r, g, b));     
  pixels.show();

  delay(100);

}
