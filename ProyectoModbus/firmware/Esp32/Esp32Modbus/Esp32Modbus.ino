// ==========================================
// CÓDIGO ARDUINO - EMISOR ULTRASÓNICO (ACTUALIZADO)
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
  long sumaDuracion = 0;
  
  // Tomar 5 muestras para estabilizar la lectura
  for (int i = 0; i < 5; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    sumaDuracion += pulseIn(ECHO_PIN, HIGH);
    delay(10); // Breve pausa entre pulsos
  }
  
  long promedioDuracion = sumaDuracion / 5;
  
  // Calcular con decimales y aplicar redondeo matemático (.5 sube)
  float distanciaReal = promedioDuracion * 0.034 / 2.0;
  int distanciaCm = round(distanciaReal); 
  
  // Enviar el dato por TX si está en un rango válido (2 a 400 cm)
  if (distanciaCm >= 2 && distanciaCm <= 400) {
    Serial.println(distanciaCm); // Envía el valor seguido de salto de línea (\n)
  }
  
  // Tasa de envío (150 ms de pausa + las pausas del ciclo for)
  delay(150);
}