const int pwmPin = 11;  // OC1A on Arduino Mega

enum Phase { ON_PHASE, OFF_PHASE };
Phase currentPhase = OFF_PHASE;

unsigned long phaseStartTime = 0;
bool pwmOn = false;
bool paused = false;
bool sweepRunning = false;
bool disclaimerShown = false;

int sweepFrequencies[] = {
  2, 3, 4, 5, 6, 7, 8, 10, 13, 14, 15, 16, 17, 19, 20, 21, 22, 24, 25,
  26, 27, 28, 29, 30, 31, 66, 67, 68, 70, 71, 72, 73, 74, 75, 76, 77,
  78, 79, 80, 81, 83, 84, 85, 86, 87, 88, 89, 90, 92, 93, 94, 95
};
const int sweepCount = sizeof(sweepFrequencies) / sizeof(int);
int sweepIndex = 0;

int userStartFreq = 1;
int userEndFreq = 100;
int userStep = 1;
int currentFreq = 0;
bool useAutoList = true;

void setup() {
  pinMode(pwmPin, OUTPUT);
  Serial.begin(9600);
  delay(100);

  if (!disclaimerShown) {
    Serial.println("⚠️ NOTE: Some frequencies are known to be unreliable and will be skipped.");
    Serial.println("Auto and custom sweeps are filtered to only use known working frequencies.");
    Serial.println("=====================================================================");
    disclaimerShown = true;
  }

  Serial.println("Commands:");
  Serial.println("  'auto'   → Auto sweep (all valid freqs once)");
  Serial.println("  'custom' → Enter your own sweep (only valid freqs will run)");
  Serial.println("  'pause'  → Pause the sweep");
  Serial.println("  'resume' → Resume the sweep");
  Serial.print("> ");

  // ✅ Start immediately — no delay before first ON
  phaseStartTime = millis() - 6000;
}

void loop() {
  handleSerialInput();

  if (!sweepRunning || paused) {
    disablePWMOutput();
    return;
  }

  unsigned long now = millis();

  // === Start ON Phase ===
  if (!pwmOn && (now - phaseStartTime >= 6000)) {
    setupTimer1(currentFreq);
    Serial.print("PWM ON at ");
    Serial.print(currentFreq);
    Serial.println(" Hz");

    pwmOn = true;
    phaseStartTime = now;
  }

  // === End ON Phase ===
  if (pwmOn && (now - phaseStartTime >= 2000)) {
    disablePWMOutput();
    Serial.println("PWM OFF");

    pwmOn = false;
    phaseStartTime = now;

    // === Advance Frequency ===
    if (useAutoList) {
      sweepIndex++;
      if (sweepIndex >= sweepCount) {
        Serial.println("[Auto Sweep complete]");
        sweepRunning = false;
        return;
      }
      currentFreq = sweepFrequencies[sweepIndex];
    } else {
      int nextFreq = currentFreq + userStep;
      while (nextFreq <= userEndFreq && !isValidFreq(nextFreq)) {
        Serial.print("Skipping invalid freq: ");
        Serial.println(nextFreq);
        nextFreq += userStep;
      }
      if (nextFreq > userEndFreq) {
        Serial.println("[Custom Sweep complete]");
        sweepRunning = false;
        return;
      }
      currentFreq = nextFreq;
    }
  }
}

bool isValidFreq(int freq) {
  for (int i = 0; i < sweepCount; i++) {
    if (sweepFrequencies[i] == freq) return true;
  }
  return false;
}

void handleSerialInput() {
  static int inputStep = 0;

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toLowerCase();

    if (input == "pause") {
      paused = true;
      disablePWMOutput();
      Serial.println("[PWM Paused]");
      return;
    }

    if (input == "resume") {
      paused = false;
      phaseStartTime = millis();
      Serial.println("[PWM Resumed]");
      return;
    }

    if (input == "auto") {
      useAutoList = true;
      sweepIndex = 0;
      currentFreq = sweepFrequencies[sweepIndex];
      sweepRunning = true;
      phaseStartTime = millis() - 6000;
      Serial.println("[Auto sweep starting]");
      return;
    }

    if (input == "custom") {
      useAutoList = false;
      inputStep = 0;
      Serial.println("Enter start frequency:");
      return;
    }

    if (!useAutoList && !sweepRunning) {
      int val = input.toInt();
      if (inputStep == 0) {
        userStartFreq = val;
        currentFreq = userStartFreq;
        Serial.println("Enter end frequency:");
        inputStep++;
      } else if (inputStep == 1) {
        userEndFreq = val;
        Serial.println("Enter step size:");
        inputStep++;
      } else if (inputStep == 2) {
        userStep = val;
        sweepRunning = true;
        inputStep = 0;
        phaseStartTime = millis() - 6000;
        Serial.println("[Custom sweep starting]");
      }
    }

    while (Serial.available()) Serial.read();  // Clear buffer
  }
}

// === Timer1 PWM Setup ===
void setupTimer1(int freq) {
  disablePWMOutput();
  setPWMFreqHz(freq);
  delayMicroseconds(100);

  TCCR1A = 0;
  TCCR1B = 0;

  TCCR1A |= (1 << COM1A1);
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM12) | (1 << WGM13);
  TCCR1B |= (1 << CS12) | (1 << CS10);
}

void disablePWMOutput() {
  TCCR1A &= ~(1 << COM1A1);
  digitalWrite(pwmPin, LOW);
}

void setPWMFreqHz(int freq) {
  const long F_CPU_HZ = 16000000L;
  const int prescaler = 1024;
  long top = F_CPU_HZ / (prescaler * freq);

  if (top > 65535) top = 65535;
  if (top < 2) top = 2;

  ICR1 = top;
  OCR1A = top / 2;
}
