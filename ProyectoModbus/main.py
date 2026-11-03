// ==========================================
// CÓDIGO ESP32 - SERVIDOR MODBUS TCP + RX SERIAL
// ==========================================

#include <WiFi.h>
#include <ModbusIP_ESP8266.h>

// 1. CONFIGURACIÓN DE RED
const char* SSID_WIFI = "Iphone de Cheinbau";
const char* PASS_WIFI = "12345678!";
const char* HOSTNAME  = "ESP32-Modbus-Sensor";

// 2. CONFIGURACIÓN DE HARDWARE Y MODBUS
#define RX2_PIN 16 // Recepción serial desde el Arduino
#define TX2_PIN 17 // Transmisión serial hacia el Arduino

ModbusIP mb;

// Direcciones de registros Holding (Modbus TCP)
const int DISTANCIA_IREG = 100;
const int PIEZAS_IREG = 101; 

// ==========================================
// 3. PARÁMETROS DE CÁLCULO FÍSICO CORREGIDOS
// ==========================================
const float DISTANCIA_VACIO = 32.0; // Distancia del sensor al fondo (30cm de piezas + 2cm de margen)
const float GROSOR_PIEZA    = 1.0;  // Altura exacta de cada pieza
const float OFFSET_PIEZAS   = 0.0;  // Ajuste fino 

int ultimaDistancia = 0;

void setup() {
  Serial.begin(115200);  
  Serial1.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN); // Uso de Serial1 para compatibilidad

  WiFi.setHostname(HOSTNAME);
  WiFi.begin(SSID_WIFI, PASS_WIFI);
  
  Serial.print("[ESP32] Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n[ESP32] Wi-Fi conectado exitosamente.");
  Serial.print("[ESP32] IP: ");
  Serial.println(WiFi.localIP());

  mb.server();
  mb.addHreg(DISTANCIA_IREG, 0); 
  mb.addHreg(PIEZAS_IREG, 0);    
}

void loop() {
  mb.task();

  if (Serial1.available() > 0) {
    int distanciaRecibida = Serial1.parseInt();

    if (distanciaRecibida > 0) {
      ultimaDistancia = distanciaRecibida;
      
      // 1. Calcular espacio ocupado
      float espacioOcupado = DISTANCIA_VACIO - (float)ultimaDistancia;
      
      // Si lee más de 32cm (vacío total o falso eco), forzamos a 0
      if (espacioOcupado < 0) espacioOcupado = 0;

      // 2. Calcular piezas con redondeo
      float PD = (espacioOcupado / GROSOR_PIEZA) + OFFSET_PIEZAS;
      int PDi = round(PD);

      // 3. Saturación (Límites)
      if (PDi < 0) PDi = 0;
      if (PDi > 30) PDi = 30; // Límite máximo de piezas

      // 4. Escribir en Modbus
      mb.Hreg(DISTANCIA_IREG, ultimaDistancia);
      mb.Hreg(PIEZAS_IREG, PDi);

      Serial.print("[ESP32] Distancia Cruda: ");
      Serial.print(ultimaDistancia);
      Serial.print(" cm | Espacio Ocupado: ");
      Serial.print(espacioOcupado);
      Serial.print(" cm | Piezas disponibles: ");
      Serial.println(PDi);
    }
  }
  yield();
}