// ==========================================
// CÓDIGO ARDUINO - EMISOR ULTRASÓNICO (DECIMALES)
// ==========================================

const int TRIG_PIN = 9;
const int ECHO_PIN = 8;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  long sumaDuracion = 0;
  
  for (int i = 0; i < 5; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    sumaDuracion += pulseIn(ECHO_PIN, HIGH);
    delay(10);
  }
  
  long promedioDuracion = sumaDuracion / 5;
  
  // Calcula la distancia real con decimales
  float distanciaReal = promedioDuracion * 0.034 / 2.0;
  
  if (distanciaReal >= 2.0 && distanciaReal <= 400.0) {
    // Envía el valor con 2 decimales al ESP32
    Serial.println(distanciaReal, 2); 
  }
  
  delay(150);
}
