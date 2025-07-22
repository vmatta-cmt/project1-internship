const int pwmPin = 11;  // OC1A on Arduino Mega

enum Phase { ON_PHASE, OFF_PHASE };
Phase currentPhase = OFF_PHASE;

unsigned long phaseStartTime = 0;
bool pwmOn = false;
bool paused = false;
bool sweepRunning = false;

int startFreq = 1;
int endFreq = 10;
int stepSize = 2;
int currentFreq = 1;

void setup() {
  pinMode(pwmPin, OUTPUT);
  Serial.begin(9600);
  delay(100);

  Serial.println("PWM Auto Sweep Setup (2s ON / 6s OFF):");
  Serial.println("Enter start frequency (Hz):");
}

void loop() {
  handleSerialInput();

  if (!sweepRunning || paused) {
    disablePWMOutput();
    return;
  }

  unsigned long now = millis();

  // === Start ON phase ===
  if (!pwmOn && (now - phaseStartTime >= 6000)) {  // 6s OFF
    setupTimer1(currentFreq);
    Serial.print("PWM ON at ");
    Serial.print(currentFreq);
    Serial.println(" Hz");

    pwmOn = true;
    phaseStartTime = now;
  }

  // === End ON phase ===
  if (pwmOn && (now - phaseStartTime >= 2000)) {  // 2s ON
    disablePWMOutput();
    Serial.println("PWM OFF");

    pwmOn = false;
    phaseStartTime = now;

    // Update frequency
    currentFreq += stepSize;
    if (currentFreq > endFreq) {
      Serial.println("[Sweep complete]");
      sweepRunning = false;
    }
  }
}

// === Serial input for pause/resume/setup ===
void handleSerialInput() {
  static int step = 0;

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.equalsIgnoreCase("pause")) {
      paused = true;
      disablePWMOutput();
      Serial.println("[PWM Paused]");
      return;
    }

    if (input.equalsIgnoreCase("resume")) {
      paused = false;
      phaseStartTime = millis();  // resume cleanly
      Serial.println("[PWM Resumed]");
      return;
    }

    // Setup sequence
    if (!sweepRunning) {
      int val = input.toInt();
      if (step == 0) {
        startFreq = val;
        Serial.print("Start frequency set to ");
        Serial.println(startFreq);
        Serial.println("Enter end frequency (Hz):");
        step++;
      } else if (step == 1) {
        endFreq = val;
        Serial.print("End frequency set to ");
        Serial.println(endFreq);
        Serial.println("Enter frequency step size (Hz):");
        step++;
      } else if (step == 2) {
        stepSize = val;
        Serial.print("Step size set to ");
        Serial.println(stepSize);
        Serial.println("[Sweep will begin]");
        currentFreq = startFreq;
        sweepRunning = true;
        phaseStartTime = millis();
        step = 0;
      }
    }

    while (Serial.available()) Serial.read();
  }
}

// === Timer1 PWM Setup ===
void setupTimer1(int freq) {
  disablePWMOutput();
  setPWMFreqHz(freq);
  delayMicroseconds(100);

  TCCR1A = 0;
  TCCR1B = 0;

  TCCR1A |= (1 << COM1A1);  // OC1A non-inverting
  TCCR1A |= (1 << WGM11);   // Fast PWM mode
  TCCR1B |= (1 << WGM12) | (1 << WGM13);
  TCCR1B |= (1 << CS12) | (1 << CS10);  // Prescaler = 1024
}

void disablePWMOutput() {
  TCCR1A &= ~(1 << COM1A1);  // Disconnect PWM
  digitalWrite(pwmPin, LOW);
}

void setPWMFreqHz(int freq) {
  const long F_CPU_HZ = 16000000L;
  const int prescaler = 1024;
  long top = F_CPU_HZ / (prescaler * freq);

  if (top > 65535) top = 65535;
  if (top < 2) top = 2;

  ICR1 = top;
  OCR1A = top / 2;  // 50% duty cycle
}
