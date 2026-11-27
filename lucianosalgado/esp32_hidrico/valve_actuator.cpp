#include "valve_actuator.h"

#include <Arduino.h>

void setValveState(WaterNodeState& state, bool open) {
  state.valveOpen = open;
  Serial.print("[ACTUADOR] Válvula -> ");
  Serial.println(state.valveStateStr());
}
