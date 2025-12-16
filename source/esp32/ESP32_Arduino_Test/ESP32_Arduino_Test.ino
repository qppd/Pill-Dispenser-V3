// ESP32 to Arduino Uno Communication Test
// ESP32 GPIO25 (RX) <- Arduino Pin 3 (TX)
// ESP32 GPIO26 (TX) -> Arduino Pin 2 (RX)

#define PIN_UNO_RX 25   // ESP32 RX <- Arduino TX
#define PIN_UNO_TX 26   // ESP32 TX -> Arduino RX

HardwareSerial SerialUNO(1);  // UART1

void setup() {
  Serial.begin(115200); // USB debug
  delay(1000);
  
  Serial.println("ESP32-Arduino Communication Test");
  Serial.println("================================");
  
  // Initialize Arduino UART
  SerialUNO.begin(115200, SERIAL_8N1, PIN_UNO_RX, PIN_UNO_TX);
  delay(100);
  
  Serial.println("Waiting for Arduino...");
  delay(2000);
  
  // Test ping
  Serial.println("\n[TEST] Sending PING...");
  SerialUNO.println("PING");
}

void loop() {
  // Read responses from Arduino
  if (SerialUNO.available()) {
    String response = SerialUNO.readStringUntil('\n');
    Serial.println("[Arduino] " + response);
  }
  
  // Interactive commands via Serial Monitor
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command.length() > 0) {
      Serial.println("[ESP32] Sending: " + command);
      SerialUNO.println(command);
    }
  }
  
  // Auto-test commands every 10 seconds
  static unsigned long lastTest = 0;
  static int testStep = 0;
  
  if (millis() - lastTest > 10000) {
    lastTest = millis();
    
    switch(testStep) {
      case 0:
        Serial.println("\n[TEST] Requesting STATUS...");
        SerialUNO.println("STATUS");
        break;
      case 1:
        Serial.println("\n[TEST] Testing servo 0...");
        SerialUNO.println("SA0,90"); // Set servo 0 to 90 degrees
        break;
      case 2:
        Serial.println("\n[TEST] Dispensing from slot 2...");
        SerialUNO.println("DP2"); // Dispense from slot 2
        break;
      case 3:
        Serial.println("\n[TEST] Sending PING...");
        SerialUNO.println("PING");
        break;
    }
    
    testStep = (testStep + 1) % 4;
  }
}
