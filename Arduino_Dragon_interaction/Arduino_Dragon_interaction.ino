#include <SoftwareSerial.h>
#include "serial_mp3.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif


// my dragon has 2 lead single addressable led strips
// Each is on a wing and interacts with the pressior sensors from right left and center

#define NUMPIXELS 30
int num_of_strips = 2;
Adafruit_NeoPixel strips[] = {
  Adafruit_NeoPixel(NUMPIXELS, 2, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(NUMPIXELS, 3, NEO_GRB + NEO_KHZ800)
};
// The color map is to create an integrated color changes on a snigle strip
uint32_t color_map[] = {
  strips[0].Color(128, 0, 128),   // yellow
  strips[0].Color(0, 0, 255),     // green
  strips[0].Color(255, 255, 0),   // Purple
  strips[0].Color(255, 0, 0)      // Red
};
// Each LED strip will be light up like so:
//       Yellow           Green          Purple          Red
//  |---------------|--------------|---------------|---------------|
uint32_t black =   strips[0].Color(0, 0, 0);
int color_pos = 0 ;
uint8_t ilumination_delay = 5; //millisec

// Serial MP3 reader uses 2 parameter
// Folder name - to address the right sounds for the dragon
// Song name - to know what song to play
// For now song is picked up in random
int folder = 3;
int song = 1;


// Each FSR is located at the below pin
// Adding more FSR is easy.

int indicator_pins[] = { 7, 8, 9}; //{6, 7, 8, 9, 10};
int indicator_pins_count = (sizeof(indicator_pins) / sizeof(int));
int impact_sensors[] = {0, 1, 2};//{0, 1, 2, 3, 4, 5};
int impact_sensors_count = (sizeof(impact_sensors) / sizeof(int)) - 1;

int sensorValues[] = {0, 0, 0, 0, 0};
int sensorLastValues[] = {0, 0, 0, 0, 0};

// For multiplae impact areas to be litten at the same time
int currentImpact[] = {0};
int currentImpactCount = 1;

// This threshold needs to be check in the setup stage and on each modification to the FSR arrray
int impactThreshold = 150;

int impactTimeoutMax = 5000;
int impactTimeoutMin = 1000;

int impactCountDown = 0;
int impactStatus = 0; // '0' no impact || '1' impact

// if you want to see the serial output change this param
bool debug = false;
// mode is the game you play with the dragon
// for now there are 2 game modes
// 0 basic for kids
// 1 mode advance
int mode = 1 ;

// the setup routine runs once when you press reset:
void setup() {
  if (debug) {
    Serial.begin(9600);
  }
  // for randome initilization
  randomSeed(analogRead(0));

  // Init lead strip
  for (int i = 0; i < num_of_strips; i++) {
    strips[i].begin();
    strips[i].show(); // Initialize all pixels to 'off'
  }

  // Setup FSR sensors
  for (int i = 0; i < indicator_pins_count; i++) {
    // Serial.print("indicator_pins ");  Serial.print(indicator_pins[i]); Serial.println();
    pinMode(indicator_pins[i], OUTPUT);
    digitalWrite(indicator_pins[i], HIGH);
    digitalWrite(indicator_pins[i], LOW);
  }
  // Show impact avilable after impactTimeoutMin
  impactCountDown = impactTimeoutMin;

  // Setup Serial MP3
  serialmp3_init(); // Initialize sound
}

// the loop routine runs over and over again forever:
void loop() {
  if (debug) {
    Serial.print("currentImpactCount ");  Serial.print(currentImpactCount); Serial.println();
  }

  // Check the sensor values of current impact zone
  for (int i = 0 ; i < currentImpactCount ; i++) {
    int j = impact_sensors[currentImpact[i]];
    if (debug) {
      Serial.print("j ");  Serial.print(j); Serial.println();
    }
    sensorValues[j] = analogRead(j);
    if (debug) {
      Serial.print("j ");  Serial.print(j); Serial.print("value "); Serial.print(sensorValues[j]); Serial.print("last "); Serial.print(sensorLastValues[j]);  Serial.println();
    }
    if (sensorValues[j] > sensorLastValues[j] + impactThreshold) {
      impact(currentImpact[i], sensorValues[j]);
    }
    sensorLastValues[j] = sensorValues[j];
  }


  // Delay impact surfaces changes
  impactCountDown--;
  if (debug) {
    Serial.print("impactCountDown ");  Serial.print(impactCountDown); Serial.println();
  }
  // current game mode
  switch (mo) {
    case 0:
      mode0();
      break;
    case 1:
      mode1();
      break;
  }


  delay(1);        // delay in between reads for stability use min 1
}

// mode0 is the basic interaction
// you need to attack all body parts as fast as you can
void mode0() {
  if (impactCountDown == 0) {
    if (impactStatus == 1) { // got a hit wait for one interval
      currentImpactCount = 1;
      impactStatus = 0;
    }

    indicators_off();

    // write a for loop
    for (int i = 0 ; i < currentImpactCount ; i++) {
      currentImpact[i] = random(impact_sensors_count + 1);
      digitalWrite(indicator_pins[currentImpact[i]], HIGH);
    }


    impactCountDown = random(impactTimeoutMin, impactTimeoutMax);
  }
}
// mode1 is the basic interaction that every time X Sensor are lit up
// i call it the Q mode because you wait for the Dragon to attack and then fight back
void mode1() {
  if (impactCountDown == 0) {
    if (debug) {
      Serial.print("impactStatus ");  Serial.print(impactStatus); Serial.println();
    }
    if (impactStatus == 1) { // got a hit wait for one interval
      currentImpactCount = 1;
      impactStatus = 0;
    }

    currentImpact[0] = random(impact_sensors_count + 1);
    if (debug) {
      Serial.print("currentImpact ");  Serial.print(currentImpact[0]); Serial.println();
    }
    indicators_off();

    if (debug) {
      Serial.print("indicator_pins[currentImpact[0]] ");  Serial.print(indicator_pins[currentImpact[0]]); Serial.println();
    }

    digitalWrite(indicator_pins[currentImpact[0]], HIGH);


    impactCountDown = random(impactTimeoutMin, impactTimeoutMax);
  }
}
// mode2 will be an interaction game that will work with patterns
void mode2 () {
}
void impact(int position, int amount) {
  if (debug) {
    Serial.print("Impact ");  Serial.print(position); Serial.print("\t"); Serial.println(amount); Serial.println();
  }

  // so i decided position 0 is the Head
  // Head shot both LED strip lights UP according to strangth
  // Hand Gets hit only single LED strip is litten
  int a = map(amount, 0, 1024, 0, NUMPIXELS);
  if (position == 0 || position == 2) { // left hand or head
    if (debug) {
      Serial.print("left hand or head ");
    }
    colorRainbow(color_map, ilumination_delay, a, 0);
  }
  if (position == 1 || position == 2) { // right hand or head
    if (debug) {
      Serial.print("right hand or head ");
    }
    colorRainbow(color_map, ilumination_delay, a, 1);
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

// Change all led indicators to Off
void indicators_off() {
  for (int i = 0; i < indicator_pins_count; i++) {
    digitalWrite(indicator_pins[i], LOW);
  }
}

// This function adds the magic...
// If you read up to hear your great :)
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
