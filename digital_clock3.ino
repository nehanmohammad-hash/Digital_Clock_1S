// Arduino Pin Assignments
const int pinA = 2; // LSB
const int pinB = 3;
const int pinC = 4;
const int pinD = 5; // MSB

const int segmentAPin = 12; // Pin 12 for upper segment 'a' (fixes 6)
const int segmentDPin = 13; // Pin 13 for lower segment 'd' (fixes 9)

// REVERSED DIGIT PINS: Swap pin order 6 through 11
const int digitPins[6] = {6, 7, 8, 9, 10, 11}; 

// Button Pins (Using A0, A1, A2, A3 as digital inputs)
const int btnEditPin = A0;   // 1st button: Enter/Exit Edit Mode
const int btnNextPin = A1;   // 2nd button: Move cursor right (loops)
const int btnIncPin  = A2;   // 3rd button: Increase number
const int btnDecPin  = A3;   // 4th button: Decrease number

int hours = 12;
int minutes = 0;
int seconds = 0;

// Edit Mode State Variables
bool editMode = false;
int cursorIndex = 0; // 0 to 5 for the 6 digits

// Button debounce & blinking tracking
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 250; // ms
unsigned long blinkPreviousMillis = 0;
bool blinkState = true;
const long blinkInterval = 300; // Blink speed in ms

unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);
  
  pinMode(segmentAPin, OUTPUT);
  pinMode(segmentDPin, OUTPUT);
  
  digitalWrite(segmentAPin, HIGH); 
  digitalWrite(segmentDPin, HIGH); 

  for (int i = 0; i < 6; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW);
  }

  pinMode(btnEditPin, INPUT);
  pinMode(btnNextPin, INPUT);
  pinMode(btnIncPin, INPUT);
  pinMode(btnDecPin, INPUT);
}

void loop() {
  unsigned long currentMillis = millis();

  // Clock runs ONLY when NOT in edit mode
  if (!editMode) {
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
  } else {
    // Handle blinking state timer when in edit mode
    if (currentMillis - blinkPreviousMillis >= blinkInterval) {
      blinkPreviousMillis = currentMillis;
      blinkState = !blinkState;
    }
  }

  // Handle user inputs via buttons
  handleButtons();

  refreshDisplay();
}

void handleButtons() {
  if (millis() - lastDebounceTime > debounceDelay) {
    
    // 1st Button: Enter/Exit Edit Mode
    if (digitalRead(btnEditPin) == HIGH) {
      editMode = !editMode; 
      cursorIndex = 0;      
      lastDebounceTime = millis();
      return;
    }

    // Only allow other buttons to work if Edit Mode is active
    if (editMode) {
      int h1 = hours / 10;
      int h2 = hours % 10;
      int m1 = minutes / 10;
      int m2 = minutes % 10;
      int s1 = seconds / 10;
      int s2 = seconds % 10;

      // 2nd Button: Move cursor right (loops from far right back to far left)
      if (digitalRead(btnNextPin) == HIGH) {
        cursorIndex = (cursorIndex + 1) % 6;
        lastDebounceTime = millis();
        return;
      }

      // 3rd Button: Increase value
      if (digitalRead(btnIncPin) == HIGH) {
        if (cursorIndex == 0) h1 = (h1 + 1) % 3; 
        else if (cursorIndex == 1) {
          h2 = (h2 + 1) % 10;
          if (h1 == 2 && h2 > 3) h2 = 0; 
        }
        else if (cursorIndex == 2) m1 = (m1 + 1) % 6; 
        else if (cursorIndex == 3) m2 = (m2 + 1) % 10; 
        else if (cursorIndex == 4) s1 = (s1 + 1) % 6; 
        else if (cursorIndex == 5) s2 = (s2 + 1) % 10; 
        
        recalculateTime(h1, h2, m1, m2, s1, s2);
        lastDebounceTime = millis();
        return;
      }

      // 4th Button: Decrease value
      if (digitalRead(btnDecPin) == HIGH) {
        if (cursorIndex == 0) h1 = (h1 - 1 < 0) ? 2 : h1 - 1;
        else if (cursorIndex == 1) {
          h2 = (h2 - 1 < 0) ? 9 : h2 - 1;
          if (h1 == 2 && h2 > 3) h2 = 3;
        }
        else if (cursorIndex == 2) m1 = (m1 - 1 < 0) ? 5 : m1 - 1;
        else if (cursorIndex == 3) m2 = (m2 - 1 < 0) ? 9 : m2 - 1;
        else if (cursorIndex == 4) s1 = (s1 - 1 < 0) ? 5 : s1 - 1;
        else if (cursorIndex == 5) s2 = (s2 - 1 < 0) ? 9 : s2 - 1;

        recalculateTime(h1, h2, m1, m2, s1, s2);
        lastDebounceTime = millis();
        return;
      }
    }
  }
}

void recalculateTime(int h1, int h2, int m1, int m2, int s1, int s2) {
  hours = (h1 * 10) + h2;
  if (hours > 23) hours = 23;
  minutes = (m1 * 10) + m2;
  seconds = (s1 * 10) + s2;
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
    // If in edit mode, on the active cursor digit, and blinkState is false, skip lighting it up (creates blink effect)
    if (editMode && (i == cursorIndex) && !blinkState) {
      continue; 
    }

    writeBCD(digits[i]);

    // Separate control block for number 6 (Upper segment 'a')
    if (digits[i] == 6) {
      digitalWrite(segmentAPin, LOW); 
    } else {
      digitalWrite(segmentAPin, HIGH);
    }

    // Separate control block for number 9 (Lower segment 'd')
    if (digits[i] == 9) {
      digitalWrite(segmentDPin, LOW); 
    } else {
      digitalWrite(segmentDPin, HIGH);
    }

    digitalWrite(digitPins[i], HIGH);
    delayMicroseconds(2000);
    digitalWrite(digitPins[i], LOW);
  }
  
  digitalWrite(segmentAPin, HIGH);
  digitalWrite(segmentDPin, HIGH);
}
