/**
 * ============================================================================
 * Project: Autonomous Obstacle Avoiding Mobile Robot
 * Course: ME-376 Control Engineering Sessional
 * Institution: Military Institute of Science and Technology (MIST)
 * Author: Mashrur Hasan
 * Controller: ATmega328P (Arduino Uno R3)
 * Motor Driver: L298N Dual H-Bridge Driver
 * Sensors: Dual Optical Infrared (IR) Obstacle Detection Modules
 * Power Management: LM2596 DC-DC Step-Down Buck Converter (12V -> 5V Logic / Motor Rail)
 * ============================================================================
 */

// ==========================================
// PIN CONFIGURATION & MAPPING
// ==========================================

// --- L298N Dual H-Bridge Motor Driver Pins ---
// Left Motor (Motor A)
const int ENA = 5;       // PWM Speed Control Pin (Left Motor)
const int IN1 = 8;       // Direction Control 1 (Left Motor Forward)
const int IN2 = 7;       // Direction Control 2 (Left Motor Reverse)

// Right Motor (Motor B)
const int IN3 = 4;       // Direction Control 3 (Right Motor Forward)
const int IN4 = 2;       // Direction Control 4 (Right Motor Reverse)
const int ENB = 6;       // PWM Speed Control Pin (Right Motor)

// --- Infrared (IR) Proximity Sensors ---
// Note: Active-LOW sensors (LOW = Obstacle Detected, HIGH = Clear Path)
const int IR_LEFT_PIN  = 11;  // Left IR Obstacle Sensor
const int IR_RIGHT_PIN = 12;  // Right IR Obstacle Sensor

// ==========================================
// SPEED & TIMING TUNING CONSTANTS
// ==========================================
const int BASE_SPEED        = 175;  // Base cruise speed PWM (0 - 255)
const int TURN_SPEED        = 190;  // Turn boost speed PWM for responsive pivoting
const int REVERSE_SPEED     = 160;  // Safe reversing speed PWM
const int REVERSE_DURATION  = 350;  // Reversing duration in ms
const int TURN_DURATION     = 300;  // Turn duration in ms
const int SETTLING_DELAY    = 50;   // Inertia settling buffer in ms

// ==========================================
// ROBOT NAVIGATION STATES
// ==========================================
enum NavigationState {
  STATE_FORWARD,
  STATE_TURN_LEFT,
  STATE_TURN_RIGHT,
  STATE_AVOID_DEADLOCK,
  STATE_STOP
};

NavigationState currentState = STATE_FORWARD;

// ==========================================
// FUNCTION DECLARATIONS
// ==========================================
void moveForward(int speed);
void moveBackward(int speed);
void turnLeft(int speed);
void turnRight(int speed);
void stopMotors();
void executeAvoidance();

// ==========================================
// SETUP ROUTINE
// ==========================================
void setup() {
  // Initialize Serial interface for debugging & telemetry
  Serial.begin(9600);
  Serial.println(F("[SYSTEM INIT] Autonomous Obstacle Avoidance Robot Starting..."));

  // Configure Motor Control Digital & PWM Output Pins
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Configure Sensor Input Pins with Internal Pull-Ups
  pinMode(IR_LEFT_PIN, INPUT_PULLUP);
  pinMode(IR_RIGHT_PIN, INPUT_PULLUP);

  // Ensure motors are initially stationary
  stopMotors();
  delay(1000); // Startup safety delay for system stabilization
  Serial.println(F("[SYSTEM READY] Autonomous Controller Active."));
}

// ==========================================
// MAIN CONTROL LOOP
// ==========================================
void loop() {
  // Read digital outputs from active-low IR sensors
  // 0 (LOW) = Obstacle detected | 1 (HIGH) = Clear path
  int leftObstacle  = digitalRead(IR_LEFT_PIN);
  int rightObstacle = digitalRead(IR_RIGHT_PIN);

  // Decision State Machine
  if (leftObstacle == HIGH && rightObstacle == HIGH) {
    // Both sides clear -> Cruise Forward
    currentState = STATE_FORWARD;
    moveForward(BASE_SPEED);
  } 
  else if (leftObstacle == LOW && rightObstacle == HIGH) {
    // Obstacle detected on Left -> Pivot Right
    currentState = STATE_TURN_RIGHT;
    stopMotors();
    delay(SETTLING_DELAY);
    turnRight(TURN_SPEED);
    delay(TURN_DURATION);
  } 
  else if (leftObstacle == HIGH && rightObstacle == LOW) {
    // Obstacle detected on Right -> Pivot Left
    currentState = STATE_TURN_LEFT;
    stopMotors();
    delay(SETTLING_DELAY);
    turnLeft(TURN_SPEED);
    delay(TURN_DURATION);
  } 
  else if (leftObstacle == LOW && rightObstacle == LOW) {
    // Obstacle detected directly in front (both sensors triggered) -> Deadlock recovery
    currentState = STATE_AVOID_DEADLOCK;
    stopMotors();
    delay(SETTLING_DELAY);
    
    // Step 1: Back away from the obstacle
    moveBackward(REVERSE_SPEED);
    delay(REVERSE_DURATION);
    
    // Step 2: Stop and pivot to clear the frontal blockage
    stopMotors();
    delay(SETTLING_DELAY);
    turnRight(TURN_SPEED);
    delay(TURN_DURATION + 150); // Extended pivot angle
  }

  // Loop pacing delay for deterministic response time
  delay(20);
}

// ==========================================
// LOW-LEVEL MOTOR CONTROL ACTUATOR DRIVERS
// ==========================================

/**
 * Drive both motors forward at specified PWM duty cycle
 */
void moveForward(int speed) {
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

/**
 * Drive both motors in reverse at specified PWM duty cycle
 */
void moveBackward(int speed) {
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

/**
 * Differential skid steering: Pivot Left (Left reverse, Right forward)
 */
void turnLeft(int speed) {
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

/**
 * Differential skid steering: Pivot Right (Left forward, Right reverse)
 */
void turnRight(int speed) {
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

/**
 * Disable drive to all motor bridges (Brake / Coast)
 */
void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
