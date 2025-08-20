#include <MD_MAX72xx.h>
#include <SPI.h>

// Hardware configuration - using ICSTATION_HW for 10888AS modules
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 4
#define CLK_PIN   5
#define CS_PIN    17
#define DATA_PIN  16
#define BUTTON_PIN 33  // GPIO32 for start/stop/reset button

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Stopwatch variables
unsigned long startTime = 0;
unsigned long pausedTime = 0;
unsigned long totalPausedTime = 0;
enum StopwatchState { STOPPED, RUNNING, PAUSED, PAUSED_IDLE, RESET_IDLE };
StopwatchState stopwatchState = STOPPED;

// Button variables
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 5; // 5ms debounce delay for better responsiveness
byte buttonState = HIGH;
byte lastButtonState = HIGH;

// Define 8x8 patterns for digits 0-9
// Each byte represents a row, MSB is leftmost pixel
uint8_t digitPatterns[10][8] = {
  // Digit 0
  {
    0b00111100,  // Row 0: __XXXX__
    0b01100110,  // Row 1: _XX__XX_
    0b01100110,  // Row 2: _XX__XX_
    0b01100110,  // Row 3: _XX__XX_
    0b01100110,  // Row 4: _XX__XX_
    0b01100110,  // Row 5: _XX__XX_
    0b00111100,  // Row 6: __XXXX__
    0b00000000   // Row 7: ________
  },
  // Digit 1
  {
    0b00011000,  // Row 0: ___XX___
    0b00111000,  // Row 1: __XXX___
    0b00011000,  // Row 2: ___XX___
    0b00011000,  // Row 3: ___XX___
    0b00011000,  // Row 4: ___XX___
    0b00011000,  // Row 5: ___XX___
    0b01111100,  // Row 6: _XXXXX__
    0b00000000   // Row 7: ________
  },
  // Digit 2
  {
    0b00111100,  // Row 0: __XXXX__
    0b01100110,  // Row 1: _XX__XX_
    0b00000110,  // Row 2: _____XX_
    0b00001100,  // Row 3: ____XX__
    0b00110000,  // Row 4: __XX____
    0b01100000,  // Row 5: _XX_____
    0b01111100,  // Row 6: _XXXXX__
    0b00000000   // Row 7: ________
  },
  // Digit 3
  {
    0b00111100,  // Row 0: __XXXX__
    0b01100110,  // Row 1: _XX__XX_
    0b00000110,  // Row 2: _____XX_
    0b00011100,  // Row 3: ___XXX__
    0b00000110,  // Row 4: _____XX_
    0b01100110,  // Row 5: _XX__XX_
    0b00111100,  // Row 6: __XXXX__
    0b00000000   // Row 7: ________
  },
  // Digit 4
  {
    0b00001100,  // Row 0: ____XX__
    0b00011100,  // Row 1: ___XXX__
    0b00111100,  // Row 2: __XXXX__
    0b01101100,  // Row 3: _XX_XX__
    0b01111110,  // Row 4: _XXXXXX_
    0b00001100,  // Row 5: ____XX__
    0b00001100,  // Row 6: ____XX__
    0b00000000   // Row 7: ________
  },
  // Digit 5
  {
    0b01111110,  // Row 0: _XXXXXX_
    0b01100000,  // Row 1: _XX_____
    0b01100000,  // Row 2: _XX_____
    0b01111100,  // Row 3: _XXXXX__
    0b00000110,  // Row 4: _____XX_
    0b01100110,  // Row 5: _XX__XX_
    0b00111100,  // Row 6: __XXXX__
    0b00000000   // Row 7: ________
  },
  // Digit 6
  {
    0b00111100,  // Row 0: __XXXX__
    0b01100110,  // Row 1: _XX__XX_
    0b01100000,  // Row 2: _XX_____
    0b01111100,  // Row 3: _XXXXX__
    0b01100110,  // Row 4: _XX__XX_
    0b01100110,  // Row 5: _XX__XX_
    0b00111100,  // Row 6: __XXXX__
    0b00000000   // Row 7: ________
  },
  // Digit 7
  {
    0b01111110,  // Row 0: _XXXXXX_
    0b00000110,  // Row 1: _____XX_
    0b00001100,  // Row 2: ____XX__
    0b00011000,  // Row 3: ___XX___
    0b00110000,  // Row 4: __XX____
    0b00110000,  // Row 5: __XX____
    0b00110000,  // Row 6: __XX____
    0b00000000   // Row 7: ________
  },
  // Digit 8
  {
    0b00111100,  // Row 0: __XXXX__
    0b01100110,  // Row 1: _XX__XX_
    0b01100110,  // Row 2: _XX__XX_
    0b00111100,  // Row 3: __XXXX__
    0b01100110,  // Row 4: _XX__XX_
    0b01100110,  // Row 5: _XX__XX_
    0b00111100,  // Row 6: __XXXX__
    0b00000000   // Row 7: ________
  },
  // Digit 9
  {
    0b00111100,  // Row 0: __XXXX__
    0b01100110,  // Row 1: _XX__XX_
    0b01100110,  // Row 2: _XX__XX_
    0b00111110,  // Row 3: __XXXXX_
    0b00000110,  // Row 4: _____XX_
    0b01100110,  // Row 5: _XX__XX_
    0b00111100,  // Row 6: __XXXX__
    0b00000000   // Row 7: ________
  }
};

// Decimal point patterns - split between panels 2 and 1
uint8_t decimalPatternLeft[8] = {  // For panel 2 (right side)
  0b00000000,  // Row 0: ________
  0b00000000,  // Row 1: ________
  0b00000000,  // Row 2: ________
  0b00000000,  // Row 3: ________
  0b00000000,  // Row 4: ________
  0b00000000,  // Row 5: ________
  0b00000001,  // Row 6: _______X
  0b00000000   // Row 7: ________
};

uint8_t decimalPatternRight[8] = {  // For panel 1 (left side)
  0b00000000,  // Row 0: ________
  0b00000000,  // Row 1: ________
  0b00000000,  // Row 2: ________
  0b00000000,  // Row 3: ________
  0b00000000,  // Row 4: ________
  0b00000000,  // Row 5: ________
  0b00000000,  // Row 6: ________
  0b00000000   // Row 7: ________
};

// Function to display a digit on a specific panel
void displayDigit(int panel, int digit) {
  for (int row = 0; row < 8; row++) {
    mx.setRow(panel, row, digitPatterns[digit][row]);
  }
}

// Function to blank a specific panel
void blankPanel(int panel) {
  for (int row = 0; row < 8; row++) {
    mx.setRow(panel, row, 0b00000000);
  }
}

// Function to display digit with left decimal point on panel 2
void displayDigitWithLeftDecimal(int panel, int digit) {
  for (int row = 0; row < 8; row++) {
    uint8_t pattern = digitPatterns[digit][row] | decimalPatternLeft[row];
    mx.setRow(panel, row, pattern);
  }
}

// Function to display digit with right decimal point on panel 1
void displayDigitWithRightDecimal(int panel, int digit) {
  for (int row = 0; row < 8; row++) {
    uint8_t pattern = digitPatterns[digit][row] | decimalPatternRight[row];
    mx.setRow(panel, row, pattern);
  }
}

// Function to clear all displays
void clearDisplay() {
  mx.clear();
}

// Function to display "00.00"
void displayZeros() {
  displayDigit(0, 0);  // Tens of seconds
  displayDigitWithLeftDecimal(1, 0);  // Ones of seconds with left decimal
  displayDigitWithRightDecimal(2, 0);  // Tenths of seconds with right decimal
  displayDigit(3, 0);  // Hundredths of seconds
}

// Function to update the stopwatch display
void updateStopwatchDisplay() {
  if (stopwatchState != RUNNING) return;
  
  // Calculate elapsed time in centiseconds (hundredths of a second)
  unsigned long currentTime = millis();
  unsigned long elapsed = (currentTime - startTime - totalPausedTime);
  unsigned long centiseconds = elapsed / 10; // Convert to centiseconds
  
  // Extract seconds and centiseconds
  unsigned long totalSeconds = centiseconds / 100;
  unsigned long remainingCentiseconds = centiseconds % 100;
  
  // Limit display to 99.99 seconds
  if (totalSeconds >= 100) {
    totalSeconds = 99;
    remainingCentiseconds = 99;
  }
  
  // Extract individual digits
  int secondsTens = (totalSeconds / 10) % 10;
  int secondsOnes = totalSeconds % 10;
  int centisecondsTens = (remainingCentiseconds / 10) % 10;
  int centisecondsOnes = remainingCentiseconds % 10;
  
  // Display format: SS.DD (seconds.centiseconds)
  // Panel layout: [0][1].[2][3] where decimal is split between panels 1 and 2
  // Correct order: Panel 0=tens of seconds, Panel 1=ones of seconds with left decimal,
  //                Panel 2=tenths with right decimal, Panel 3=hundredths
  if (secondsTens == 0) {
    blankPanel(0);
   } else {
    displayDigit(0, secondsTens);                    // Tens of seconds
   }
  displayDigitWithLeftDecimal(1, secondsOnes);     // Ones of seconds with left decimal point
  displayDigitWithRightDecimal(2, centisecondsTens); // Tenths of seconds with right decimal point
  displayDigit(3, centisecondsOnes);               // Hundredths of seconds
}

// Function to handle button events
byte checkButton() {
  byte reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        return 1; // Pressed
      } else {
        return 2; // Released
      }
    }
  }

  lastButtonState = reading;
  return 0; // No event
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("MAX7219 Stopwatch with Button Control");
  Serial.println("=====================================");
  Serial.println("Format: SS.DD (Seconds.Centiseconds)");
  Serial.println("Panel layout: [Tens][Ones].[Tenths][Hundredths]");
  Serial.println("              [0]   [1]    [2]     [3]");
  Serial.println();
  Serial.println("Button Control (GPIO32):");
  Serial.println("1st press: START stopwatch");
  Serial.println("2nd press: STOP/PAUSE stopwatch");
  Serial.println("3rd press: RESET and clear display");
  
  // Initialize button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize the display
  if (!mx.begin()) {
    Serial.println("ERROR: MAX7219 initialization failed!");
    while(1) {
      delay(500);
    }
  }
  
  Serial.println("SUCCESS: MAX7219 initialized.");
  
  // Configure display settings
  mx.control(MD_MAX72XX::INTENSITY, 15);   // Maximum brightness
  mx.control(MD_MAX72XX::SHUTDOWN, false); // Turn on display
  clearDisplay();                          // Clear all panels
  
  delay(1000);
  
  // Keep display clear initially
  clearDisplay();
  
  Serial.println("Stopwatch ready! Press button on GPIO32 to start.");
}

void loop() {
  byte event = checkButton();

  switch (stopwatchState) {
    case STOPPED:
      if (event == 2) { // Start on release
        startTime = millis();
        totalPausedTime = 0;
        stopwatchState = RUNNING;
        Serial.println("Stopwatch STARTED");
      }
      break;

    case RUNNING:
      if (event == 1) { // Stop on press
        stopwatchState = PAUSED_IDLE; // Go to idle state to wait for release
        Serial.println("Stopwatch PAUSED");
      }
      break;

    case PAUSED_IDLE:
        // Wait for button release to avoid multiple triggers
        if (event == 2) {
            stopwatchState = PAUSED;
        }
        break;

    case PAUSED:
      if (event == 1) { // Reset on the next press
        clearDisplay();
        stopwatchState = RESET_IDLE; // Wait for release
        Serial.println("Stopwatch RESET");
      }
      break;
    
    case RESET_IDLE:
      if (event == 2) { // On release, go to STOPPED state
        stopwatchState = STOPPED; // Ready for a clean start
      }
      break;
  }

  // Update display if running
  if (stopwatchState == RUNNING) {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 10) {  // Update every 10ms
      lastUpdate = millis();
      updateStopwatchDisplay();
    }
  }
  
}
