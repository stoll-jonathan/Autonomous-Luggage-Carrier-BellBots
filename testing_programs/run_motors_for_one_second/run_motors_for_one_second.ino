// BellBots Capstone Project - Autonomous Bellcart System

// MOTOR SETUP
const int DIR_PINS[2] = {8, 10}; // right, left
const int PWM_PINS[2] = {9, 11}; // right, lef

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
