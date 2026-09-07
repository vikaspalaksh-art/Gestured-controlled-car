#include "BluetoothSerial.h"

// Check if Bluetooth is properly enabled in setup
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it.
#endif

BluetoothSerial SerialBT;

// Your HC-05 Receiver MAC Address
uint8_t address[6]  = {0x00, 0x24, 0x08, 0x00, 0x5F, 0xED};

// --- Sensor 1 Pins (Steering) ---
const int trigPin1 = 12;
const int echoPin1 = 13;

// --- Sensor 2 Pins (Throttle) ---
const int trigPin2 = 14;
const int echoPin2 = 27;

#define SOUND_SPEED 0.0343

void setup() {
  Serial.begin(115200);
  
  // Start ESP32 Bluetooth Master mode
  SerialBT.begin("ESP32_Remote", true); 
  Serial.println("Bluetooth Master Started. Connecting to HC-05...");
  
  // Connect directly using the MAC Address
  bool connected = SerialBT.connect(address);
  
  if(connected) {
    Serial.println("Successfully Connected to HC-05!");
  } else {
    Serial.println("Connection failed. Retrying in loop...");
    while(!SerialBT.connected()) {
      SerialBT.connect(address);
      delay(2000);
    }
  }

  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
}

void loop() {
  // If connection drops, attempt reconnection
  if (!SerialBT.connected()) {
    Serial.println("Connection lost. Reconnecting...");
    SerialBT.connect(address);
    delay(500);
    return;
  }

  // --- Read Sensor 1 (Steering) ---
  digitalWrite(trigPin1, LOW); delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  long d1 = pulseIn(echoPin1, HIGH);
  float dist1 = d1 * SOUND_SPEED / 2;
  
  delay(20); // Prevent ultrasonic crosstalk

  // --- Read Sensor 2 (Throttle) ---
  digitalWrite(trigPin2, LOW); delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  long d2 = pulseIn(echoPin2, HIGH);
  float dist2 = d2 * SOUND_SPEED / 2;

  char steering = 'S';  // Default: Straight
  char direction = 'X'; // Default: Stop
  int motorSpeed = 0;

  // --- Evaluate Steering (Sensor 1) ---
  if (dist1 >= 4.0 && dist1 <= 12.0) {
    steering = 'R';
  } else if (dist1 >= 14.0 && dist1 <= 24.0) {
    steering = 'L';
  }

  // --- Evaluate Throttle & Speed (Sensor 2) ---
  if (dist2 < 4.0 || dist2 > 24.0 || (dist2 >= 13.5 && dist2 <= 14.5)) {
    direction = 'X';
    motorSpeed = 0;
  } 
  else if (dist2 >= 4.0 && dist2 <= 13.0) {
    direction = 'F';
    motorSpeed = map(dist2, 4, 13, 255, 120);
  } 
  else if (dist2 >= 15.0 && dist2 <= 24.0) {
    direction = 'B';
    motorSpeed = map(dist2, 15, 24, 120, 255);
  }

  // --- Send Data via Bluetooth ---
  // Packet string syntax: "Direction,Speed,Steering\n"
  SerialBT.print(direction);
  SerialBT.print(",");
  SerialBT.print(motorSpeed);
  SerialBT.print(",");
  SerialBT.println(steering);

  // Debugging to ESP32 Serial Monitor
  Serial.print("Sent: "); Serial.print(direction);
  Serial.print(","); Serial.print(motorSpeed);
  Serial.print(","); Serial.println(steering);

  delay(100); 
}