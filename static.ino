// Arduino Pin Assignments
const int pinA = 2; // LSB
const int pinB = 3;
const int pinC = 4;
const int pinD = 5; // MSB

// REVERSED DIGIT PINS: Swap pin order 6 through 11
const int digitPins[6] = {6, 7, 8, 9, 10, 11}; 

int hours = 88;
int minutes =88;
int seconds = 88;

unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);

  for (int i = 0; i < 6; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW);
  }
}

void loop() {
  unsigned long currentMillis = millis();
  /*if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    seconds++;
    if (seconds >= 60) {
      seconds = 0;
      minutes++;
      if (minutes >= 60) {
        minutes = 0;
        hours++;
        if (hours >= 24) hours = 0;
      }
    }
  }*/

  refreshDisplay();
}

void writeBCD(int val) {
  digitalWrite(pinA, (val & 0x01) ? HIGH : LOW);
  digitalWrite(pinB, (val & 0x02) ? HIGH : LOW);
  digitalWrite(pinC, (val & 0x04) ? HIGH : LOW);
  digitalWrite(pinD, (val & 0x08) ? HIGH : LOW);
}

void refreshDisplay() {
  int digits[6];
  
  // Directly assignment from Top-to-Bottom / Left-to-Right
  digits[0] = hours / 10;   // Digit 1 (Top/Left)
  digits[1] = hours % 10;   // Digit 2
  digits[2] = minutes / 10; // Digit 3
  digits[3] = minutes % 10; // Digit 4
  digits[4] = seconds / 10; // Digit 5
  digits[5] = seconds % 10; // Digit 6 (Bottom/Right)

  for (int i = 0; i < 6; i++) {
    writeBCD(digits[i]);
    digitalWrite(digitPins[i], HIGH);
    delayMicroseconds(2000);
    digitalWrite(digitPins[i], LOW);
  }
}
