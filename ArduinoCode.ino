#include <SoftwareSerial.h>

// Software Serial for HC-05 (RX=Pin 2, TX=Pin 3)
SoftwareSerial BTSerial(2, 3); 

// L298N Motor Driver Pins
const int ENA = 5;  // PWM pin Left
const int IN1 = 6;
const int IN2 = 7;
const int IN3 = 8;
const int IN4 = 9;
const int ENB = 10; // PWM pin Right

void setup() {
  Serial.begin(115200);
  BTSerial.begin(9600); // Standard HC-05 baud rate
  
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  stopCar();
}

void loop() {
  if (BTSerial.available()) {
    // Read the packet until newline character
    String data = BTSerial.readStringUntil('\n');
    
    // Locate commas for data parsing
    int firstComma = data.indexOf(',');
    int secondComma = data.indexOf(',', firstComma + 1);
    
    if (firstComma != -1 && secondComma != -1) {
      char direction = data.charAt(0);
      int targetSpeed = data.substring(firstComma + 1, secondComma).toInt();
      char steering = data.charAt(secondComma + 1);
      
      controlMotors(direction, targetSpeed, steering);
    }
  }
}

void controlMotors(char direction, int targetSpeed, char steering) {
  
  // --- PRIORITY 1: STEERING GESTURES (HIGHEST PRIORITY) ---
  
  if (steering == 'R') {
    // Left side spins FORWARD
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    // Right side spins BACKWARD
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    
    // If throttle hand is close, use its speed. If throttle hand is at rest/stop (0), 
    // fall back to a fixed speed of 220 so the car still spins powerfully.
    int spinSpeed = (targetSpeed > 0) ? targetSpeed : 220; 
    analogWrite(ENA, spinSpeed);
    analogWrite(ENB, spinSpeed);
  } 
  
  else if (steering == 'L') {
    // Left side spins BACKWARD
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    // Right side spins FORWARD
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    
    int spinSpeed = (targetSpeed > 0) ? targetSpeed : 220;
    analogWrite(ENA, spinSpeed);
    analogWrite(ENB, spinSpeed);
  } 
  
  // --- PRIORITY 2: THROTTLE GESTURES (Only runs if Steering is 'S' / Neutral) ---
  
  else {
    if (direction == 'F') {
      // Move Straight Forward
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      analogWrite(ENA, targetSpeed);
      analogWrite(ENB, targetSpeed);
    } 
    else if (direction == 'B') {
      // Move Straight Backward
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      analogWrite(ENA, targetSpeed);
      analogWrite(ENB, targetSpeed);
    } 
    else {
      // Both sensors are either at rest or out of bounds
      stopCar();
    }
  }
}

void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}