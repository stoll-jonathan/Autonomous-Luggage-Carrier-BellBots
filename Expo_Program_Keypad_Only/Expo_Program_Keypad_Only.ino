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


// DOOR LOCK SETUP
const int actuatorPins[2] = {3, 4};


void setup() {
  Serial.begin(9600);

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
