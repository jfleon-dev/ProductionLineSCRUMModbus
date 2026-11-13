// ==========================================
// CÓDIGO ESP32 - PRUEBA LOCAL (CON DECIMALES)
// ==========================================

#include <WiFi.h>
#include <ModbusIP_ESP8266.h>

const char* SSID_WIFI = "POCO M7";
const char* PASS_WIFI = "DatosDylPoco";
const char* HOSTNAME  = "ESP32-Modbus-Sensor";

#define RX2_PIN 16
#define TX2_PIN 17

// ModbusIP mb; // Comentado para prueba local

const int DISTANCIA_IREG = 100;
const int PIEZAS_IREG = 101; 

// Ya ajustado a 30.0 cm para que inicie en 0 piezas
const float DISTANCIA_VACIO = 32.0; 
const float GROSOR_PIEZA    = 1.0;  
const float OFFSET_PIEZAS   = 0.0;  

void setup() {
  Serial.begin(115200);  
  Serial1.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN); 

  Serial.println("\n[ESP32] Modo prueba local. Wi-Fi y Modbus desactivados.");
}

void loop() {
  // mb.task(); // Comentado para prueba local

  if (Serial1.available() > 0) {
    float distanciaDecimal = Serial1.parseFloat();

    if (distanciaDecimal > 0) {
      int distanciaEntera = round(distanciaDecimal);
      
      float espacioOcupado = DISTANCIA_VACIO - distanciaDecimal;
      if (espacioOcupado < 0) espacioOcupado = 0;

      float PD = (espacioOcupado / GROSOR_PIEZA) + OFFSET_PIEZAS;
      int PDi = round(PD); // <-- Aquí se crea la variable PDi

      // --- CANDADO DE SEGURIDAD ---
      // Si el sensor lee 29.5 cm o más, forzamos 0 piezas
      if (distanciaDecimal >= 29.5) {
        PDi = 0;
      }
      // ----------------------------

      if (PDi < 0) PDi = 0;
      if (PDi > 30) PDi = 30; 

      /* --- ESCRITURA MODBUS ---
      mb.Hreg(DISTANCIA_IREG, distanciaEntera);
      mb.Hreg(PIEZAS_IREG, PDi);
      --------------------------- */

      // Muestra los valores en el monitor
      Serial.print("[ESP32] Distancia Decimal: ");
      Serial.print(distanciaDecimal);
      Serial.print(" cm | Distancia Entera: ");
      Serial.print(distanciaEntera);
      Serial.print(" cm | Piezas: ");
      Serial.println(PDi);
    }
  }
  yield();
}