import time
import json
import os
from datetime import datetime
from pymodbus.client import ModbusTcpClient
from pymodbus.exceptions import ModbusException

# ==========================================
# 1. CONFIGURACIÓN DEL SISTEMA
# ==========================================
PUERTO_MODBUS = 502
ARCHIVO_JSON = "registro_conexion.json"

def gestionar_archivo_json(accion, ip=None, estado=None):
    """
    Centraliza la lectura y escritura del archivo JSON.
    La acción 'leer' retorna el diccionario completo o None si no existe.
    La acción 'escribir' crea o sobrescribe el archivo con la IP, hora y estado.
    """
    if accion == "leer":
        if os.path.exists(ARCHIVO_JSON):
            with open(ARCHIVO_JSON, 'r') as archivo:
                return json.load(archivo)
        return None
        
    elif accion == "escribir":
        datos = {
            "direccion_ip": ip,
            "fecha_hora": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            "conexion_exitosa": estado
        }
        with open(ARCHIVO_JSON, 'w') as archivo:
            json.dump(datos, archivo, indent=4)

def monitorear_esp32():
    # ==========================================
    # 2. RESOLUCIÓN DE DIRECCIÓN IP
    # ==========================================
    registro = gestionar_archivo_json("leer")
    
    if registro and registro.get("conexion_exitosa"):
        ip_actual = registro.get("direccion_ip")
        print(f"[SISTEMA] Cargando última IP exitosa del registro: {ip_actual}")
    else:
        print("[SISTEMA] No hay IP válida registrada en el sistema.")
        ip_actual = input("[INPUT] Ingresa la dirección IP del ESP32: ").strip()

    cliente = None

    # Bucle de validación de conexión
    while True:
        cliente = ModbusTcpClient(ip_actual, port=PUERTO_MODBUS)
        print(f"\n[RED] Intentando conectar al ESP32 en {ip_actual}:{PUERTO_MODBUS}...")
        
        if cliente.connect():
            print("[MODBUS] Conexión establecida con éxito.")
            gestionar_archivo_json("escribir", ip_actual, True)
            break  # Salimos del bucle para iniciar el monitoreo
        else:
            print(f"[ERROR] No se pudo establecer la conexión TCP con {ip_actual}.")
            gestionar_archivo_json("escribir", ip_actual, False)
            ip_actual = input("[INPUT] Conexión fallida. Ingresa una nueva dirección IP: ").strip()

    # ==========================================
    # 3. LECTURA DE COILS (POLLING)
    # ==========================================
    print("\n[MODBUS] Iniciando sondeo de Coils... (Presiona Ctrl+C para salir)\n")

    try:
        while True:
            # Petición al esclavo Modbus (address=0, count=3 cubre los 3 coils mapeados)
            respuesta = cliente.read_coils(address=0, count=3)

            # Verificación de integridad de la trama
            if respuesta.isError():
                print("[ERROR] Fallo al leer los registros Modbus. Trama corrupta o timeout.")
            else:
                # respuesta.bits contiene el estado lógico de los registros leídos
                estado_coil_0 = respuesta.bits[0]
                estado_coil_1 = respuesta.bits[1]
                estado_coil_2 = respuesta.bits[2]

                # Salida formateada en consola
                print(f"Coil 0 (Pin 4): {int(estado_coil_0)} | "
                      f"Coil 1 (Pin 5): {int(estado_coil_1)} | "
                      f"Coil 2 (Pin 13): {int(estado_coil_2)}")

            # Control de tráfico: 500 ms de retardo para no saturar al ESP32
            time.sleep(0.1)

    except KeyboardInterrupt:
        print("\n[SISTEMA] Monitoreo detenido por el usuario.")
    except ModbusException as e:
        print(f"\n[ERROR] Excepción de protocolo Modbus: {e}")
    finally:
        # ==========================================
        # 4. LIBERACIÓN DE RECURSOS
        # ==========================================
        if cliente:
            cliente.close()
            print("[RED] Socket TCP cerrado correctamente.")

if __name__ == "__main__":
    monitorear_esp32()
    print("[MODBUS] Conexión establecida. Iniciando sondeo (Polling)... (Presiona Ctrl+C para salir)\n")

    try:
        while True:
            # ==========================================
            # 2. LECTURA DE COILS
            # ==========================================
            # read_coils(direccion_inicial, cantidad_de_coils)
            respuesta = cliente.read_coils(address=0, count=3)

            # Verificación de integridad de la trama
            if respuesta.isError():
                print("[ERROR] Fallo al leer los registros Modbus. Trama corrupta o timeout.")
            else:
                # respuesta.bits devuelve un arreglo booleano con el estado de los coils leídos
                estado_coil_0 = respuesta.bits[0]
                estado_coil_1 = respuesta.bits[1]
                estado_coil_2 = respuesta.bits[2]

                # Salida formateada en consola
                print(f"Coil 0 (Pin 4): {int(estado_coil_0)} | "
                      f"Coil 1 (Pin 5): {int(estado_coil_1)} | "
                      f"Coil 2 (Pin 13): {int(estado_coil_2)}")

            # ==========================================
            # 3. CONTROL DE TRÁFICO (POLLING RATE)
            # ==========================================
            # 500 ms de retardo para no saturar la pila WiFi del ESP32
            time.sleep(0.1)

    except KeyboardInterrupt:
        print("\n[SISTEMA] Monitoreo detenido por el usuario.")
    except ModbusException as e:
        print(f"\n[ERROR] Excepción de protocolo Modbus: {e}")
    finally:
        # ==========================================
        # 4. LIBERACIÓN DE RECURSOS
        # ==========================================
        cliente.close()
        print("[RED] Socket TCP cerrado correctamente.")

if __name__ == "__main__":
    monitorear_esp32()