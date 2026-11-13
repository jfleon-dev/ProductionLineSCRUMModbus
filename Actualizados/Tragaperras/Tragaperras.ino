// ==========================================
// ARDUINO 2 - CONTADOR TRAGAMONEDAS (MICRO SWITCH P-CS-71)
// ==========================================

const uint8_t PIN_SWITCH = 2;
const unsigned long DEBOUNCE_MS = 30;

bool estadoEstable = HIGH;       // HIGH = sin presionar
bool ultimaLectura = HIGH;
unsigned long ultimoCambio = 0;
unsigned long contador = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_SWITCH, INPUT_PULLUP);
  
  // Envía 0 al encender para sincronizar el registro Modbus en el ESP32
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

    // Solo cuenta cuando se ACTIVA; al liberar no hace nada
    if (estadoEstable == LOW) {
      contador++;
      Serial.println(contador);
    }
  }
}