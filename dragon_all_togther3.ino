#include <SoftwareSerial.h>
#include "serial_mp3.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define NUMPIXELS 30
int num_of_strips = 2;
Adafruit_NeoPixel strips[] = {
  Adafruit_NeoPixel(NUMPIXELS, 2, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, 3, NEO_GRB + NEO_KHZ800)
};
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, PIN, NEO_GRB + NEO_KHZ800);

int folder = 3;
int song = 1;

int indicator_pins[] = { 7, 8, 9}; //{6, 7, 8, 9, 10};
int indicator_pins_count = (sizeof(indicator_pins) / sizeof(int));
int impact_sensors[] = {0, 1, 2};//{0, 1, 2, 3, 4, 5};
int impact_sensors_count = (sizeof(impact_sensors) / sizeof(int)) - 1;

int sensorValues[] = {0, 0, 0, 0, 0};
int sensorLastValues[] = {0, 0, 0, 0, 0};

// for multiplae impact areas to be litten at the same time
int currentImpact[] = {0};
int currentImpactCount = 1;

int impactThreshold = 150;

int impactTimeoutMax = 5000;
int impactTimeoutMin = 1000;

int impactCountDown = 0;
int impactStatus = 0; // '0' no impact || '1' impact

uint32_t color_map[] = {
  strips[0].Color(128, 0, 128),
  strips[0].Color(0, 0, 255),
  strips[0].Color(255, 255, 0),
  strips[0].Color(255, 0, 0)
};
uint32_t black =   strips[0].Color(0, 0, 0);
int color_pos = 0 ;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
//  Serial.begin(9600);
  randomSeed(analogRead(0));
  
  for (int i = 0; i < num_of_strips; i++) {
    strips[i].begin();
    strips[i].show(); // Initialize all pixels to 'off'
  }

  for (int i = 0; i < indicator_pins_count; i++) {
    // Serial.print("indicator_pins ");  Serial.print(indicator_pins[i]); Serial.println();
    pinMode(indicator_pins[i], OUTPUT);
    digitalWrite(indicator_pins[i], HIGH);
    digitalWrite(indicator_pins[i], LOW);
  }
 

  serialmp3_init(); // Initialize sound
  impactCountDown = impactTimeoutMin;
  
  //  serialmp3_play(folder, 1);
}

// the loop routine runs over and over again forever:
void loop() {
//    Serial.print("currentImpactCount ");  Serial.print(currentImpactCount); Serial.println();
  for (int i = 0 ; i < currentImpactCount ; i++) {
    int j = impact_sensors[currentImpact[i]];
//        Serial.print("j ");  Serial.print(j); Serial.println();
    sensorValues[j] = analogRead(j);
//Serial.print("j ");  Serial.print(j); Serial.print("value ");Serial.print(sensorValues[j]); Serial.print("last ");Serial.print(sensorLastValues[j]);  Serial.println();
    if (sensorValues[j] > sensorLastValues[j] + impactThreshold) {
      impact(currentImpact[i], sensorValues[j]);
    }
    sensorLastValues[j] = sensorValues[j];
  }


  impactCountDown--;
  //  Serial.print("impactCountDown ");  Serial.print(impactCountDown); Serial.println();
  if (impactCountDown == 0) {

    //    Serial.print("impactStatus ");  Serial.print(impactStatus); Serial.println();
    if (impactStatus == 1) { // got a hit wait for one interval
      currentImpactCount = 1;
      impactStatus = 0;
    }
    currentImpact[0] = random(impact_sensors_count + 1);
    //    Serial.print("currentImpact ");  Serial.print(currentImpact[0]); Serial.println();
    indicators_off();
    //   Serial.print("indicator_pins[currentImpact[0]] ");  Serial.print(indicator_pins[currentImpact[0]]); Serial.println();

    digitalWrite(indicator_pins[currentImpact[0]], HIGH);

    impactCountDown = random(impactTimeoutMin, impactTimeoutMax);
  }
  delay(1);        // delay in between reads for stability
}


void impact(int position, int amount) {
//    Serial.print("Impact ");  Serial.print(position); Serial.println();
  //  Serial.println(amount);
  int a = map(amount, 0, 1024, 0, 30);


  if (position == 0 || position == 2) { // left hand or head
//     Serial.print("left hand or head ");
    colorRainbow(color_map, 5, a, 0);
  }
  if (position == 1 || position == 2) { // right hand or head
//    Serial.print("right hand or head ");
    colorRainbow(color_map, 5, a, 1);
  }
    
  //song = position + 2;
  song = random(1, 7);
  //  Serial.print("song ");  Serial.print(song); Serial.println();
  //  Serial.print("folder ");  Serial.print(folder); Serial.println();
  
  serialmp3_play(folder, song);
  // delay(3000);
  // serialmp3_stop();
  //

  impactStatus = 1;
  currentImpactCount = 0;
  indicators_off();
}
void indicators_off() {
  for (int i = 0; i < indicator_pins_count; i++) {
    digitalWrite(indicator_pins[i], LOW);
  }
}

void colorRainbow(uint32_t color_map[], uint8_t wait, int until, int strip) {


  for (uint16_t i = 0; i < until; i++) {
    if ( i < NUMPIXELS / 4) {
      color_pos = 2;
    } else if ( i >= NUMPIXELS / 4 && i < (NUMPIXELS / 4) * 2 ) {
      color_pos = 1;
    } else if ( i >= (NUMPIXELS / 4) * 2 && i < (NUMPIXELS / 4) * 3 ) {
      color_pos = 0;
    } else {
      color_pos = 3;
    }

    strips[strip].setPixelColor(i, color_map[color_pos]);
    strips[strip].show();
    delay(wait);
  }

  for (uint16_t i = 0; i < until; i++) {
    strips[strip].setPixelColor(i, strips[0].Color(0, 0, 0));
  }
  strips[strip].show();

}
