#include "sensors_hw.h"

#include <Arduino.h>
#include <Wire.h>
#include <string.h>

#include "config.h"

namespace {
bool scd30Detected = false;
int lastBtnReading = HIGH;
int stableBtnState = HIGH;
unsigned long lastBtnChangeMs = 0;

// Comandos SCD30 (con CRC-8 Sensirion, polinomio 0x31, init 0xFF)
constexpr uint8_t CMD_START_CONT[5] = {0x00, 0x10, 0x00, 0x00, 0x81};  // Presión = 0 mbar
constexpr uint8_t CMD_SET_INTERVAL[5] = {0x46, 0x00, 0x00, 0x02, 0xE3};  // 2 s
constexpr uint8_t CMD_DATA_READY[2] = {0x02, 0x02};
constexpr uint8_t CMD_READ_MEAS[2] = {0x03, 0x00};

uint8_t scd30Crc8(const uint8_t* data, size_t len) {
  uint8_t crc = 0xFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t bit = 8; bit > 0; --bit) {
      crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x31)
                         : static_cast<uint8_t>(crc << 1);
    }
  }
  return crc;
}

bool scd30Write(const uint8_t* buf, size_t len) {
  Wire.beginTransmission(Config::SCD30_ADDR);
  Wire.write(buf, len);
  return Wire.endTransmission() == 0;
}

bool scd30ReadBytes(uint8_t* buf, size_t len) {
  size_t received = Wire.requestFrom(static_cast<int>(Config::SCD30_ADDR),
                                     static_cast<int>(len));
  if (received != len) {
    return false;
  }
  for (size_t i = 0; i < len; i++) {
    buf[i] = static_cast<uint8_t>(Wire.read());
  }
  return true;
}

float unpackBigEndianFloat(const uint8_t* word1, const uint8_t* word2) {
  uint32_t raw = (static_cast<uint32_t>(word1[0]) << 24) |
                 (static_cast<uint32_t>(word1[1]) << 16) |
                 (static_cast<uint32_t>(word2[0]) << 8) |
                 static_cast<uint32_t>(word2[1]);
  float val = 0.0f;
  memcpy(&val, &raw, sizeof(float));
  return val;
}

void readSoilSensor(WaterNodeState& state) {
  constexpr int SAMPLES = 8;
  long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(Config::SOIL_ADC_PIN);
    delay(2);
  }
  int raw = static_cast<int>(sum / SAMPLES);
  state.soilRawAdc = raw;

  if (raw >= Config::SOIL_AIR_RAW) {
    state.tankLevelPct = 0.0f;
  } else if (raw <= Config::SOIL_WATER_RAW) {
    state.tankLevelPct = 100.0f;
  } else {
    float span = static_cast<float>(Config::SOIL_AIR_RAW - Config::SOIL_WATER_RAW);
    state.tankLevelPct = (static_cast<float>(Config::SOIL_AIR_RAW - raw) * 100.0f) / span;
  }
}

void readScd30Sensor(WaterNodeState& state) {
  if (!scd30Detected) {
    return;
  }

  if (!scd30Write(CMD_DATA_READY, sizeof(CMD_DATA_READY))) {
    Serial.println("[SCD30] Error enviando consulta data-ready");
    return;
  }
  delay(5);

  uint8_t dr[3] = {0};
  if (!scd30ReadBytes(dr, sizeof(dr))) {
    return;
  }
  if (scd30Crc8(dr, 2) != dr[2]) {
    Serial.println("[SCD30] CRC inválido en data-ready");
    return;
  }
  uint16_t ready = (static_cast<uint16_t>(dr[0]) << 8) | dr[1];
  if (ready != 1) {
    return;
  }

  if (!scd30Write(CMD_READ_MEAS, sizeof(CMD_READ_MEAS))) {
    Serial.println("[SCD30] Error solicitando medición");
    return;
  }
  delay(5);

  uint8_t frame[18] = {0};
  if (!scd30ReadBytes(frame, sizeof(frame))) {
    Serial.println("[SCD30] Error leyendo trama de 18 bytes");
    return;
  }

  for (size_t i = 0; i < 18; i += 3) {
    if (scd30Crc8(&frame[i], 2) != frame[i + 2]) {
      Serial.println("[SCD30] CRC inválido en trama de medición");
      return;
    }
  }

  state.co2Ppm = unpackBigEndianFloat(&frame[0], &frame[3]);
  state.temperatureC = unpackBigEndianFloat(&frame[6], &frame[9]);
  state.humidityPct = unpackBigEndianFloat(&frame[12], &frame[15]);
  state.scd30Ready = true;
}
}  // namespace

void initSensorsHw() {
  // 1. ADC1 para sensor de humedad/agua SEN0193 en GPIO 8
  analogReadResolution(12);
  analogSetPinAttenuation(Config::SOIL_ADC_PIN, ADC_11db);
  Serial.print("[HW] SEN0193 configurado en GPIO ");
  Serial.println(Config::SOIL_ADC_PIN);

  // 2. Bus I2C para SCD30 (SDA=GPIO 1, SCL=GPIO 2, 50 kHz por clock stretching)
  Wire.begin(Config::I2C_SDA_PIN, Config::I2C_SCL_PIN);
  Wire.setClock(Config::I2C_FREQ_HZ);
  Wire.setTimeOut(200);

  if (scd30Write(CMD_START_CONT, sizeof(CMD_START_CONT))) {
    delay(20);
    scd30Write(CMD_SET_INTERVAL, sizeof(CMD_SET_INTERVAL));
    scd30Detected = true;
    Serial.println("[HW] SCD30 detectado en I2C 0x61 (SDA=1, SCL=2).");
  } else {
    scd30Detected = false;
    Serial.println("[!] SCD30 no respondió en I2C 0x61 (SDA=1, SCL=2). Revisar cableado.");
  }
  // 3. Botón integrado (escotilla en GPIO 47) y LED integrado (ventilador en GPIO 21)
  pinMode(Config::HATCH_BTN_PIN, INPUT_PULLUP);
  pinMode(Config::FAN_LED_PIN, OUTPUT);
  digitalWrite(Config::FAN_LED_PIN, LOW);
  Serial.print("[HW] Botón escotilla en GPIO ");
  Serial.print(Config::HATCH_BTN_PIN);
  Serial.print(" | LED ventilador en GPIO ");
  Serial.println(Config::FAN_LED_PIN);
}

void readSensorsHw(WaterNodeState& state) {
  if (state.useHardwareSensors) {
    readSoilSensor(state);
  }
  readScd30Sensor(state);
}

bool pollHatchButton(WaterNodeState& state) {
  int reading = digitalRead(Config::HATCH_BTN_PIN);
  unsigned long now = millis();

  if (reading != lastBtnReading) {
    lastBtnChangeMs = now;
    lastBtnReading = reading;
  }

  if ((now - lastBtnChangeMs) >= Config::BTN_DEBOUNCE_MS && reading != stableBtnState) {
    stableBtnState = reading;
    bool openNow = (stableBtnState == LOW);  // Apretado (LOW) = abierta; suelto (HIGH) = cerrada
    if (openNow != state.hatchOpen) {
      state.hatchOpen = openNow;
      Serial.print("[BOTON] Escotilla -> ");
      Serial.println(state.hatchStateStr());
      return true;
    }
  }
  return false;
}

void setFanLed(bool on) {
  digitalWrite(Config::FAN_LED_PIN, on ? HIGH : LOW);
}
