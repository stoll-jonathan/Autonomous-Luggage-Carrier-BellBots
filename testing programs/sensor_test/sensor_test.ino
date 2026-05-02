// BellBots Capstone Project - Autonomous Bellcart System

// ULTRASONIC (PROXIMITY) SENSOR SETUP
const int TRIGGER_PINS[4] = {36, 44, 40, 48}; // front left (from inside cart), front right (from inside cart), left (across from doors), right (doors)
const int ECHO_PINS[4] = {37, 45, 41, 49};
const int THRESHOLD_INCHES = 36;

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 4; i++) {
    pinMode(TRIGGER_PINS[i], OUTPUT);
    pinMode(ECHO_PINS[i], INPUT);
  }

  Serial.println("Initializing Program.");
}

void loop() {
  long duration[4], inches[4];
  testSensors(duration, inches);
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


// For Testing Purposes
void testSensors(long duration[], long inches[]) {
  readSensors(duration, inches);

  Serial.println(inches[0]); // front left (from inside cart)
  Serial.println(inches[1]); // front right (from inside cart)
  Serial.println(inches[2]); // left (across from doors)
  Serial.println(inches[3]); // right (doors)
  Serial.println();
}
