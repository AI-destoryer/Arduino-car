//---Anuhas Final Master Code---
#include <Servo.h>
#include <NewPing.h>
#define DECODE_NEC 
#include <IRremote.h>

// --- PIN CONFIGURATION ---
const int IN1 = 5; const int IN2 = 4; // Left Motor
const int IN3 = 7; const int IN4 = 6; // Right Motor
const int TRIG_PIN = A1; 
const int ECHO_PIN = A2;
const int SERVO_PIN = 10;
const int IR_RECEIVE_PIN = 2; 

// --- PARAMETERS ---
const int DIST_THRESHOLD = 40; 
const int MAX_DIST = 200;
const int IR_TIMEOUT = 250; 

// --- MODES ---
enum Mode { MANUAL, AUTO, FOLLOW_MODE };
Mode currentMode = AUTO; 

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DIST);
Servo myServo;
unsigned long lastIRTime = 0; 

void setup() {
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); 
  myServo.attach(SERVO_PIN);
  myServo.write(90); 
  
  Serial.begin(9600);
  Serial.println("Robot Ready. Modes: Auto, Manual, Follow");
}

void loop() {
  // 1. IR REMOTE HANDLING
  if (IrReceiver.decode()) {
    handleRemote(IrReceiver.decodedIRData.command);
    lastIRTime = millis();
    IrReceiver.resume(); 
  }

  // 2. READ DISTANCE
  int distance = getDistance();

  // 3. MODE LOGIC
  if (currentMode == AUTO) {
    autoControl(distance);
  } 
  else if (currentMode == FOLLOW_MODE) {
    myServo.write(90); // Keep sensor centered to follow
    smartFollow(distance);
  } 
  else { // MANUAL MODE
    if (millis() - lastIRTime > IR_TIMEOUT) {
      stopMotors();
    }
  }
}

void handleRemote(uint16_t command) {
  switch (command) {
    case 0x18: currentMode = MANUAL; moveForward();  break; 
    case 0x52: currentMode = MANUAL; moveBackward(); break; 
    case 0x08: currentMode = MANUAL; turnLeft();     break; 
    case 0x5A: currentMode = MANUAL; turnRight();    break; 
    case 0x1C: currentMode = MANUAL; stopMotors();   break; 
    case 0x42: currentMode = AUTO;   stopMotors();   break; 
    case 0x44: currentMode = FOLLOW_MODE; stopMotors(); break; // New Follow Button
  }
}

// --- FOLLOW LOGIC ---
void smartFollow(int distance) {
  // If person is at a safe distance (15-30cm), STOP
  if (distance >= 15 && distance <= 30) {
    stopMotors();
  }
  // If person moves away (31-60cm), FOLLOW
  else if (distance > 30 && distance <= 60) {
    moveForward();
  }
  // If person gets too close (under 15cm), BACK UP
  else if (distance > 1 && distance < 15) {
    moveBackward();
  }
  // If nobody is seen (0 or >60cm), STOP
  else {
    stopMotors();
  }
}

// --- AUTO AVOIDANCE ---
void autoControl(int distance) {
  if (distance > DIST_THRESHOLD || distance == 0) {
    moveForward();
  } 
  else {
    stopMotors(); 
    delay(300);
    moveBackward(); 
    delay(500);
    stopMotors();
    
    myServo.write(30);  delay(500);
    int distR = getDistance();
    myServo.write(150); delay(600);
    int distL = getDistance();
    myServo.write(90);  delay(300);
    
    if (distL > distR) turnLeft(); else turnRight();
    delay(600);
    stopMotors();
  }
}

int getDistance() {
  delay(50); 
  int cm = sonar.ping_cm();
  return cm;
}

// --- MOVEMENT ---
void moveForward()  { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  }
void moveBackward() { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); }
void turnLeft()     { digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  }
void turnRight()    { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); }
void stopMotors()   { digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);  }
