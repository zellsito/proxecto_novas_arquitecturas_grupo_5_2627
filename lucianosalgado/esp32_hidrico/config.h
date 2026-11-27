#pragma once

#include <Arduino.h>
#include "secrets.h"

namespace Config {
constexpr unsigned long SERIAL_BAUD = 115200;
constexpr unsigned long TELEMETRY_INTERVAL_MS = 10000;
constexpr unsigned long WIFI_RETRY_MS = 5000;
constexpr unsigned long MQTT_RETRY_MS = 5000;
constexpr uint16_t MQTT_BUFFER_SIZE = 512;

// Bus I2C compartido (SCD30) - mismos pines que tp7-2 (Firebeetle 2 ESP32-S3)
constexpr int I2C_SDA_PIN = 1;
constexpr int I2C_SCL_PIN = 2;
constexpr uint32_t I2C_FREQ_HZ = 50000;
constexpr uint8_t SCD30_ADDR = 0x61;

// Sensor capacitivo de humedad/agua SEN0193 (ADC1 Ch7 -> GPIO 8)
constexpr int SOIL_ADC_PIN = 8;
constexpr int SOIL_AIR_RAW = 3200;    // Lectura en seco (~0%)
constexpr int SOIL_WATER_RAW = 1400;  // Lectura sumergido (~100%)

// Botón integrado (escotilla) y LED integrado (extractor/ventilador)
constexpr int HATCH_BTN_PIN = 47;
constexpr int FAN_LED_PIN = 21;
constexpr unsigned long BTN_DEBOUNCE_MS = 50;

inline String telemetryTopic() {
  return String(MQTT_BASE_TOPIC) + "/telemetria/hidrico";
}

inline String environmentTopic() {
  return String(MQTT_BASE_TOPIC) + "/telemetria/ambiente";
}

inline String accessTopic() {
  return String(MQTT_BASE_TOPIC) + "/telemetria/acceso";
}

inline String actuatorsStateTopic() {
  return String(MQTT_BASE_TOPIC) + "/estado/actuadores";
}
}  // namespace Config
