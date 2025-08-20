#include <Arduino.h>

// Define the pins for the RGB LED and the button
#define LED_RED_PIN 19
#define LED_GREEN_PIN 23
#define LED_BLUE_PIN 18
#define BUTTON_PIN 32

// PWM Configuration for color mixing
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;
const int RED_CHANNEL = 0;
const int GREEN_CHANNEL = 1;
const int BLUE_CHANNEL = 2;

// Variable to store the last state of the button
int lastButtonState = HIGH;

// Variables for non-blocking delay
unsigned long whiteStartTime = 0;
const unsigned long WHITE_DURATION = 1000;
bool showingWhite = false;

// Helper function to set the color of the RGB LED
void setColor(int red, int green, int blue) {
  // Values are 0-255, corresponds to PWM_RESOLUTION of 8
  ledcWrite(RED_CHANNEL, red);
  ledcWrite(GREEN_CHANNEL, green);
  ledcWrite(BLUE_CHANNEL, blue);
}

void setup() {
  // Start serial communication for logging
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect
  Serial.println("\n--- ESP32 RGB LED Initializing ---");

  // Set up the LEDC PWM channels
  ledcSetup(RED_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(GREEN_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(BLUE_CHANNEL, PWM_FREQ, PWM_RESOLUTION);

  // Attach the pins to the PWM channels
  ledcAttachPin(LED_RED_PIN, RED_CHANNEL);
  ledcAttachPin(LED_GREEN_PIN, GREEN_CHANNEL);
  ledcAttachPin(LED_BLUE_PIN, BLUE_CHANNEL);

  // Configure the button pin as an input with an internal pull-up resistor.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // --- Startup Light Sequence ---
  // Red
  Serial.println("Startup: LED Red");
  setColor(255, 0, 0);
  delay(500);
  // Green
  Serial.println("Startup: LED Green");
  setColor(0, 255, 0);
  delay(500);
  // Blue
  Serial.println("Startup: LED Blue");
  setColor(0, 0, 255);
  delay(500);
  // --- End of Sequence ---

  // Turn the LED off initially
  setColor(0, 0, 0);
  
  Serial.println("Setup complete - waiting for button presses");
}

void loop() {
  // Read the current state of the button
  int currentButtonState = digitalRead(BUTTON_PIN);
  
  // Debug output - this will help us see what's happening
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 500) {  // Print every 500ms
    Serial.print("Button state: ");
    Serial.println(currentButtonState);
    lastDebug = millis();
  }

  // Handle white LED timeout (non-blocking)
  if (showingWhite && (millis() - whiteStartTime >= WHITE_DURATION)) {
    showingWhite = false;
    Serial.println("Turning LED off. Awaiting next press.");
    setColor(0, 0, 0);
  }

  // Check if the button was just pressed (a one-time event)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    Serial.println("Button pressed down. Setting LED to Orange.");
    // Cancel white if still showing
    showingWhite = false;
  }

  // If the button is held down, keep the LED orange (only if not showing white)
  if (currentButtonState == LOW && !showingWhite) {
    // Set LED to orange
    setColor(255, 165, 0);
  }
  // Check if the button was just released
  else if (lastButtonState == LOW && currentButtonState == HIGH) {
    Serial.println("Button released.");
    // Set LED to white
    Serial.println("Setting LED to White for 1 second.");
    setColor(255, 255, 255);
    showingWhite = true;
    whiteStartTime = millis();
  }

  // Save the current state to detect changes in the next loop
  lastButtonState = currentButtonState;

  // A small delay to help with button debouncing
  delay(30);
}
