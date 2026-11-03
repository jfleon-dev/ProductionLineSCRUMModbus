// ==========================================
// CÓDIGO ARDUINO - EMISOR ULTRASÓNICO
// ==========================================

const int TRIG_PIN = 9;
const int ECHO_PIN = 8;

void setup() {
  // Inicializa la comunicación serial a 115200 baudios
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // 1. Generar el pulso de disparo (10 us)
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // 2. Medir el tiempo del pulso de eco
  long duracion = pulseIn(ECHO_PIN, HIGH);

  // 3. Calcular la distancia en cm (Velocidad del sonido: 0.034 cm/us)
  int distanciaCm = duracion * 0.034 / 2;

  // 4. Enviar el dato por TX si está en un rango válido (2 a 400 cm)
  if (distanciaCm >= 2 && distanciaCm <= 400) {
    Serial.println(distanciaCm); // Envía el valor seguido de salto de línea (\n)
  }

  // Tasa de envío cada 200 ms
  delay(200);
}