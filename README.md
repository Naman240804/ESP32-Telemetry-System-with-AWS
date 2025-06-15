# EV Go-Kart Dual-Mode Telemetry System

A comprehensive, two-part telemetry system designed for an Electric Vehicle (EV) test platform. The system captures real-time performance data and simultaneously displays it on a local dashboard while logging it to the AWS cloud for advanced analytics.

This repository contains the two main Arduino sketches for the project:
1.  `Transmitter-AWS.ino`: The code for the on-vehicle data gateway.
2.  `Receiver-TFT.ino`: The code for the local driver dashboard.

---

### System Architecture & Core Concept

This project consists of two distinct ESP32 modules that work in tandem: a **Transmitter Gateway** and a **Receiver & Display Unit**. This dual-device architecture ensures real-time data is always available to a local driver, even if internet connectivity is lost.

#### 1. Transmitter Gateway (`Transmitter-AWS.ino`)
This module is the heart of the system, mounted on the EV.
- **Data Simulation:** Simulates key vehicle metrics such as RPM, voltage, current, and motor/controller temperatures.
- **Dual Communication:** It broadcasts this data using two protocols:
    - **ESP-NOW:** For high-speed, low-latency communication with the local dashboard.
    - **MQTT over Wi-Fi:** For secure, robust data transmission to a dedicated **AWS IoT Core** endpoint.
- **Data Formatting:** Data sent to the cloud is structured as a JSON payload for easy integration with AWS services like IoT Shadow.

#### 2. Receiver & Display Unit (`Receiver-TFT.ino`)
This module serves as the driver's real-time dashboard.
- **Data Reception:** Exclusively listens for incoming data packets via ESP-NOW.
- **Graphical Visualization:** Renders the received data on a 2.2" ILI9225 TFT screen, featuring a speedometer dial and digital readouts.

### Technology Stack
- **Hardware:** 2x ESP32 Modules, 2.2" ILI9225 TFT Display
- **Firmware & Communication:** C++ (in Arduino IDE), ESP-NOW Protocol, MQTT Protocol
- **Key Libraries:** `esp_now`, `WiFi`, `PubSubClient`, `ArduinoJson`, `TFT_22_ILI9225`
- **Cloud:** **AWS IoT Core**

### How to Replicate
1. **Hardware Setup:**
    - Connect the TFT display to one ESP32 (this will be the Receiver).
    - The second ESP32 will act as the Transmitter.
2. **Software Setup & Flashing:**
    - Open `Transmitter-AWS.ino` in the Arduino IDE. In the code, **you must replace the placeholder Wi-Fi and AWS credentials with your own**. Flash this code to the Transmitter ESP32.
    - Open `Receiver-TFT.ino` in the Arduino IDE. Flash this code to the ESP32 connected to the screen.
3. **Operation:** Power on both devices. The receiver's screen will immediately start displaying data sent from the transmitter. You can also monitor your AWS IoT console to see the incoming MQTT messages.

---
