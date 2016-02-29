#include <EEPROM.h>
#include <FastLED.h>
#define NUM_SLOTS 20
#define DATA_PIN 12
#define PIN1A 7
#define PIN1B 6
#define PIN1C 5
#define PIN2A 10
#define PIN2B 9
#define PIN2C 8
#define ANIMSPEED 60

CRGB   leds[NUM_SLOTS];

static unsigned long lWaitMillis;

// Initial hal-referencevalues for On state for each slot
int valueOn[NUM_SLOTS] = {
  1023,1023,1023,1023,1023,
  1023,1023,1023,1023,1023,
  1023,1023,1023,1023,1023,
  1023,1023,1023,1023,1023
};

// Initial hal-referencevalues for Off state for each slot
int valueOff[NUM_SLOTS] = {
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

int needCalibrate = 1;
int forceCalibration = 1;

void setup() {
  Serial.begin(9600);
  pinMode(PIN1A, OUTPUT);
  pinMode(PIN1B, OUTPUT);
  pinMode(PIN1C, OUTPUT);
  pinMode(PIN2A, OUTPUT);
  pinMode(PIN2B, OUTPUT);
  pinMode(PIN2C, OUTPUT);
  FastLED.addLeds<WS2812,DATA_PIN, GRB>(leds, NUM_SLOTS);
  lWaitMillis = millis() + ANIMSPEED;

  if (forceCalibration == 0) {
    if ( EEPROM.read ( 0 ) != 0xff ) {
      for (int i = 0; i < NUM_SLOTS; ++i ) {
        valueOn[i] = EEPROM.read(i);
      }
      for (int i = 0; i < NUM_SLOTS; ++i ) {
        valueOff[i] = EEPROM.read(i+NUM_SLOTS);
      } 
      for (int i = 0; i < NUM_SLOTS; ++i ) {
        calibrate[i] = 0;
      }  
      needCalibrate = 0;
    }
  }
}

// Initial hal-values for each slot
long value[NUM_SLOTS] = {
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0
  };

// Initial holidy status from all elements
int holiday[NUM_SLOTS] = {
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0,
  0,0,0,0,0
};



int statusFlags[NUM_SLOTS] = {
  2, 2, 2, 2, 2,
  2, 2, 2, 2, 2,
  2, 2, 2, 2, 2,
  2, 2, 2, 2, 2
};

int lastStatusFlags[NUM_SLOTS] = {
  2, 2, 2, 2, 2,
  2, 2, 2, 2, 2,
  2, 2, 2, 0, 2,
  2, 2, 2, 2, 2
};

int blockLedWrite[NUM_SLOTS] = {
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0
};

int rippleEffect[NUM_SLOTS] = {
  20, 20, 20, 20, 20,
  20, 20, 20, 20, 20,
  20, 20, 20, 20, 20,
  20, 20, 20, 20, 20
};

int rippleEffectDirection[NUM_SLOTS] = {
  20, 20, 20, 20, 20,
  20, 20, 20, 20, 20,
  20, 20, 20, 20, 20,
  20, 20, 20, 20, 20
};

// iteration Settings
int flatteningIteration = 0;
int flatteningIterations = 10;

// calibration Settings
int onOffOffset = 30;

String holidayStatus;

void loop() {

  // Save Calibration to EPROM
  /*
  if (needCalibrate == 1) {
    int allCalibrated = 1;
    for (int i = 0; i < NUM_SLOTS; ++i ) {
      if (calibrate[i] == 1) allCalibrated = 0;
    }
    if (allCalibrated) {
      needCalibrate = 0;
      for (int i = 0; i < NUM_SLOTS; ++i ) {
        EEPROM.write(i, valueOn[i]);
      }
      for (int i = 0; i < NUM_SLOTS; ++i ) {
        EEPROM.write(i+NUM_SLOTS, valueOff[i]);
      } 
    }
  }
  */
  
  if(Serial.available() > 0) {
        holidayStatus = Serial.readStringUntil('\n');
        
        for (int i = 0; i < holidayStatus.length(); i++) {
            if (i < NUM_SLOTS) {
              holiday[i] = holidayStatus[i] - '0';
            }
        }
    }

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
        statusFlags[slot] = 1;
      // If value is nearer on the reference value for Off
      } else {
        statusFlags[slot] = 0;
      }

      if (holiday[slot] == 1) {
        statusFlags[slot] = 3;
      }
      
      if (holiday[slot] == 2) {
        statusFlags[slot] = 4;
      }

      // if more calibration is needed
      if ((valueOff[slot] - valueOn[slot]) < onOffOffset) {
        statusFlags[slot] = 2;
      // if calibration is complete
      } else {
        calibrate[slot] = 0;
      }

      Serial.print(statusFlags[slot]);
    }
    Serial.println('E');
    flatteningIteration = -1;
  }

  for (int slot = 0; slot < NUM_SLOTS; slot++) {
    if (lastStatusFlags[slot] != statusFlags[slot]) {
      rippleEffect[slot] = 0;
      if (statusFlags[slot] == 1) {
        rippleEffectDirection[slot] = NUM_SLOTS;
      } else {
        rippleEffectDirection[slot] = 0;
      }
      lastStatusFlags[slot] = statusFlags[slot];
    }

    if (blockLedWrite[slot] == 0) {
      if (statusFlags[slot] == 1) {
        leds[slot] = 0x65FB06;
      } else {
        leds[slot] = 0x300000;
      }
      if (statusFlags[slot] == 3) {
        leds[slot] = 0x00FFCC;
      }
      if (statusFlags[slot] == 4) {
        leds[slot] = 0xFFDD00;
      }
      if (statusFlags[slot] == 2) {
        leds[slot] = 0x0000FF;
      }
    }
  }

  if( (long)( millis() - lWaitMillis ) >= 0) {

    for (int slot = 0; slot < NUM_SLOTS; slot++) {

      blockLedWrite[slot] = 0;

      if (rippleEffect[slot] < NUM_SLOTS) {
        int notDrawn = 1;

        //do {
          int slotNr = slot;
          int pos1t = slotNr + rippleEffect[slot];
          int pos2t = slotNr - rippleEffect[slot];

          if (pos1t < NUM_SLOTS && pos1t >= 0) {
            leds[pos1t] = 0xFFFFFF;
            blockLedWrite[pos1t] = 1;
            notDrawn = 0;
          }

          if (pos2t >= 0 && pos2t < NUM_SLOTS) {
            leds[pos2t] = 0xFFFFFF;
            blockLedWrite[pos2t] = 1;
            notDrawn = 0;
          }
          rippleEffect[slot]++;
        //} while (notDrawn==1);

      }

    }
    lWaitMillis += ANIMSPEED;
  }
  FastLED.show();

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
