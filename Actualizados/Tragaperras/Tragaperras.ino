const uint8_t PIN_SWITCH = 2;
const unsigned long DEBOUNCE_MS = 30;

bool estadoEstable = HIGH;
bool ultimaLectura = HIGH;
unsigned long ultimoCambio = 0;
unsigned long contador = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_SWITCH, INPUT_PULLUP);

  // Pequeña espera para estabilizar el puerto serial y sincronizar el 0 con el ESP32
  delay(200);
  Serial.println(contador);
}

void loop() {
  bool lectura = digitalRead(PIN_SWITCH);

  if (lectura != ultimaLectura) {
    ultimoCambio = millis();
    ultimaLectura = lectura;
  }

  if ((millis() - ultimoCambio) > DEBOUNCE_MS && lectura != estadoEstable) {
    estadoEstable = lectura;

    if (estadoEstable == LOW) {
      contador++;
      Serial.println(contador); // Envía únicamente el número limpio + salto de línea (\n)
    }
  }
}