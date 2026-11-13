// ==========================================
// CÓDIGO ARDUINO 3 - CONTROL BANDA TRANSPORTADORA L298N
// ==========================================

const uint8_t PIN_IN1 = 6;
const uint8_t PIN_IN2 = 5;
const uint8_t PIN_ENA = 9;

bool motorEncendido = false;
uint8_t velocidadPWM = 200;

void setup() {
  Serial.begin(9600);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_ENA, OUTPUT);

  pararMotor();
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    if (cmd == '1' || cmd == 'R' || cmd == 'r') {
      motorEncendido = true;
    } 
    else if (cmd == '0' || cmd == 'S' || cmd == 's') {
      pararMotor();
    } 
    else if (cmd == 'V' || cmd == 'v') {
      int val = Serial.parseInt();
      velocidadPWM = constrain(val, 0, 255);
      if (motorEncendido) {
        analogWrite(PIN_ENA, velocidadPWM);
      }
    }
  }

  if (motorEncendido) {
    digitalWrite(PIN_IN1, HIGH);
    digitalWrite(PIN_IN2, LOW);
    analogWrite(PIN_ENA, velocidadPWM);
  } else {
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
    analogWrite(PIN_ENA, 0);
  }
}

void pararMotor() {
  motorEncendido = false;
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  analogWrite(PIN_ENA, 0);
}