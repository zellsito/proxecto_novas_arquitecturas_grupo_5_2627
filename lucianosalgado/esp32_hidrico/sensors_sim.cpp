#include "sensors_sim.h"

#include <Arduino.h>

void initSensorsSim() {
  randomSeed(static_cast<uint32_t>(esp_random()));
}

void stepSensorsSim(WaterNodeState& state) {
  if (!state.autoSimEnabled) {
    return;
  }

  float jitter = random(-4, 5) / 10.0f;
  state.tankLevelPct += state.valveOpen ? (1.5f + jitter) : (-1.2f + jitter);

  if (state.tankLevelPct > 100.0f) state.tankLevelPct = 100.0f;
  if (state.tankLevelPct < 0.0f) state.tankLevelPct = 0.0f;
}

void setSimulatedLevel(WaterNodeState& state, float levelPct) {
  if (levelPct < 0.0f) levelPct = 0.0f;
  if (levelPct > 100.0f) levelPct = 100.0f;
  state.tankLevelPct = levelPct;

  Serial.print("[SIM] Nivel -> ");
  Serial.print(state.tankLevelPct, 1);
  Serial.println("%");
}

void setSimulatedLeak(WaterNodeState& state, bool detected) {
  state.leakDetected = detected;
  Serial.print("[SIM] Fuga -> ");
  Serial.println(detected ? "true" : "false");
}
