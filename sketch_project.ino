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
const int DIST_THRESHOLD = 25; 
const int MAX_DIST = 200;
const int IR_TIMEOUT = 250; 

// --- GLOBAL OBJECTS ---
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DIST);
Servo myServo;
bool autoMode = true; 
unsigned long lastIRTime = 0; 

void setup() {
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); 
  myServo.attach(SERVO_PIN);
  myServo.write(90); 
  
  Serial.begin(9600);
  Serial.println("Robot Ready.");
}

void loop() {
  // 1. IR REMOTE HANDLING
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      lastIRTime = millis(); 
    } else {
      handleRemote(IrReceiver.decodedIRData.command);
      lastIRTime = millis();
    }
    IrReceiver.resume(); 
  }

  // 2. DRIVE LOGIC
  if (autoMode) {
    autoControl();
  } else {
    // Manual Safety: Stops motor if button is released
    if (millis() - lastIRTime > IR_TIMEOUT) {
      stopMotors();
    }
  }
}

void handleRemote(uint16_t command) {
  switch (command) {
    case 0x18: autoMode = false; moveForward();  break; 
    case 0x52: autoMode = false; moveBackward(); break; 
    case 0x08: autoMode = false; turnLeft();     break; 
    case 0x5A: autoMode = false; turnRight();    break; 
    case 0x1C: autoMode = false; stopMotors();   break; 
    case 0x45: autoMode = false; stopMotors();   break; // OFF Button
    case 0x42: autoMode = true;  stopMotors();   break; // AUTO Button
  }
}

void autoControl() {
  int distance = getDistance();
  if (distance > DIST_THRESHOLD) {
    moveForward();
  } else {
    stopMotors();
    delay(200);
    moveBackward();
    delay(400);
    stopMotors();
    
    myServo.write(30); delay(500);
    int distR = getDistance();
    myServo.write(150); delay(600);
    int distL = getDistance();
    myServo.write(90); delay(300);
    
    if (distL > distR) turnLeft(); else turnRight();
    delay(500);
    stopMotors();
  }
}

int getDistance() {
  delay(50); 
  int cm = sonar.ping_cm();
  return (cm == 0) ? MAX_DIST : cm;
}

// --- CORE MOVEMENT FUNCTIONS ---
void moveForward()  { 
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  
}
void moveBackward() { 
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); 
}
void turnLeft()     { 
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  
}
void turnRight()    { 
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); 
}
void stopMotors()   { 
  digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);  
  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);  
}
