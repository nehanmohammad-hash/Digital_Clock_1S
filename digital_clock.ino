// ============================================================
//            6-DIGIT DIGITAL CLOCK (WITH EDIT MODE)
//            7447 BCD DRIVER + MULTIPLEXED DISPLAYS
// ============================================================


//First project-1st Semister. 
//Courtsey of Professor GVV Sharma
//Courtsey of Teaching assistants; Subhodeep, Jnanesh, Vivek



// ---------------- Shared 7447 Inputs ----------------
const int PIN_A = 2;
const int PIN_B = 3;
const int PIN_C = 4;
const int PIN_D = 5;

// ---------------- Custom Segment Override Pins ----------------
const int segmentAPin = 12; // Pin 12 for upper segment 'a' (fixes 6)
const int segmentDPin = 13; // Pin 13 for lower segment 'd' (fixes 9)

// ---------------- Enable pins for multiplexing ----------------
// sec1, sec10, min1, min10, hr1, hr10
const int EN[] = {11,10,9,8,7,6};
const int NUM_DIGITS = 6;

// ---------------- Buttons ----------------
const int PAUSE_BTN = A0; // 1st button: Enter/Exit Edit mode (pauses clock)
const int NEXT_BTN  = A1; // 2nd button: Move cursor right (loops)
const int INC_BTN   = A2; // 3rd button: Increase value
const int DEC_BTN   = A3; // 4th button: Decrease value

// ---------------- Button states ----------------
bool paused = false;

int lastPauseState = HIGH;
int lastNextState  = HIGH;
int lastIncState   = HIGH;
int lastDecState   = HIGH;

// ---------------- Digit selection when paused ----------------
// 0 = sec ones, 1 = sec tens, 2 = min ones, 3 = min tens, 4 = hour ones, 5 = hour tens
int selectedDigit = 0;

// ---------------- Blink ----------------
unsigned long lastBlink = 0;
bool blinkOn = true;

// ---------------- Current States (Initialized to 12:00:00) ----------------
// Format follows your reference bit tracking (W, X, Y, Z)

// Seconds ones (0) -> W=0, X=0, Y=0, Z=0
int W1 = 0; int X1 = 0; int Y1 = 0; int Z1 = 0;

// Seconds tens (0) -> W=0, X=0, Y=0
int W2 = 0; int X2 = 0; int Y2 = 0;

// Minutes ones (0) -> W=0, X=0, Y=0, Z=0
int W3 = 0; int X3 = 0; int Y3 = 0; int Z3 = 0;

// Minutes tens (0) -> W=0, X=0, Y=0
int W4 = 0; int X4 = 0; int Y4 = 0;

// Hours ones (2 for 12:00:00) -> 2 in BCD is W=0, X=1, Y=0, Z=0
int W5 = 0; int X5 = 1; int Y5 = 0; int Z5 = 0;

// Hours tens (1 for 12:00:00) -> 1 in BCD is W=1, X=0, Y=0
int W6 = 1; int X6 = 0; int Y6 = 0;

// ---------------- Timer ----------------
unsigned long lastSecUpdate = 0;


// ============================================================
//                     DISPLAY CONTROL
// ============================================================

void allDisplaysOff()
{
  for(int i = 0; i < NUM_DIGITS; i++)
  {
    digitalWrite(EN[i], LOW);
  }
  // Also turn off active-low segment hardware override lines
  digitalWrite(segmentAPin, HIGH);
  digitalWrite(segmentDPin, HIGH);
}

void showDigit(int A, int B, int C, int D, int ENpin)
{
  allDisplaysOff();
  delayMicroseconds(50);

  // Set BCD inputs
  digitalWrite(PIN_A, A);
  digitalWrite(PIN_B, B);
  digitalWrite(PIN_C, C);
  digitalWrite(PIN_D, D);

  // Check if current active BCD represents a 6 or 9 to trigger custom fix pins
  // Value 6 = Binary 0110 (A=0, B=1, C=1, D=0) -> fixes segment 'a' via Pin 12
  if (A == 0 && B == 1 && C == 1 && D == 0) {
    digitalWrite(segmentAPin, LOW);
  }
  // Value 9 = Binary 1001 (A=1, B=0, C=0, D=1) -> fixes segment 'd' via Pin 13
  else if (A == 1 && B == 0 && C == 0 && D == 1) {
    digitalWrite(segmentDPin, LOW);
  }

  delayMicroseconds(50);
  digitalWrite(ENpin, HIGH);
}


// ============================================================
//                      SETUP
// ============================================================

void setup()
{
  // 7447 BCD inputs
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_C, OUTPUT);
  pinMode(PIN_D, OUTPUT);

  // Segment Overrides
  pinMode(segmentAPin, OUTPUT);
  pinMode(segmentDPin, OUTPUT);
  digitalWrite(segmentAPin, HIGH);
  digitalWrite(segmentDPin, HIGH);

  // Display enable pins
  for(int i = 0; i < NUM_DIGITS; i++)
  {
    pinMode(EN[i], OUTPUT);
  }

  allDisplaysOff();

  // Buttons (using external pull-down or internal pull-up depending on setup; matching reference INPUT_PULLUP)
  pinMode(PAUSE_BTN, INPUT_PULLUP);
  pinMode(NEXT_BTN, INPUT_PULLUP);
  pinMode(INC_BTN, INPUT_PULLUP);
  pinMode(DEC_BTN, INPUT_PULLUP);
}


// ============================================================
//                   INCREMENT DIGIT
// ============================================================

void incrementDigit(int d)
{
  int A, B, C, D;

  switch(d)
  {
    case 0: // Seconds ones: 0-9
    {
      A = !W1;
      B = (W1 && !X1 && !Z1) || (!W1 && X1);
      C = (!X1 && Y1) || (!W1 && Y1) || (W1 && X1 && !Y1);
      D = (!W1 && Z1) || (W1 && X1 && Y1);
      W1 = A; X1 = B; Y1 = C; Z1 = D;
    }
    break;

    case 1: // Seconds tens: 0-5
    {
      A = !W2;
      B = (W2 && !X2 && !Y2) || (!W2 && X2);
      C = (W2 && X2) || (!W2 && !X2 && Y2);
      W2 = A; X2 = B; Y2 = C;
    }
    break;

    case 2: // Minutes ones: 0-9
    {
      A = !W3;
      B = (W3 && !X3 && !Z3) || (!W3 && X3);
      C = (!X3 && Y3) || (!W3 && Y3) || (W3 && X3 && !Y3);
      D = (!W3 && Z3) || (W3 && X3 && Y3);
      W3 = A; X3 = B; Y3 = C; Z3 = D;
    }
    break;

    case 3: // Minutes tens: 0-5
    {
      A = !W4;
      B = (W4 && !X4 && !Y4) || (!W4 && X4);
      C = (W4 && X4) || (!W4 && !X4 && Y4);
      W4 = A; X4 = B; Y4 = C;
    }
    break;

    case 4: // Hours ones
    {
      if(X6 == 0)
      {
        A = !W5;
        B = (W5 && !X5 && !Z5) || (!W5 && X5);
        C = (!X5 && Y5) || (!W5 && Y5) || (W5 && X5 && !Y5);
        D = (!W5 && Z5) || (W5 && X5 && Y5);
        W5 = A; X5 = B; Y5 = C; Z5 = D;
      }
      else
      {
        A = !W5;
        B = (W5 && !X5) || (!W5 && X5);
        W5 = A; X5 = B; Y5 = 0; Z5 = 0;
      }
    }
    break;

    case 5: // Hours tens: 0-2
    {
      if(!(X6 == 0 && W6 == 1 && Y5 == 1))
      {
        A = !W6 && !X6;
        B = W6 && !X6;
        W6 = A; X6 = B; Y6 = 0;
      }
    }
    break;
  }
}


// ============================================================
//                   DECREMENT DIGIT
// ============================================================

void decrementDigit(int d)
{
  int A, B, C, D;

  switch(d)
  {
    case 0: // Seconds ones
    {
      A = !W1;
      B = (!X1 && !W1 && ((!Z1 && Y1) || (Z1 && !Y1))) || (!Z1 && W1 && X1);
      C = (!Z1 && Y1 && (X1 || W1)) || (Z1 && !X1 && !W1 && !Y1);
      D = !X1 && !Y1 && ((Z1 && W1) || (!Z1 && !W1));
      W1 = A; X1 = B; Y1 = C; Z1 = D;
    }
    break;

    case 1: // Seconds tens
    {
      A = !W2;
      B = (Y2 && !X2 && !W2) || (!Y2 && X2 && W2);
      C = !X2 && ((Y2 && W2) || (!Y2 && !W2));
      W2 = A; X2 = B; Y2 = C;
    }
    break;

    case 2: // Minutes ones
    {
      A = !W3;
      B = (!X3 && !W3 && ((!Z3 && Y3) || (Z3 && !Y3))) || (!Z3 && W3 && X3);
      C = (!Z3 && Y3 && (X3 || W3)) || (Z3 && !X3 && !W3 && !Y3);
      D = !X3 && !Y3 && ((Z3 && W3) || (!Z3 && !W3));
      W3 = A; X3 = B; Y3 = C; Z3 = D;
    }
    break;

    case 3: // Minutes tens
    {
      A = !W4;
      B = (Y4 && !X4 && !W4) || (!Y4 && X4 && W4);
      C = !X4 && ((Y4 && W4) || (!Y4 && !W4));
      W4 = A; X4 = B; Y4 = C;
    }
    break;

    case 4: // Hours ones
    {
      if(X6 == 0)
      {
        A = !W5;
        B = (!X5 && !W5 && ((!Z5 && Y5) || (Z5 && !Y5))) || (!Z5 && W5 && X5);
        C = (!Z5 && Y5 && (X5 || W5)) || (Z5 && !X5 && !W5 && !Y5);
        D = !X5 && !Y5 && ((Z5 && W5) || (!Z5 && !W5));
        W5 = A; X5 = B; Y5 = C; Z5 = D;
      }
      else
      {
        A = !W5;
        B = (X5 && W5) || (!X5 && !W5);
        W5 = A; X5 = B; Y5 = 0; Z5 = 0;
      }
    }
    break;

    case 5: // Hours tens
    {
      if(!(X6 == 0 && W6 == 0 && Y5 == 1))
      {
        A = X6 && !W6;
        B = !X6 && !W6;
        W6 = A; X6 = B; Y6 = 0;
      }
    }
    break;
  }
}


// ============================================================
//                    DISPLAY REFRESH
// ============================================================

void refreshDisplay()
{
  for(int d = 0; d < NUM_DIGITS; d++)
  {
    // If paused and selected digit matches, blink it OFF periodically
    if(paused && d == selectedDigit && !blinkOn)
    {
      allDisplaysOff();
      delayMicroseconds(1000);
      continue;
    }

    switch(d)
    {
      case 0: showDigit(W1, X1, Y1, Z1, EN[0]); break;
      case 1: showDigit(W2, X2, Y2, 0,  EN[1]); break;
      case 2: showDigit(W3, X3, Y3, Z3, EN[2]); break;
      case 3: showDigit(W4, X4, Y4, 0,  EN[3]); break;
      case 4: showDigit(W5, X5, Y5, Z5, EN[4]); break;
      case 5: showDigit(W6, X6, Y6, 0,  EN[5]); break;
    }

    delayMicroseconds(1000);
  }
}


// ============================================================
//                       MAIN LOOP
// ============================================================

void loop()
{
  // ==========================================================
  // PAUSE BUTTON (A0) - Enter/Exit Edit Mode
  // ==========================================================
  int pauseState = digitalRead(PAUSE_BTN);
  if(pauseState == LOW && lastPauseState == HIGH)
  {
    paused = !paused;
    if(paused)
    {
      selectedDigit = 0; // Starts edit selection loop at rightmost digit (sec ones)
      blinkOn = true;
      lastBlink = millis();
    }
    delay(50);
  }
  lastPauseState = pauseState;

  // ==========================================================
  // NEXT BUTTON (A1) - Move right across digits (Loops back)
  // ==========================================================
  int nextState = digitalRead(NEXT_BTN);
  if(paused && nextState == LOW && lastNextState == HIGH)
  {
    selectedDigit = (selectedDigit + 1) % 6; // Loops from right to left / wraps around
    blinkOn = true;
    lastBlink = millis();
    delay(50);
  }
  lastNextState = nextState;

  // ==========================================================
  // INCREMENT BUTTON (A2)
  // ==========================================================
  int incState = digitalRead(INC_BTN);
  if(paused && incState == LOW && lastIncState == HIGH)
  {
    incrementDigit(selectedDigit);
    delay(50);
  }
  lastIncState = incState;

  // ==========================================================
  // DECREMENT BUTTON (A3)
  // ==========================================================
  int decState = digitalRead(DEC_BTN);
  if(paused && decState == LOW && lastDecState == HIGH)
  {
    decrementDigit(selectedDigit);
    delay(50);
  }
  lastDecState = decState;

  // ==========================================================
  // BLINK SELECTED DIGIT WHEN PAUSED
  // ==========================================================
  if(paused && millis() - lastBlink >= 400)
  {
    blinkOn = !blinkOn;
    lastBlink = millis();
  }

  // ==========================================================
  // REFRESH DISPLAY
  // ==========================================================
  refreshDisplay();

  // ==========================================================
  // AUTOMATIC TIME UPDATE (Runs only when NOT paused)
  // ==========================================================
  if(!paused && millis() - lastSecUpdate >= 1000)
  {
    lastSecUpdate = millis();

    // Seconds ones
    int A = !W1;
    int B = (W1 && !X1 && !Z1) || (!W1 && X1);
    int C = (!X1 && Y1) || (!W1 && Y1) || (W1 && X1 && !Y1);
    int D = (!W1 && Z1) || (W1 && X1 && Y1);
    W1 = A; X1 = B; Y1 = C; Z1 = D;

    // Seconds tens
    if((W1 | X1 | Y1 | Z1) == 0)
    {
      A = !W2;
      B = (W2 && !X2 && !Y2) || (!W2 && X2);
      C = (W2 && X2) || (!W2 && !X2 && Y2);
      W2 = A; X2 = B; Y2 = C;

      // Minutes ones
      if((W2 | X2 | Y2) == 0)
      {
        A = !W3;
        B = (W3 && !X3 && !Z3) || (!W3 && X3);
        C = (!X3 && Y3) || (!W3 && Y3) || (W3 && X3 && !Y3);
        D = (!W3 && Z3) || (W3 && X3 && Y3);
        W3 = A; X3 = B; Y3 = C; Z3 = D;

        // Minutes tens
        if((W3 | X3 | Y3 | Z3) == 0)
        {
          A = !W4;
          B = (W4 && !X4 && !Y4) || (!W4 && X4);
          C = (W4 && X4) || (!W4 && !X4 && Y4);
          W4 = A; X4 = B; Y4 = C;

          // Hours ones
          if((W4 | X4 | Y4) == 0)
          {
            if(X6 == 0)
            {
              A = !W5;
              B = (W5 && !X5 && !Z5) || (!W5 && X5);
              C = (!X5 && Y5) || (!W5 && Y5) || (W5 && X5 && !Y5);
              D = (!W5 && Z5) || (W5 && X5 && Y5);
              W5 = A; X5 = B; Y5 = C; Z5 = D;
            }
            else
            {
              A = !W5;
              B = (W5 && !X5) || (!W5 && X5);
              W5 = A; X5 = B; Y5 = 0; Z5 = 0;
            }

            // Hours tens
            if((W5 | X5 | Y5 | Z5) == 0)
            {
              A = !W6 && !X6;
              B = W6 && !X6;
              W6 = A; X6 = B; Y6 = 0;
            }
          }
        }
      }
    }
  }
}
