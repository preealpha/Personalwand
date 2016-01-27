#include <FastLED.h>
#define NUM_SLOTS 20
#define DATA_PIN 12
#define PIN1A 7
#define PIN1B 6
#define PIN1C 5
#define PIN2A 10
#define PIN2B 9
#define PIN2C 8

CRGB   leds[NUM_SLOTS];

void setup() {
  Serial.begin(9600);
  pinMode(PIN1A, OUTPUT);
  pinMode(PIN1B, OUTPUT);
  pinMode(PIN1C, OUTPUT);
  pinMode(PIN2A, OUTPUT);
  pinMode(PIN2B, OUTPUT);
  pinMode(PIN2C, OUTPUT);
  FastLED.addLeds<WS2812,DATA_PIN, GRB>(leds, NUM_SLOTS);
}

// Initial hal-referencevalues for Off state for each slot
int valueOff[NUM_SLOTS] = {
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0
};

// Initial hal-referencevalues for On state for each slot
int valueOn[NUM_SLOTS] = {
  1023,1023,1023,1023,1023,
  1023,1023,1023,1023,1023,
  1023,1023,1023,1023,1023,
  1023,1023,1023,1023,1023
};

// Initial hal-values for each slot
long value[NUM_SLOTS] = {
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0
  };

// Initial "needs to be calibrated"-flag for each slot
int calibrate[NUM_SLOTS] = {
  1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1
};

// iteration Settings
int flatteningIteration = 0;
int flatteningIterations = 50;

// calibration Settings
int onOffOffset = 20;

void loop() {
  // If this is the first run, just read the hal-values
  if (flatteningIteration == 0) {
    for (int slot = 0; slot < NUM_SLOTS; slot++) {
      value[slot] = readSensor(slot);
    }
  }

  // on every other run, add the hal-value to the initial value
  if (flatteningIteration > 0) {
    for (int slot = 0; slot < NUM_SLOTS; slot++) {
      value[slot] += readSensor(slot);
    }
  }

  // if it is the last run, do the magic
  if (flatteningIteration >= flatteningIterations) {
    
    for (int slot = 0; slot < NUM_SLOTS; slot++) {
      // Calculate the average value of all iterations
      value[slot] = value[slot]/flatteningIterations;
      Serial.print(slot);
      Serial.print('-');
      Serial.print(value[slot]);
      Serial.print('-');

      // If calibration is needed
      if (calibrate[slot] == 1) {
        if (value[slot] > valueOff[slot]) valueOff[slot] = (valueOff[slot] + value[slot]) / 2;
        if (value[slot] < valueOn[slot]) valueOn[slot] = (valueOn[slot] + value[slot]) / 2;
      }

      // Calculate how far away the actual value is from the referencevalues
      int offsetOff = value[slot]-valueOff[slot];
      int offsetOn = value[slot]-valueOn[slot];

      // If value is nearer on the reference value for On
      if (abs(offsetOn) < abs(offsetOff)) {
        leds[slot] = 0x65FB06;
      // If value is nearer on the reference value for Off
      } else {
        leds[slot] = 0x300000;
      }

      // if more calibration is needed
      if ((valueOff[slot] - valueOn[slot]) < onOffOffset) {
      leds[slot] = 0x0000FF;

      // if calibration is complete
      } else {
        calibrate[slot] = 0;
      }
      
      Serial.print('-');
    }
    Serial.println('E');
    FastLED.show();  
    flatteningIteration = -1;
  }
  flatteningIteration++;
}

int readSensor(int slot) {

   // select the bit
  int value1A = bitRead(slot,0);    
  int value1B = bitRead(slot,1);
  int value1C = bitRead(slot,2);
  int value2A = bitRead(slot,3);    
  int value2B = bitRead(slot,4);
  int value2C = bitRead(slot,5);

  // Write the bits to the multiplexer
  digitalWrite(PIN1A, value1A);
  digitalWrite(PIN1B, value1B);
  digitalWrite(PIN1C, value1C);
  digitalWrite(PIN2A, value2A);
  digitalWrite(PIN2B, value2B);
  digitalWrite(PIN2C, value2C);

  // Read the hal-value
  return analogRead(A0);
}

