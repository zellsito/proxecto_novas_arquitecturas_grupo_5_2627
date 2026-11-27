#pragma once

#include "node_state.h"

void initSensorsSim();
void stepSensorsSim(WaterNodeState& state);
void setSimulatedLevel(WaterNodeState& state, float levelPct);
void setSimulatedLeak(WaterNodeState& state, bool detected);
