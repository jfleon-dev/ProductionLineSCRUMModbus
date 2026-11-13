// ==========================================
// CÓDIGO ESP32 - GATEWAY MODBUS TCP WI-FI (3 ARDUINOS)
// ==========================================

#include <WiFi.h>
#include <ModbusIP_ESP8266.h>

// 1. CONFIGURACIÓN DE RED WI-FI
const char* SSID_WIFI = "INVESTIGADORES";
const char* PASS_WIFI = "5703215703";
const char* HOSTNAME  = "ESP32-Linea-Produccion";

// 2. PINES SERIALES HARDWARE
#define RX1_PIN 16  // Recibe distancia desde Arduino 1 (115200 baudios)
#define TX1_PIN 17

#define RX2_PIN 18  // Recibe conteo desde Arduino 2 (9600 baudios)
#define TX2_PIN 19  // Envía comandos hacia Arduino 3 Banda (9600 baudios)

ModbusIP mb;

// 3. REGISTROS HOLDING MODBUS TCP
const int DISTANCIA_HREG   = 100;
const int PIEZAS_RACK_HREG = 101;
const int CONTADOR_HREG    = 102;
const int BANDA_HREG       = 103;
const int VELOCIDAD_HREG   = 104;

// 4. PARÁMETROS DEL RACK
const float DISTANCIA_VACIO = 32.0;
const float GROSOR_PIEZA    = 1.0;
const float OFFSET_PIEZAS   = 0.0;

String bufferUltrasonico  = "";
String bufferTragamonedas = "";

int ultimoEstadoBanda = -1;
int ultimaVelocidad   = -1;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
  Serial2.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

  WiFi.setHostname(HOSTNAME);
  WiFi.begin(SSID_WIFI, PASS_WIFI);

  Serial.print("[ESP32] Conectando a INVESTIGADORES");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[ESP32] Conectado por Wi-Fi.");
  Serial.print("[ESP32] IP asignada (copiar en Python): ");
  Serial.println(WiFi.localIP());

  mb.server();
  mb.addHreg(DISTANCIA_HREG, 0);
  mb.addHreg(PIEZAS_RACK_HREG, 0);
  mb.addHreg(CONTADOR_HREG, 0);
  mb.addHreg(BANDA_HREG, 0);       // Inicia detenida (0)
  mb.addHreg(VELOCIDAD_HREG, 200); // PWM inicial en 200
}

void loop() {
  mb.task();

  // --- 1. LEER ARDUINO 1 (ULTRASÓNICO) ---
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    if (c == '\n') {
      procesarUltrasonico(bufferUltrasonico);
      bufferUltrasonico = "";
    } else if (c != '\r') {
      bufferUltrasonico += c;
    }
  }

  // --- 2. LEER ARDUINO 2 (TRAGAMONEDAS) ---
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    if (c == '\n') {
      procesarTragamonedas(bufferTragamonedas);
      bufferTragamonedas = "";
    } else if (c != '\r') {
      bufferTragamonedas += c;
    }
  }

  // --- 3. ENVIAR ORDEN DE PYTHON AL ARDUINO 3 (BANDA) ---
  int estadoBanda = mb.Hreg(BANDA_HREG);
  if (estadoBanda != ultimoEstadoBanda) {
    ultimoEstadoBanda = estadoBanda;
    if (estadoBanda == 1) {
      Serial2.println("1");
      Serial.println("[ESP32 -> Arduino 3] Orden: MARCHA (1)");
    } else {
      Serial2.println("0");
      Serial.println("[ESP32 -> Arduino 3] Orden: PARO (0)");
    }
  }

  int velocidad = mb.Hreg(VELOCIDAD_HREG);
  if (velocidad != ultimaVelocidad) {
    ultimaVelocidad = velocidad;
    Serial2.print("V");
    Serial2.println(velocidad);
  }

  yield();
}

void procesarUltrasonico(String dato) {
  dato.trim();
  if (dato.length() == 0 || !isDigit(dato.charAt(0))) return;

  float distanciaDecimal = dato.toFloat();
  if (distanciaDecimal > 0) {
    int distanciaEntera = round(distanciaDecimal);

    float espacioOcupado = DISTANCIA_VACIO - distanciaDecimal;
    if (espacioOcupado < 0) espacioOcupado = 0;

    float PD = (espacioOcupado / GROSOR_PIEZA) + OFFSET_PIEZAS;
    int PDi = round(PD);

    if (distanciaDecimal >= 29.5) PDi = 0;
    if (PDi < 0) PDi = 0;
    if (PDi > 30) PDi = 30;

    mb.Hreg(DISTANCIA_HREG, distanciaEntera);
    mb.Hreg(PIEZAS_RACK_HREG, PDi);
  }
}

void procesarTragamonedas(String dato) {
  dato.trim();
  if (dato.length() == 0 || !isDigit(dato.charAt(0))) return;

  long conteo = dato.toInt();
  mb.Hreg(CONTADOR_HREG, (uint16_t)conteo);
}