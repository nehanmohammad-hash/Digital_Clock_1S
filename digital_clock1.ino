// Arduino Pin Assignments
const int pinA = 2; // LSB
const int pinB = 3;
const int pinC = 4;
const int pinD = 5; // MSB

const int segmentAPin = 12; // NEW: Extra pin to force segment 'a' ON

// REVERSED DIGIT PINS: Swap pin order 6 through 11
const int digitPins[6] = {6, 7, 8, 9, 10, 11}; 

int hours = 12;
int minutes = 0;
int seconds = 0;

unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);
  
  pinMode(segmentAPin, OUTPUT);
  digitalWrite(segmentAPin, HIGH); // Default HIGH (inactive for active-low)

  for (int i = 0; i < 6; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW);
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
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
  }

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
  
  digits[0] = hours / 10;   
  digits[1] = hours % 10;   
  digits[2] = minutes / 10; 
  digits[3] = minutes % 10; 
  digits[4] = seconds / 10; 
  digits[5] = seconds % 10; 

  for (int i = 0; i < 6; i++) {
    writeBCD(digits[i]);

    // If the current digit being multiplexed is a 6, force segment 'a' LOW
    if (digits[i] == 6) {
      digitalWrite(segmentAPin, LOW); 
    } else {
      digitalWrite(segmentAPin, HIGH);
    }

    digitalWrite(digitPins[i], HIGH);
    delayMicroseconds(2000);
    digitalWrite(digitPins[i], LOW);
  }
  
  // Reset segment pin state at the end of the refresh cycle
  digitalWrite(segmentAPin, HIGH);
}
