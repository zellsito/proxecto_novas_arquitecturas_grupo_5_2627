#pragma once

#include "node_state.h"

void initSensorsHw();
void readSensorsHw(WaterNodeState& state);
bool pollHatchButton(WaterNodeState& state);
void setFanLed(bool on);
