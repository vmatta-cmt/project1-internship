const int stepPin = 2;
const int dirPin = 3;
const int steps = 200;  // Number of steps in one direction

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Step 1: Rotate CW
  digitalWrite(dirPin, LOW);  // Set direction to CW
  moveMotor(steps);

  delay(1000);  // Wait 1 second

  // Step 2: Rotate CCW
  digitalWrite(dirPin, HIGH); // Reverse direction
  moveMotor(steps);
}

void loop() {
  // Do nothing forever
}

// Move the motor a given number of steps
void moveMotor(int numSteps) {
  for (int i = 0; i < numSteps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);  // Adjust speed
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
}
