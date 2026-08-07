// ======== RPM SENSOR - ARDUINO PRO MINI ========
// This code measures drum RPM using a hall sensor and sends it to Mega2560

// Use Hardware Serial for communication with Mega2560
// Connect: Pro Mini TX (pin 1) -> Mega2560 RX1 (pin 19)
// Connect: Pro Mini RX (pin 0) -> Mega2560 TX1 (pin 18)
// Connect: GND -> GND

// ======== SETTINGS ========
const byte hallPin = 2;           // Interrupt pin (must be 2 or 3 on Pro Mini)
const byte pulsesPerRev = 1;      // Number of magnets/pulses per revolution

// ======== VARIABLES ========
volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;
volatile bool newPulse = false;

unsigned long rpm = 0; 
unsigned long lastSendTime = 0;  
unsigned long lastRpmCalculation = 0;
const unsigned long sendInterval = 100;    // Send every 100ms for smooth display
const unsigned long rpmTimeout = 2000000;  // 2 seconds timeout (2,000,000 microseconds)

// ======== INTERRUPT ROUTINE ========
void pulseISR() {
  unsigned long now = micros();
  
  // Debounce: ignore pulses less than 5ms apart
  if (now - lastPulseTime > 5000) {
    pulseInterval = now - lastPulseTime;
    lastPulseTime = now;
    newPulse = true;
  }
}

void setup() {
  Serial.begin(9600);  // Use hardware serial for communication
  pinMode(hallPin, INPUT_PULLUP);  // Enable internal pullup
  attachInterrupt(digitalPinToInterrupt(hallPin), pulseISR, FALLING);  // Trigger on falling edge
  
  // Send initial zero value
  Serial.println("0");
}

void loop() {
  unsigned long currentTime = micros();
  
  // Calculate RPM when we have a new pulse
  if (newPulse) {
    noInterrupts();
    unsigned long interval = pulseInterval;
    newPulse = false;
    interrupts();
    
    if (interval > 0) {
      // RPM = (60 seconds * 1,000,000 microseconds) / (interval * pulses per rev)
      rpm = (60UL * 1000000UL) / (interval * pulsesPerRev);
      lastRpmCalculation = currentTime;
    }
  }
  
  // Check for timeout (drum stopped)
  if (currentTime - lastPulseTime > rpmTimeout) {
    rpm = 0;
  }
  
  // Send RPM data at regular intervals
  if (millis() - lastSendTime >= sendInterval) {
    Serial.println(rpm);
    lastSendTime = millis();
  }
  
  delay(10); // Small delay to prevent overwhelming the loop
}