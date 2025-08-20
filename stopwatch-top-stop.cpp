#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Device MAC addresses
uint8_t topDeviceMAC[] = {0xFC, 0xB4, 0x67, 0x4E, 0x7E, 0x38}; // This device MAC
uint8_t bottomDeviceMAC[] = {0xFC, 0xB4, 0x67, 0x4E, 0x7D, 0x58}; // Bottom device MAC

// Hardware configuration - using ICSTATION_HW for 10888AS modules
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 4
#define CLK_PIN   5
#define CS_PIN    17
#define DATA_PIN  16
#define BUTTON_PIN 33        // Button pad at top of wall
#define LED_RED_PIN 19       // RGB LED Red
#define LED_GREEN_PIN 23     // RGB LED Green  
#define LED_BLUE_PIN 18      // RGB LED Blue

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Stopwatch variables
unsigned long startTime = 0;
unsigned long finalTime = 0;
enum StopwatchState { WAITING, RUNNING, STOPPED, DISPLAYING };
StopwatchState stopwatchState = WAITING;

// Button variables
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 5; // 5ms debounce delay for better responsiveness
byte buttonState = HIGH;
byte lastButtonState = HIGH;

// LED states
enum LEDState { LED_OFF, LED_GREEN };
LEDState currentLEDState = LED_OFF;

// Communication message types
typedef struct {
  int messageType; // 1 = start signal, 2 = reset signal, 3 = ping, 4 = pong
  unsigned long timestamp;
} Message;

// Connection status variables
bool isConnectedToBottom = false;
unsigned long lastPingTime = 0;
unsigned long lastPongTime = 0;
const unsigned long PING_INTERVAL = 1000; // Send ping every 1 second
const unsigned long CONNECTION_TIMEOUT = 3000; // Consider disconnected after 3 seconds

// Define 8x8 patterns for digits 0-9
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

// Function to set RGB LED color
void setLEDColor(int red, int green, int blue) {
  analogWrite(LED_RED_PIN, red);
  analogWrite(LED_GREEN_PIN, green);
  analogWrite(LED_BLUE_PIN, blue);
}

// Function to turn LED off
void turnLEDOff() {
  setLEDColor(0, 0, 0);
  currentLEDState = LED_OFF;
}

// Function to set LED to green
void setLEDGreen() {
  setLEDColor(0, 255, 0);
  currentLEDState = LED_GREEN;
}

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

// Function to update the stopwatch display
void updateStopwatchDisplay() {
  if (stopwatchState != RUNNING) return;
  
  // Calculate elapsed time in centiseconds (hundredths of a second)
  unsigned long currentTime = millis();
  unsigned long elapsed = (currentTime - startTime);
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
  if (secondsTens == 0) {
    blankPanel(0);
   } else {
    displayDigit(0, secondsTens);                    // Tens of seconds
   }
  displayDigitWithLeftDecimal(1, secondsOnes);     // Ones of seconds with left decimal point
  displayDigitWithRightDecimal(2, centisecondsTens); // Tenths of seconds with right decimal point
  displayDigit(3, centisecondsOnes);               // Hundredths of seconds
}

// Function to display final time (when stopped)
void displayFinalTime() {
  unsigned long centiseconds = finalTime / 10; // Convert to centiseconds
  
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
      }
    }
  }

  lastButtonState = reading;
  return 0; // No event
}

// Function to display "PAIR" on the matrix
void displayPairMessage() {
  // Clear display first
  clearDisplay();
  
  // Simple pattern for "PAIR" - using basic shapes
  // P on panel 0
  uint8_t pPattern[8] = {
    0b01111100,  // _XXXXX__
    0b01100110,  // _XX__XX_
    0b01100110,  // _XX__XX_
    0b01111100,  // _XXXXX__
    0b01100000,  // _XX_____
    0b01100000,  // _XX_____
    0b01100000,  // _XX_____
    0b00000000   // ________
  };
  
  // A on panel 1
  uint8_t aPattern[8] = {
    0b00111100,  // __XXXX__
    0b01100110,  // _XX__XX_
    0b01100110,  // _XX__XX_
    0b01111110,  // _XXXXXX_
    0b01100110,  // _XX__XX_
    0b01100110,  // _XX__XX_
    0b01100110,  // _XX__XX_
    0b00000000   // ________
  };
  
  // I on panel 2
  uint8_t iPattern[8] = {
    0b01111110,  // _XXXXXX_
    0b00011000,  // ___XX___
    0b00011000,  // ___XX___
    0b00011000,  // ___XX___
    0b00011000,  // ___XX___
    0b00011000,  // ___XX___
    0b01111110,  // _XXXXXX_
    0b00000000   // ________
  };
  
  // R on panel 3
  uint8_t rPattern[8] = {
    0b01111100,  // _XXXXX__
    0b01100110,  // _XX__XX_
    0b01100110,  // _XX__XX_
    0b01111100,  // _XXXXX__
    0b01101100,  // _XX_XX__
    0b01100110,  // _XX__XX_
    0b01100110,  // _XX__XX_
    0b00000000   // ________
  };
  
  // Display each letter
  for (int row = 0; row < 8; row++) {
    mx.setRow(0, row, pPattern[row]);
    mx.setRow(1, row, aPattern[row]);
    mx.setRow(2, row, iPattern[row]);
    mx.setRow(3, row, rPattern[row]);
  }
}

// Function to display "OK" on the matrix
void displayOKMessage() {
  // Clear display first
  clearDisplay();
  
  // O on panel 1
  uint8_t oPattern[8] = {
    0b00111100,  // __XXXX__
    0b01100110,  // _XX__XX_
    0b01100110,  // _XX__XX_
    0b01100110,  // _XX__XX_
    0b01100110,  // _XX__XX_
    0b01100110,  // _XX__XX_
    0b00111100,  // __XXXX__
    0b00000000   // ________
  };
  
  // K on panel 2
  uint8_t kPattern[8] = {
    0b01100110,  // _XX__XX_
    0b01101100,  // _XX_XX__
    0b01111000,  // _XXXX___
    0b01110000,  // _XXX____
    0b01111000,  // _XXXX___
    0b01101100,  // _XX_XX__
    0b01100110,  // _XX__XX_
    0b00000000   // ________
  };
  
  // Display each letter (centered)
  for (int row = 0; row < 8; row++) {
    mx.setRow(1, row, oPattern[row]);
    mx.setRow(2, row, kPattern[row]);
  }
}

// Callback function for receiving ESP-NOW data
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  Message msg;
  memcpy(&msg, incomingData, sizeof(msg));
  
  Serial.print("Message received from: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  // Update connection status when we receive any message from bottom device
  lastPongTime = millis();
  if (!isConnectedToBottom) {
    isConnectedToBottom = true;
    Serial.println("Bottom unit connected!");
    displayOKMessage();
    delay(2000);
    clearDisplay();
  }
  
  if (msg.messageType == 1) { // Start signal
    Serial.println("Start signal received - Beginning stopwatch");
    startTime = millis();
    stopwatchState = RUNNING;
  } else if (msg.messageType == 2) { // Reset signal
    Serial.println("Reset signal received - Clearing display and turning off LED");
    clearDisplay();
    turnLEDOff();
    stopwatchState = WAITING;
  } else if (msg.messageType == 3) { // Ping received
    Serial.println("Ping received - Sending pong");
    // Send pong response
    Message pongMsg;
    pongMsg.messageType = 4;
    pongMsg.timestamp = millis();
    esp_now_send(bottomDeviceMAC, (uint8_t *) &pongMsg, sizeof(pongMsg));
  } else if (msg.messageType == 4) { // Pong received
    Serial.println("Pong received - Connection confirmed");
    // Connection already handled above
  }
}

// Function to send ping to bottom unit
void sendPing() {
  Message pingMsg;
  pingMsg.messageType = 3;
  pingMsg.timestamp = millis();
  esp_now_send(bottomDeviceMAC, (uint8_t *) &pingMsg, sizeof(pingMsg));
  lastPingTime = millis();
}

// Initialize ESP-NOW
void initESPNow() {
  // Show pairing message
  displayPairMessage();
  Serial.println("Initializing ESP-NOW...");
  Serial.println("Waiting for bottom unit to connect...");
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.print("WiFi MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: ESP-NOW initialization failed!");
    return;
  }
  Serial.println("ESP-NOW initialized successfully");
  
  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Receive callback registered");
  
  // Add peer (bottom device)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, bottomDeviceMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;
  
  Serial.print("Adding peer with MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", bottomDeviceMAC[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  // Add peer
  esp_err_t addStatus = esp_now_add_peer(&peerInfo);
  if (addStatus != ESP_OK) {
    Serial.print("Failed to add peer. Error: ");
    Serial.println(addStatus);
    return;
  }
  
  Serial.println("SUCCESS: Peer added successfully!");
  Serial.println("Sending ping to establish connection...");
  
  // Start connection process by sending initial ping
  sendPing();
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("Speed Climbing Stopwatch - Stop Timer (Top Unit)");
  Serial.println("=================================================");
  Serial.print("Device MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", topDeviceMAC[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  // Initialize pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  
  // Turn off LED initially
  turnLEDOff();
  
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
  
  // Initialize ESP-NOW
  initESPNow();
  
  Serial.println("Top unit initialized!");
  Serial.println("- Waiting for connection to bottom unit");
  Serial.println("- Will show 'PAIR' until connected, then 'OK'");
  Serial.println("- Press button to stop timer when running (LED turns green)");
}

void loop() {
  // Check connection status
  unsigned long currentTime = millis();
  
  // Send periodic pings if not connected or to maintain connection
  if (currentTime - lastPingTime >= PING_INTERVAL) {
    sendPing();
  }
  
  // Check if connection timed out
  if (isConnectedToBottom && (currentTime - lastPongTime > CONNECTION_TIMEOUT)) {
    isConnectedToBottom = false;
    Serial.println("Connection to bottom unit lost!");
    if (stopwatchState == WAITING) {
      displayPairMessage();
    }
  }
  
  // Check button events (stop button)
  byte buttonEvent = checkButton();
  
  if (buttonEvent == 1 && stopwatchState == RUNNING) { // Stop button pressed while running
    Serial.println("Stop button pressed - Timer stopped, LED GREEN");
    finalTime = millis() - startTime;
    stopwatchState = DISPLAYING;
    setLEDGreen();
    displayFinalTime();
    
    // Print final time to serial
    float finalTimeSeconds = finalTime / 1000.0;
    Serial.print("Final time: ");
    Serial.print(finalTimeSeconds, 2);
    Serial.println(" seconds");
  }
  
  // Update display if running
  if (stopwatchState == RUNNING) {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 10) {  // Update every 10ms
      lastUpdate = millis();
      updateStopwatchDisplay();
    }
  }
  
  delay(10); // Small delay for stability
}
