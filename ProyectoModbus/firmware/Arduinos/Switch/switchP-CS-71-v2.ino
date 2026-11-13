// Contador de piezas en cinta transportadora
// Micro switch P-CS-71: COM -> GND | NO -> Pin 2 (INPUT_PULLUP)

const uint8_t PIN_SWITCH = 2;
const unsigned long DEBOUNCE_MS = 30;

bool estadoEstable = HIGH;       // HIGH = sin presionar
bool ultimaLectura = HIGH;
unsigned long ultimoCambio = 0;
unsigned long contador = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_SWITCH, INPUT_PULLUP);
  Serial.println("Contador listo.");
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