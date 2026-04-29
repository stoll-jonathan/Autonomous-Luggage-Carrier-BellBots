// BellBots Capstone Project - Autonomous Bellcart System

// TODO: physically move back sensor to right (doors), add second sensor back into forwardPathClear, make right [1] wheel faster than left [0] to steer straight


// MOTOR SETUP
const int DIR_PINS[2] = {8, 10}; // left, right
const int PWM_PINS[2] = {9, 11}; // left, right
const int turnDuration = 5000; // ms

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 2; i++) {
    pinMode(DIR_PINS[i], OUTPUT);
    pinMode(PWM_PINS[i], OUTPUT);
  }

  Serial.println("Initializing Program.");
}

void loop() {
  if (Serial.available() > 0) {
    char key = Serial.read();

    if (key == 'a') {
      moveForward();
      delay(1000);
      stopCart();
    }
  }
}


void moveForward() {
  digitalWrite(DIR_PINS[0], HIGH); // HIGH -> forward, LOW -> backward
  digitalWrite(DIR_PINS[1], HIGH);

  analogWrite(PWM_PINS[0], 255);   // 0–255
  analogWrite(PWM_PINS[1], 255);
}

void stopCart() {
  analogWrite(PWM_PINS[0], 0);
  analogWrite(PWM_PINS[1], 0);
}

void turnLeft() {
  // move right motor faster than the left
  digitalWrite(DIR_PINS[0], HIGH); // HIGH -> forward, LOW -> backward
  digitalWrite(DIR_PINS[1], HIGH);

  analogWrite(PWM_PINS[0], 120);   // 0–255
  analogWrite(PWM_PINS[1], 255);

  delay(turnDuration);
}

void turnRight() {
  // move left motor faster than the right
  digitalWrite(DIR_PINS[0], HIGH); // HIGH -> forward, LOW -> backward
  digitalWrite(DIR_PINS[1], HIGH);

  analogWrite(PWM_PINS[0], 255);   // 0–255
  analogWrite(PWM_PINS[1], 120);

  delay(turnDuration);
}

void turn180() {
  // move right motor faster than the left
  digitalWrite(DIR_PINS[0], HIGH); // HIGH -> forward, LOW -> backward
  digitalWrite(DIR_PINS[1], HIGH);

  analogWrite(PWM_PINS[0], 120);   // 0–255
  analogWrite(PWM_PINS[1], 255);

  delay(2*turnDuration);
}