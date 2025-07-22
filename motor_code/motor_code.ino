const int PWM_PIN = 11;     // PWM speed control (Yellow)
const int DIR_PIN = 10;     // Direction control (Blue)
const int FG_PIN  = 9;      // Feedback pin (White)

int speedVal = 100;         // Current speed value
int lastSpeed = 100;        // Saved speed value before pause
bool direction = HIGH;      // Current motor direction
unsigned long lastPrintTime = 0;  // For live RPM updates

void setup() {
  Serial.begin(115200);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(FG_PIN, INPUT);

  digitalWrite(DIR_PIN, direction);
  analogWrite(PWM_PIN, speedVal);

  Serial.println("Motor Ready.");
  Serial.println("Type a number (0–255), 'pause', 'resume', or 'reverse'.");
}

void loop() {
  // ===== Handle Serial Commands =====
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.equalsIgnoreCase("pause")) {
      lastSpeed = speedVal;
      analogWrite(PWM_PIN, 255);
      Serial.println("Motor paused.");
    } else if (input.equalsIgnoreCase("resume")) {
      speedVal = lastSpeed;
      analogWrite(PWM_PIN, speedVal);
      Serial.print("Motor resumed at speed: ");
      Serial.println(speedVal);
    } else if (input.equalsIgnoreCase("reverse")) {
      direction = !direction;
      digitalWrite(DIR_PIN, direction);
      Serial.print("Direction reversed. Now: ");
      Serial.println(direction == HIGH ? "FORWARD" : "REVERSE");
    } else if (input.toInt() >= 0 && input.toInt() <= 255) {
      speedVal = input.toInt();
      analogWrite(PWM_PIN, speedVal);
      Serial.print("Speed set to: ");
      Serial.println(speedVal);
    } else {
      Serial.println("Invalid input. Use 0–255, 'pause', 'resume', or 'reverse'.");
    }
  }

  // ===== Live RPM + Speed Display =====
  unsigned long currentTime = millis();
  if (currentTime - lastPrintTime > 1000) { // Update every second
    long pulseTime = pulseIn(FG_PIN, HIGH, 500000);
    if (pulseTime > 0) {
      float rpm = 111111.0 / pulseTime;
      Serial.print("Live RPM: ");
      Serial.print(rpm);
      Serial.print(" | Speed Value: ");
      Serial.println(speedVal);
    } else {
      Serial.println("No FG signal detected.");
    }
    lastPrintTime = currentTime;
  }
}
