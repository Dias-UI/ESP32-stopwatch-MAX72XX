#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Device MAC addresses
uint8_t topDeviceMAC[] = {0xFC, 0xB4, 0x67, 0x4E, 0x7E, 0x38}; // Top device MAC
uint8_t bottomDeviceMAC[] = {0xFC, 0xB4, 0x67, 0x4E, 0x7D, 0x58}; // This device MAC

// Pin definitions
#define BUTTON_PAD_PIN 33    // Button pad (two metal pads)
#define RESET_BUTTON_PIN 25  // Reset button
#define LED_RED_PIN 19       // RGB LED Red
#define LED_GREEN_PIN 23     // RGB LED Green  
#define LED_BLUE_PIN 18      // RGB LED Blue

// Button variables
unsigned long lastDebounceTime = 0;
unsigned long resetLastDebounceTime = 0;
unsigned long debounceDelay = 5; // 50ms debounce delay for better responsiveness
byte buttonState = HIGH;
byte lastButtonState = HIGH;
byte resetButtonState = HIGH;
byte lastResetButtonState = HIGH;

// LED states
enum LEDState { LED_OFF, LED_WHITE, LED_ORANGE };
LEDState currentLEDState = LED_OFF;

// Communication message types
typedef struct {
  int messageType; // 1 = start signal, 2 = reset signal
  unsigned long timestamp;
} Message;

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

// Function to set LED to white
void setLEDWhite() {
  setLEDColor(255, 255, 255);
  currentLEDState = LED_WHITE;
}

// Function to set LED to orange
void setLEDOrange() {
  setLEDColor(255, 165, 0);
  currentLEDState = LED_ORANGE;
}

// Function to handle button pad events
byte checkButtonPad() {
  byte reading = digitalRead(BUTTON_PAD_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        return 1; // Pressed (climber stepped on pad)
      } else {
        return 2; // Released (climber released pad to start climbing)
      }
    }
  }

  lastButtonState = reading;
  return 0; // No event
}

// Function to handle reset button events
byte checkResetButton() {
  byte reading = digitalRead(RESET_BUTTON_PIN);

  if (reading != lastResetButtonState) {
    resetLastDebounceTime = millis();
  }

  if ((millis() - resetLastDebounceTime) > debounceDelay) {
    if (reading != resetButtonState) {
      resetButtonState = reading;
      if (resetButtonState == LOW) {
        return 1; // Reset button pressed
      }
    }
  }

  lastResetButtonState = reading;
  return 0; // No event
}

// Callback function for ESP-NOW send status
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Delivery Success");
  } else {
    Serial.println("Delivery Fail");
  }
}

// Function to send start signal to top device
void sendStartSignal() {
  Message msg;
  msg.messageType = 1; // Start signal
  msg.timestamp = millis();
  
  esp_err_t result = esp_now_send(topDeviceMAC, (uint8_t *) &msg, sizeof(msg));
  
  if (result == ESP_OK) {
    Serial.println("Start signal sent successfully");
  } else {
    Serial.println("Error sending start signal");
  }
}

// Function to send reset signal to top device
void sendResetSignal() {
  Message msg;
  msg.messageType = 2; // Reset signal
  msg.timestamp = millis();
  
  esp_err_t result = esp_now_send(topDeviceMAC, (uint8_t *) &msg, sizeof(msg));
  
  if (result == ESP_OK) {
    Serial.println("Reset signal sent successfully");
  } else {
    Serial.println("Error sending reset signal");
  }
}

// Initialize ESP-NOW
void initESPNow() {
  Serial.println("Initializing ESP-NOW...");
  
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
  
  // Register send callback
  esp_now_register_send_cb(OnDataSent);
  Serial.println("Send callback registered");
  
  // Add peer (top device)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, topDeviceMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;
  
  Serial.print("Adding peer with MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", topDeviceMAC[i]);
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
  Serial.println("Bottom unit paired and ready!");
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("Speed Climbing Stopwatch - Start Timer (Bottom Unit)");
  Serial.println("====================================================");
  Serial.print("Device MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", bottomDeviceMAC[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  // Initialize pins
  pinMode(BUTTON_PAD_PIN, INPUT_PULLUP);
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  
  // Turn off LED initially
  turnLEDOff();
  
  // Initialize ESP-NOW
  initESPNow();
  
  Serial.println("Bottom unit ready!");
  Serial.println("- Step on button pad to turn LED white");
  Serial.println("- Release button pad to start timer (LED turns orange)");
  Serial.println("- Press reset button to clear top display");
}

void loop() {
  // Check button pad events
  byte padEvent = checkButtonPad();
  
  if (padEvent == 1) { // Climber stepped on pad
    Serial.println("Climber stepped on pad - LED WHITE");
    setLEDWhite();
  } else if (padEvent == 2) { // Climber released pad to start climbing
    Serial.println("Climber released pad - Starting timer, LED ORANGE");
    setLEDOrange();
    sendStartSignal();
  }
  
  // Check reset button
  byte resetEvent = checkResetButton();
  
  if (resetEvent == 1) { // Reset button pressed
    Serial.println("Reset button pressed - Clearing display and turning off LED");
    turnLEDOff();
    sendResetSignal();
  }
  
  delay(10); // Small delay for stability
}
