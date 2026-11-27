#pragma once

struct WaterNodeState {
  // Subsistema hídrico (SEN0193 en GPIO 8)
  float tankLevelPct = 0.0f;
  int soilRawAdc = 0;
  bool leakDetected = false;
  bool valveOpen = true;

  // Subsistema ambiental (SCD30 por I2C 0x61 en SDA=1, SCL=2)
  bool scd30Ready = false;
  float temperatureC = 0.0f;
  float humidityPct = 0.0f;
  float co2Ppm = 0.0f;
  bool fanOn = false;

  // Subsistema acceso (botón integrado GPIO 47)
  bool hatchOpen = false;

  // Modo de lectura: hardware real por defecto; sim para override manual
  bool useHardwareSensors = true;
  bool autoSimEnabled = false;

  const char* valveStateStr() const {
    return valveOpen ? "OPEN" : "CLOSED";
  }

  const char* fanStateStr() const {
    return fanOn ? "ON" : "OFF";
  }

  const char* hatchStateStr() const {
    return hatchOpen ? "open" : "closed";
  }
};
