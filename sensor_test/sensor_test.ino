// BellBots Capstone Project - Autonomous Bellcart System

#include <Keypad.h>

// GENERAL SETUP
bool DISABLED = true;
const char* demo_program_names[4] = {
  "Turn Left on Detection",
  "Turn Right on Detection",
  "Stop on Detection",
  "Forward then 180"
};

// KEYPAD SETUP
const int ROW_NUM = 4;
const int COLUMN_NUM = 4;

const char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

const byte row_pins[ROW_NUM] = {23, 25, 27, 29};
const byte column_pins[COLUMN_NUM] = {22, 24, 26, 28};
Keypad keypad = Keypad( makeKeymap(keys), row_pins, column_pins, ROW_NUM, COLUMN_NUM );

const int CODE_LENGTH = 4;
const unsigned long TIMEOUT_MS = 2000; // allow 2 seconds between keypresses before entry window resets

const char correct_code[CODE_LENGTH] = {'1', '9', '7', '2'}; // hardcoded for demo purposes
const char disable_program_code[CODE_LENGTH] = {'#', '#', '#', '#'};
const char demo_program_codes[4][CODE_LENGTH] = { {'#', 'A', '1', '1'}, {'#', 'B', '2', '2'}, {'#', 'C', '3', '3'}, {'#', 'D', '4', '4'} };
char curr_program_code[CODE_LENGTH] = {0, 0, 0, 0};

char entered_code[CODE_LENGTH];
int code_index = 0;
unsigned long last_keypress_time = 0;


// ULTRASONIC (PROXIMITY) SENSOR SETUP
const int TRIGGER_PINS[4] = {36, 44, 40, 48}; // front left (from inside cart), front right (from inside cart), left (across from doors), right (doors)
const int ECHO_PINS[4] = {37, 45, 41, 49};
const int THRESHOLD_INCHES = 36;


// MOTOR SETUP
const int DIR_PINS[2] = {8, 10}; // left, right
const int PWM_PINS[2] = {9, 11}; // left, right
const int turnDuration = 5000; // ms
const int leftBaseSpeed = 200; // physical imperfection: left motor tends to run faster than right by default
const int rightBaseSpeed = 225;


// DOOR LOCK SETUP
bool doorLocked = true;
const int actuatorPins[2] = {3, 4};


// Function Prototypes
void leftOnDetection(long duration[], long inches[]);
void rightOnDetection(long duration[], long inches[]);
void stopOnDetection(long duration[], long inches[]);
void forwardThen180(long duration[], long inches[]);

bool doorCodesAreEqual(char code1[], char code2[], int length);
bool isValidProgramCode(char code[]);
void lockDoor();
void unlockDoor();
bool forwardPathClear(long duration[], long inches[]);
bool leftPathClear(long duration[], long inches[]);
bool rightPathClear(long duration[], long inches[]);
void readSensors(long duration[], long inches[]);
long microsecondsToInches(long microseconds);
void moveForward(long duration[], long inches[]);
void stopCart();
void turnLeft();
void turnRight();
void turn180();

void testSensors(long duration[], long inches[]);
void moveBackwardFiveSeconds();


void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 4; i++) {
    pinMode(TRIGGER_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
  }

  for (int i = 0; i < 2; i++) {
    pinMode(DIR_PINS[i], OUTPUT);
    pinMode(PWM_PINS[i], OUTPUT);
  }

  pinMode(actuatorPins[0], OUTPUT);
  pinMode(actuatorPins[1], OUTPUT);

  Serial.println("Initializing Program.");
}

void loop() {
  long duration[4], inches[4];
  testSensors(duration, inches);
}


// Demonstration Programs
void leftOnDetection(long duration[], long inches[]) {
  // TASK: move forward until an object is detected, then turn left 90° and stop

  if (forwardPathClear(duration, inches)) {
    Serial.println("Free to move");

    moveForward(duration, inches);
  }
  else {
    Serial.print("Object detected ");
    Serial.print(inches[0]);
    Serial.println("in away");

    turnLeft();

    DISABLED = true; // return to program selection menu
  }
}

void rightOnDetection(long duration[], long inches[]) {
  // TASK: move forward until an object is detected, then turn right 90° and stop

  if (forwardPathClear(duration, inches)) {
    Serial.println("Free to move");

    moveForward(duration, inches);
  }
  else {
    Serial.print("Object detected ");
    Serial.print(inches[0]);
    Serial.println("in away");

    turnRight();

    DISABLED = true; // return to program selection menu
  }
}

void stopOnDetection(long duration[], long inches[]) {
  // TASK: move forward until an object is detected, then stop

  if (forwardPathClear(duration, inches)) {
    Serial.println("Free to move");

    moveForward(duration, inches);
  }
  else {
    Serial.print("Object detected ");
    Serial.print(inches[0]);
    Serial.println("in away");

    stopCart();

    DISABLED = true; // return to program selection menu
  }
}

void forwardThen180(long duration[], long inches[]) {
  // TASK: move forward for 10 seconds, do a 180° turn, drive back, and turn 180° again

  moveForward(duration, inches);
  delay(10000); // ms
  turn180();

  moveForward(duration, inches);
  delay(10000);
  turn180();

  DISABLED = true; // return to program selection menu
}

// Helper Functions
bool doorCodesAreEqual(char code1[], char code2[], int length) {
  for (int i = 0; i < length; i++) {
    if (code1[i] != code2[i]) 
      return false;
  }
  return true;
}

bool isValidProgramCode(char code[]) {
  for (int i = 0; i < 4; i++) {
    if (doorCodesAreEqual(code, demo_program_codes[i], CODE_LENGTH)) {
      return true;
    }
  }
  return false;
}

void lockDoor() {
  stopCart();
  
  if (!doorLocked) { // skip delay if door is already locked
    // move actuator out
    digitalWrite(actuatorPins[0], LOW);
    digitalWrite(actuatorPins[1], HIGH);

    delay(7000); // takes 5 seconds for actuator to extend
  }
  doorLocked = true;
}

void unlockDoor() {
  stopCart();
  
  if (doorLocked) { // skip delay if door is already unlocked
    // move actuator in
    digitalWrite(actuatorPins[0], HIGH);
    digitalWrite(actuatorPins[1], LOW);

    delay(7000); // takes 5 seconds for actuator to return
  }
  doorLocked = false;
}

bool forwardPathClear(long duration[], long inches[]) {
  return ( (duration[0] == 0 || inches[0] > THRESHOLD_INCHES) && (duration[1] == 0 || inches[1] > THRESHOLD_INCHES) );
}

bool leftPathClear(long duration[], long inches[]) {
  return (duration[2] == 0 || inches[2] > THRESHOLD_INCHES);
}

bool rightPathClear(long duration[], long inches[]) {
  return (duration[3] == 0 || inches[3] > THRESHOLD_INCHES);
}

void readSensors(long duration[], long inches[]) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(TRIGGER_PINS[i], LOW);

    delayMicroseconds(2);

    digitalWrite(TRIGGER_PINS[i], HIGH);

    delayMicroseconds(10);

    digitalWrite(TRIGGER_PINS[i], LOW);

    duration[i] = pulseIn(ECHO_PINS[i], HIGH, 6000);
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

void moveForward(long duration[], long inches[]) {
  // Cart will course-correct based on differences in detected distances between the two front sensors.
  
  const float GAIN = 1.0;
  const int maxRange = 96; // inches
  
  int leftSpeed, rightSpeed;
  
  // Only correct if both sensors got a valid reading and both are within a reliable range
  if (duration[0] != 0 && duration[1] != 0 && inches[0] < maxRange && inches[1] < maxRange) {
    int correction = constrain((inches[1] - inches[0]) * GAIN, -50, 50); // 0 = front left, 1 = front right

    leftSpeed = constrain(leftBaseSpeed - correction, 0, 255);
    rightSpeed = constrain(rightBaseSpeed + correction, 0, 255);
  } 
  else {
    leftSpeed = leftBaseSpeed;
    rightSpeed = rightBaseSpeed;
  }
  
  digitalWrite(DIR_PINS[0], HIGH); // HIGH -> forward, LOW -> backward
  digitalWrite(DIR_PINS[1], HIGH);

  analogWrite(PWM_PINS[0], leftSpeed);
  analogWrite(PWM_PINS[1], rightSpeed);
}

void stopCart() {
  analogWrite(PWM_PINS[0], 0);
  analogWrite(PWM_PINS[1], 0);
}

void turnLeft() {
  // move right motor faster than the left
  digitalWrite(DIR_PINS[0], HIGH); // HIGH -> forward, LOW -> backward
  digitalWrite(DIR_PINS[1], HIGH);

  analogWrite(PWM_PINS[0], leftBaseSpeed/2);
  analogWrite(PWM_PINS[1], rightBaseSpeed);

  delay(turnDuration);
  stopCart();
}

void turnRight() {
  // move left motor faster than the right
  digitalWrite(DIR_PINS[0], HIGH); // HIGH -> forward, LOW -> backward
  digitalWrite(DIR_PINS[1], HIGH);

  analogWrite(PWM_PINS[0], leftBaseSpeed);   // 0–255
  analogWrite(PWM_PINS[1], rightBaseSpeed/2);

  delay(turnDuration);
  stopCart();
}

void turn180() {
  // move right motor faster than the left
  digitalWrite(DIR_PINS[0], HIGH); // HIGH -> forward, LOW -> backward
  digitalWrite(DIR_PINS[1], HIGH);

  analogWrite(PWM_PINS[0], leftBaseSpeed/2);
  analogWrite(PWM_PINS[1], rightBaseSpeed);

  delay(2*turnDuration);
  stopCart();
}


// For Testing Purposes
void testSensors(long duration[], long inches[]) {
  readSensors(duration, inches);

  Serial.println(inches[0]); // front left (from inside cart)
  Serial.println(inches[1]); // front right (from inside cart)
  Serial.println(inches[2]); // left (across from doors)
  Serial.println(inches[3]); // right (doors)
  Serial.println();
}

void moveBackwardFiveSeconds() {
  digitalWrite(DIR_PINS[0], LOW); // HIGH -> forward, LOW -> backward
  digitalWrite(DIR_PINS[1], LOW);

  analogWrite(PWM_PINS[0], leftBaseSpeed);
  analogWrite(PWM_PINS[1], rightBaseSpeed);

  delay(5000);
  stopCart();
}