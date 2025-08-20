#include <MD_MAX72xx.h>
#include <SPI.h>

// Hardware configuration - using ICSTATION_HW for 10888AS modules
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 4
#define CLK_PIN   5
#define CS_PIN    17
#define DATA_PIN  16

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
  Serial.begin(115200);
  delay(2000); // Give serial time to initialize
  Serial.println("MAX7219 Comprehensive Diagnostic Test");
  Serial.println("=====================================");
  Serial.println("ESP32 Pin Configuration:");
  Serial.println("  DIN (Data In)  -> ESP32 GPIO5");
  Serial.println("  CLK (Clock)    -> ESP32 GPIO16");
  Serial.println("  CS  (Chip Sel) -> ESP32 GPIO17");
  Serial.println();
  Serial.println("Voltage Requirements:");
  Serial.println("  Most MAX7219 modules work with 3.3V or 5V");
  Serial.println("  Check your module specifications");
  Serial.println("  Ensure adequate current supply (at least 1A for 4 modules)");
  Serial.println();
  
  Serial.println("Test 1: Basic Initialization Check");
  Serial.println("Attempting to initialize MAX7219 modules...");
  
  // Try to initialize display
  if (!mx.begin()) {
    Serial.println("ERROR: MAX72XX initialization failed!");
    Serial.println("Possible causes:");
    Serial.println("1. Wiring issues (check DIN, CLK, CS connections)");
    Serial.println("2. Power supply problems");
    Serial.println("3. Faulty modules");
    Serial.println("4. Incorrect pin assignments");
    while(1) {
      delay(500);
    }
  }
  
  Serial.println("SUCCESS: MAX72XX initialized.");
  Serial.println();
  
  // Configure display settings
  Serial.println("Test 2: Display Configuration");
  mx.control(MD_MAX72XX::INTENSITY, 15);   // Maximum brightness
  mx.control(MD_MAX72XX::SHUTDOWN, false); // Turn on display
  mx.clear();                              // Clear all panels
  
  delay(1000);
  
  Serial.println("Test 3: Individual Pixel Test");
  Serial.println("Testing one pixel per module...");
  
  // Test one pixel on each module
  for (int module = 0; module < MAX_DEVICES; module++) {
    Serial.print("Testing pixel on module ");
    Serial.print(module);
    Serial.println("...");
    
    // Clear all first
    mx.clear();
    delay(1000);
    
    // Light up one pixel (row 0, column 0) on this module
    mx.setPoint(module, 0, true);
    
    delay(2000);
  }
  
  mx.clear();
  delay(1000);
  
  Serial.println("Test 4: Sequential Pixel Test");
  Serial.println("Testing sequential pixels across modules...");
  
  // Test sequential pixels
  for (int i = 0; i < 8; i++) {
    mx.clear();
    delay(500);
    
    // Light up pixel (i, 0) on each module
    for (int module = 0; module < MAX_DEVICES; module++) {
      mx.setPoint(module, i, true);
    }
    
    Serial.print("Testing pixels at row ");
    Serial.print(i);
    Serial.println(" on all modules...");
    
    delay(1500);
  }
  
  mx.clear();
  delay(1000);
  
  Serial.println("Test 5: Individual Module Full Test");
  Serial.println("Testing each module at full brightness...");
  
  for (int module = 0; module < MAX_DEVICES; module++) {
    Serial.print("Testing full illumination on module ");
    Serial.print(module);
    Serial.println("...");
    
    // Clear all first
    mx.clear();
    delay(500);
    
    // Light up this module only at full brightness
    for (int row = 0; row < 8; row++) {
      mx.setRow(module, row, 0xFF);
    }
    
    delay(2000);
  }
  
  mx.clear();
  delay(1000);
  
  Serial.println("Test 6: Row-by-Row Test");
  Serial.println("Testing each row separately...");
  
  for (int row = 0; row < 8; row++) {
    Serial.print("Testing row ");
    Serial.print(row);
    Serial.println("...");
    
    // Clear all first
    mx.clear();
    delay(500);
    
    // Light up this row on all modules
    for (int module = 0; module < MAX_DEVICES; module++) {
      mx.setRow(module, row, 0xFF);
    }
    
    delay(1500);
  }
  
  mx.clear();
  delay(1000);
  
  Serial.println("Test 7: Column-by-Column Test");
  Serial.println("Testing each column separately...");
  
  for (int col = 0; col < 8; col++) {
    Serial.print("Testing column ");
    Serial.print(col);
    Serial.println("...");
    
    // Clear all first
    mx.clear();
    delay(500);
    
    // Light up this column on all modules
    for (int module = 0; module < MAX_DEVICES; module++) {
      mx.setColumn(module, col, 0xFF);
    }
    
    delay(1500);
  }
  
  mx.clear();
  delay(1000);
  
  Serial.println("Test 8: Pattern Test");
  Serial.println("Testing with specific patterns...");
  
  // Test pattern - checkerboard
  Serial.println("Checkerboard pattern:");
  for (int module = 0; module < MAX_DEVICES; module++) {
    for (int row = 0; row < 8; row++) {
      uint8_t pattern = (row % 2 == 0) ? 0x55 : 0xAA; // Alternating pattern
      mx.setRow(module, row, pattern);
    }
  }
  
  delay(3000);
  
  // Test pattern - diagonal
  Serial.println("Diagonal pattern:");
  mx.clear();
  delay(500);
  
  for (int module = 0; module < MAX_DEVICES; module++) {
    for (int row = 0; row < 8; row++) {
      mx.setRow(module, row, (1 << row)); // Diagonal line
    }
  }
  
  delay(3000);
  
  mx.clear();
  Serial.println();
  Serial.println("DIAGNOSTIC COMPLETE");
  Serial.println("==================");
  Serial.println("Troubleshooting Guide:");
  Serial.println();
  Serial.println("If no pixels light up:");
  Serial.println("  1. Check power supply connections");
  Serial.println("  2. Verify voltage requirements for your modules");
  Serial.println("  3. Check all wiring connections (DIN, CLK, CS)");
  Serial.println("  4. Ensure proper grounding");
  Serial.println();
  Serial.println("If only some pixels light up:");
  Serial.println("  1. Check for faulty modules");
  Serial.println("  2. Verify module chaining connections");
  Serial.println("  3. Look for loose connections");
  Serial.println();
  Serial.println("If flickering occurs:");
  Serial.println("  1. Check power supply adequacy");
  Serial.println("  2. Add decoupling capacitors near modules");
  Serial.println("  3. Verify stable connections");
  Serial.println();
  Serial.println("Wiring Verification:");
  Serial.println("  DIN (Data In)  -> ESP32 GPIO5");
  Serial.println("  CLK (Clock)    -> ESP32 GPIO16");
  Serial.println("  CS  (Chip Sel) -> ESP32 GPIO17");
  Serial.println("  VCC            -> 3.3V or 5V (check module specs)");
  Serial.println("  GND            -> GND");
  Serial.println();
  Serial.println("Module Chaining:");
  Serial.println("  OUT of first module -> IN of second module");
  Serial.println("  OUT of second module -> IN of third module");
  Serial.println("  OUT of third module -> IN of fourth module");
  Serial.println("  Only the first module connects to ESP32 pins");
}

void loop() {
  Serial.println("Continuous test: Sequential individual pixels");
  
  // Test each pixel individually across all modules
  for (int module = 0; module < MAX_DEVICES; module++) {
    for (int row = 0; row < 8; row++) {
      // Clear all
      mx.clear();
      
      // Light up just one pixel in each row
      mx.setPoint(module, row, true);
      
      Serial.print("Module ");
      Serial.print(module);
      Serial.print(", Row ");
      Serial.println(row);
      
      delay(200);
    }
  }
}
