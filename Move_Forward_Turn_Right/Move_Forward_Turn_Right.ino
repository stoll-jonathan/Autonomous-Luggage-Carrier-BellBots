// BellBots Capstone Project - Autonomous Bellcart System

// IF THIS DOESN'T WORK: remove the pinmode calls for motor from setup()

#include <Keypad.h>

// GENERAL SETUP
bool DISABLED = true;

// KEYPAD SETUP
const int ROW_NUM = 4;
const int COLUMN_NUM = 4;

const char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

const byte row_pins[ROW_NUM] = {24, 26, 28, 30};
const byte column_pins[COLUMN_NUM] = {23, 25, 27, 29};
Keypad keypad = Keypad( makeKeymap(keys), row_pins, column_pins, ROW_NUM, COLUMN_NUM );

bool programEnabled = false;
const int CODE_LENGTH = 4;
const unsigned long TIMEOUT_MS = 2000; // allow 2 seconds between keypresses before entry window resets

const char correct_code[CODE_LENGTH] = {'1', '9', '7', '2'}; // hardcoded for demo purposes
const char enable_program_code[CODE_LENGTH] = {'*', '*', '*', '*'};
const char disable_program_code[CODE_LENGTH] = {'#', '#', '#', '#'};

char entered_code[CODE_LENGTH];
int code_index = 0;
unsigned long last_keypress_time = 0;


// ULTRASONIC (PROXIMITY) SENSOR SETUP
const int TRIGGER_PINS[5] = {36, 40, 44, 48, 52};
const int ECHO_PINS[5] = {37, 41, 45, 49, 53};
const int THRESHHOLD_INCHES = 36;


// MOTOR SETUP
const int DIR_PINS[2] = {8, 10};
const int PWM_PINS[2] = {9, 11};
const int turnDuration = 5000; // ms


// DOOR LOCK SETUP
const int actuatorPins[2] = {3, 4};


void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 5; i++) {
    pinMode(TRIGGER_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
  }

  for (int i = 0; i < 2; i++) {
    pinMode(DIR_PINS[i], OUTPUT);
    pinMode(PWM_PINS[i], OUTPUT);
  }

  Serial.println("Initializing Program.");
}

void loop() {

  // KEYPAD AND DOOR LOGIC
  char key = keypad.getKey();

  // Reset if too much time has passed since the last keypress
  unsigned long now = millis();
  if (code_index > 0 && (now - last_keypress_time) > TIMEOUT_MS) {
    Serial.println("Timeout - restarting entry.");
    code_index = 0;
  }
    
  if (key) {
    entered_code[code_index] = key;
    code_index++;
    last_keypress_time = now;

    Serial.print("Key pressed: ");
    Serial.println(key);

    if (code_index == CODE_LENGTH) {
      Serial.print("Code entered: ");
      for (int i = 0; i < CODE_LENGTH; i++) {
        Serial.print(entered_code[i]);
      }
      Serial.println();

      if (doorCodesAreEqual(entered_code, correct_code, CODE_LENGTH)) {
        Serial.println("Access granted!");
        Serial.println();
        unlockDoor();
      }
      else {
        Serial.println("Door Locked.");
        Serial.println();
        lockDoor();
      }

      if (doorCodesAreEqual(entered_code, enable_program_code, CODE_LENGTH)) {
        Serial.println("Program Started");
        Serial.println();
        DISABLED = false;
      }
      else if (doorCodesAreEqual(entered_code, disable_program_code, CODE_LENGTH)) {
        Serial.println("Program Stopped");
        Serial.println();
        DISABLED = true;
      }

      code_index = 0; // Reset for next entry
    }
  }


  // SENSOR AND MOVEMENT LOGIC
  if (DISABLED) {
    stopCart();
    return;
  }

  long duration[5], inches[5];
  readSensors(duration, inches);

  if (forwardPathClear(duration, inches)) {
    Serial.println("Free to move");

    moveForward();
  }
  else {
    Serial.print("Object detected ");
    Serial.print(inches[0]);
    Serial.println("in away");

    turnRight();
  }

}


bool doorCodesAreEqual(char code1[], char code2[], int length) {
  for (int i = 0; i < length; i++) {
    if (code1[i] != code2[i]) 
      return false;
  }
  return true;
}

void lockDoor() {
  // move actuator out
  digitalWrite(3, HIGH);
  digitalWrite(4, LOW);

  delay(10000); // takes 7 seconds for actuator to extend
}

void unlockDoor() {
  // move actuator in
  digitalWrite(3, LOW);
  digitalWrite(4, HIGH);

  delay(10000); // takes 7 seconds for actuator to return
}

bool forwardPathClear(long duration[], long inches[]) {
  return ( (duration[0] == 0 || inches[0] > THRESHHOLD_INCHES) && (duration[1] == 0 || inches[1] > THRESHHOLD_INCHES) );
}

bool rightPathClear(long duration[], long inches[]) {
  return (duration[2] == 0 || inches[2] > THRESHHOLD_INCHES);
}

bool leftPathClear(long duration[], long inches[]) {
  return (duration[3] == 0 || inches[3] > THRESHHOLD_INCHES);
}

bool backwardPathClear(long duration[], long inches[]) {
  return (duration[4] == 0 || inches[4] >= THRESHHOLD_INCHES);
}

void readSensors(long duration[], long inches[]) {
  digitalWrite(TRIGGER_PINS[0], LOW); // forward high
  digitalWrite(TRIGGER_PINS[1], LOW); // forward low
  digitalWrite(TRIGGER_PINS[2], LOW); // left
  digitalWrite(TRIGGER_PINS[3], LOW); // right
  digitalWrite(TRIGGER_PINS[4], LOW); // back

  delayMicroseconds(2);

  digitalWrite(TRIGGER_PINS[0], HIGH);
  digitalWrite(TRIGGER_PINS[1], HIGH);
  digitalWrite(TRIGGER_PINS[2], HIGH);
  digitalWrite(TRIGGER_PINS[3], HIGH);
  digitalWrite(TRIGGER_PINS[4], HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIGGER_PINS[0], LOW);
  digitalWrite(TRIGGER_PINS[1], LOW);
  digitalWrite(TRIGGER_PINS[2], LOW);
  digitalWrite(TRIGGER_PINS[3], LOW);
  digitalWrite(TRIGGER_PINS[4], LOW);


  for (int i = 0; i < 5; i++) {
    duration[i] = pulseIn(ECHO_PINS[i], HIGH, 30000); // timeout after 30ms (up to about 17 ft). If duration == 0, no objects in range of the sensor
    inches[i] = microsecondsToInches(duration[i]);
  }

}

long microsecondsToInches(long microseconds) {
  // According to Parallax's datasheet for the PING))), there are 73.746
  // microseconds per inch (i.e. sound travels at 1130 feet per second).
  // This gives the distance travelled by the ping, outbound and return,
  // so we divide by 2 to get the distance of the obstacle.
  // See: https://www.parallax.com/package/ping-ultrasonic-distance-sensor-downloads/
  return microseconds / 74 / 2;
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

  delayMicroseconds(turnDuration);
}

void turnRight() {
  // move left motor faster than the right
  digitalWrite(DIR_PINS[0], HIGH); // HIGH -> forward, LOW -> backward
  digitalWrite(DIR_PINS[1], HIGH);

  analogWrite(PWM_PINS[0], 255);   // 0–255
  analogWrite(PWM_PINS[1], 120);

  delayMicroseconds(turnDuration);
}

void turn180() {
  // move right motor faster than the left
  digitalWrite(DIR_PINS[0], HIGH); // HIGH -> forward, LOW -> backward
  digitalWrite(DIR_PINS[1], HIGH);

  analogWrite(PWM_PINS[0], 120);   // 0–255
  analogWrite(PWM_PINS[1], 255);

  delayMicroseconds(2*turnDuration);
}