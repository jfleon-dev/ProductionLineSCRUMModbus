import time
import cv2
import numpy as np
import tensorflow as tf
from tensorflow.keras import models, layers
from pyModbusTCP.client import ModbusClient

# ==========================================
# 1. CONEXIÓN WI-FI MODBUS TCP AL ESP32
# ==========================================
ESP32_IP = "192.168.1.100"  # <-- Pega aquí la IP que muestra el ESP32
ESP32_PORT = 502

REG_DISTANCIA = 100
REG_PIEZAS_RACK = 101
REG_CONTADOR_TRAGAMONEDAS = 102
REG_CONTROL_BANDA = 103
REG_VELOCIDAD_BANDA = 104

modbus = ModbusClient(host=ESP32_IP, port=ESP32_PORT, auto_open=True, timeout=0.5)

# Variables de estado
distancia_rack = 0
piezas_rack = 0
contador_piezas = 0
banda_en_marcha = False
motivo_paro = "INICIO EN PARO (Presiona R)"
modbus_conectado = False
ultimo_poll = 0.0
INTERVALO_POLL = 0.15  # Lectura cada 150 ms

def enviar_orden_banda(arrancar, motivo=""):
    global banda_en_marcha, motivo_paro
    valor = 1 if arrancar else 0
    if modbus.write_single_register(REG_CONTROL_BANDA, valor):
        banda_en_marcha = arrancar
        motivo_paro = motivo
        print(f"[WI-FI MODBUS] Banda -> {'MARCHA (1)' if arrancar else 'PARO (0) - ' + motivo}")
    else:
        print("[ERROR WI-FI] No se pudo enviar orden al ESP32")

# ==========================================
# 2. RED NEURONAL BINARIA
# ==========================================
model = models.Sequential([
    layers.Input(shape=(200, 200, 1)),
    layers.Conv2D(32, (3, 3), activation='relu'),
    layers.MaxPooling2D((2, 2)),
    layers.Flatten(),
    layers.Dense(64, activation='relu'),
    layers.Dense(1, activation='sigmoid')
])

model.load_weights('modelo_cuadrados_binario.h5')

# ==========================================
# 3. BUCLE DE CÁMARA Y CONTROL
# ==========================================
cap = cv2.VideoCapture(1)
cap.set(cv2.CAP_PROP_FOCUS, 85)

enviar_orden_banda(False, "INICIO EN PARO (Presiona R)")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # --- A. LECTURA DE SENSORES POR WI-FI ---
    ahora = time.time()
    if ahora - ultimo_poll >= INTERVALO_POLL:
        ultimo_poll = ahora
        registros = modbus.read_holding_registers(REG_DISTANCIA, 4)
        if registros is not None:
            modbus_conectado = True
            distancia_rack = registros[0]
            piezas_rack = registros[1]
            contador_piezas = registros[2]
            banda_en_marcha = (registros[3] == 1)
        else:
            modbus_conectado = False

    # --- B. DETECCIÓN DE FORMAS ---
    grayframe = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blur = cv2.blur(grayframe, (10, 10))
    _, binaryframe = cv2.threshold(blur, 200, 255, cv2.THRESH_BINARY)

    contours, _ = cv2.findContours(binaryframe, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    no_cuadrado_detectado = False

    for contour in contours:
        area = cv2.contourArea(contour)
        if area < 500:
            continue

        x, y, w, h = cv2.boundingRect(contour)
        figura_crop = binaryframe[y:y+h, x:x+w]
        if figura_crop.size == 0:
            continue

        figura_resized = cv2.resize(figura_crop, (200, 200))
        image_normalized = figura_resized / 255.0
        image_normalized = np.expand_dims(image_normalized, axis=(-1, 0))
        image_normalized = tf.convert_to_tensor(image_normalized, dtype=tf.float32)

        prediction = model(image_normalized, training=False).numpy()[0][0]

        aspect_ratio = float(w) / h
        perimeter = cv2.arcLength(contour, True)
        circularity = (4 * np.pi * area) / (perimeter ** 2) if perimeter > 0 else 0

        if circularity > 0.80:
            label = f"No Cuadrado ({prediction:.2f})"
            color = (0, 0, 255)
            no_cuadrado_detectado = True
        elif prediction >= 0.5 and 0.8 <= aspect_ratio <= 1.2:
            label = f"Cuadrado ({prediction:.2f})"
            color = (0, 255, 0)
        else:
            label = f"No Cuadrado ({prediction:.2f})"
            color = (0, 0, 255)
            no_cuadrado_detectado = True

        cv2.putText(frame, label, (x, max(20, y - 10)),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)
        cv2.rectangle(frame, (x, y), (x + w, y + h), color, 2)

    # --- C. PARO AUTOMÁTICO POR WI-FI AL DETECTAR NO CUADRADO ---
    if no_cuadrado_detectado and banda_en_marcha:
        enviar_orden_banda(False, "PIEZA NO CUADRADA DETECTADA")

    # --- D. PANEL DE DATOS EN PANTALLA ---
    overlay = frame.copy()
    cv2.rectangle(overlay, (10, 10), (500, 165), (20, 20, 20), -1)
    cv2.addWeighted(overlay, 0.75, frame, 0.25, 0, frame)

    estado_wifi = f"CONECTADO ({ESP32_IP})" if modbus_conectado else "DESCONECTADO - Revisar IP/Red"
    color_wifi = (0, 255, 0) if modbus_conectado else (0, 0, 255)

    texto_banda = "EN MARCHA" if banda_en_marcha else f"DETENIDA ({motivo_paro})"
    color_banda = (0, 255, 0) if banda_en_marcha else (0, 0, 255)

    cv2.putText(frame, f"Wi-Fi ESP32: {estado_wifi}", (20, 35),
                cv2.FONT_HERSHEY_SIMPLEX, 0.55, color_wifi, 2)
    cv2.putText(frame, f"Contador Tragamonedas: {contador_piezas} piezas", (20, 65),
                cv2.FONT_HERSHEY_SIMPLEX, 0.65, (255, 255, 0), 2)
    cv2.putText(frame, f"Rack: {piezas_rack} piezas  |  Distancia: {distancia_rack} cm", (20, 95),
                cv2.FONT_HERSHEY_SIMPLEX, 0.58, (255, 255, 255), 2)
    cv2.putText(frame, f"Banda: {texto_banda}", (20, 125),
                cv2.FONT_HERSHEY_SIMPLEX, 0.55, color_banda, 2)
    cv2.putText(frame, "Teclas: [R] Arrancar Banda  [S] Parar  [Q] Salir", (20, 152),
                cv2.FONT_HERSHEY_SIMPLEX, 0.48, (200, 200, 200), 1)

    cv2.imshow('Detector de Cuadrados - Linea Wi-Fi', frame)

    # --- E. CONTROLES MANUALES ---
    keypress = cv2.waitKey(1) & 0xFF
    if keypress == ord('q') or keypress == ord('Q'):
        enviar_orden_banda(False, "CIERRE DE SISTEMA")
        break
    elif keypress == ord('r') or keypress == ord('R'):
        enviar_orden_banda(True, "")
    elif keypress == ord('s') or keypress == ord('S'):
        enviar_orden_banda(False, "PARO MANUAL")

cap.release()
cv2.destroyAllWindows()
modbus.close()