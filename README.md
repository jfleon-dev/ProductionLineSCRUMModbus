# Production Line SCRUM Modbus

This repository contains the hardware and software architecture for the automated monitoring of a production line. The project integrates physical sensors, computer vision, and industrial connectivity (Modbus TCP) to manage inventory and material flow in real time.

## Project Architecture

The complete system is divided into three operational stages, designed to track the life cycle of each piece:

1. **Storage Detection (Implemented):** Monitoring the stock of pieces in the initial dispenser using ultrasonic telemetry.
2. **Conveyor Computer Vision (In Development):** Detection and validation of pieces in transit on the conveyor belt through image analysis.
3. **Packaging Counting (In Development):** Exact recording of the pieces falling into the final collection box.

## Hardware and Topology

The storage module operates with a distributed processing architecture between two microcontrollers:

* **Arduino Uno (Sensor Node):** Acquires real-time telemetry using an ultrasonic sensor (HC-SR04). It executes an oversampling algorithm (5 readings per cycle) to average out acoustic bounces, filters invalid readings, and transmits the clean distance via Serial.
* **ESP32 (Edge Gateway):** Listens to the Serial data frame from the Arduino, calculates the amount of physical pieces based on the available free space, saturates the maximum limits, and hosts a local Modbus TCP server to expose the variables to a SCADA system or HMI.

## Modbus TCP Register Map

The ESP32 operates as a Modbus Server (standard TCP port 502) and exposes the following *Holding Registers* (Hreg):

* **Register `100` (Distance):** Raw distance measured by the ultrasonic sensor in centimeters.
* **Register `101` (Pieces):** Number of available pieces. Processed value mathematically limited to a safe range of 0 to 30.

## Configured Physical Parameters

The mathematical calibration inside the ESP32 is adjusted to the following physical tolerances of the dispenser:

* **Empty storage distance:** 32.0 cm (30 cm total capacity + 2 cm sensor margin).
* **Thickness per piece:** 1.0 cm.
* **Maximum rack capacity:** 30 pieces.

## Connection Diagram

**Ultrasonic Sensor to Arduino Uno:**

* `TRIG` connected to Digital Pin 9
* `ECHO` connected to Digital Pin 8

**Arduino Uno to ESP32 (Serial Communication at 115200 baud):**

* Arduino `TX` connected to ESP32 `RX2` (Pin 16)
* Arduino `GND` connected to ESP32 `GND` (Common ground is strictly required to prevent noise in the UART frame)

## Software Dependencies

To compile and upload the code to both devices, ensure you have the following libraries installed in your environment (VS Code/PlatformIO or Arduino IDE):

* `WiFi.h`: Built into the ESP32 board core.
* `ModbusIP_ESP8266.h`: Required to run the Modbus TCP server stack on the ESP32.
